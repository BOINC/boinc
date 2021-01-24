/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */
package edu.berkeley.boinc

import android.app.Dialog
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.os.RemoteException
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.adapter.TaskRecyclerViewAdapter
import edu.berkeley.boinc.databinding.DialogConfirmBinding
import edu.berkeley.boinc.databinding.TasksLayoutBinding
import edu.berkeley.boinc.rpc.Result
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.utils.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlin.collections.ArrayList
import java.util.*

class TasksFragment : Fragment() {
    private lateinit var recyclerViewAdapter: TaskRecyclerViewAdapter
    private val data: MutableList<TaskData> = ArrayList()
    private var lastFullUpdateTimeStamp: Long = 0
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (Logging.VERBOSE) {
                Log.d(Logging.TAG, "TasksActivity onReceive")
            }
            loadData()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "TasksFragment onCreateView")
        }
        // Inflate the layout for this fragment
        val binding = TasksLayoutBinding.inflate(inflater, container, false)
        recyclerViewAdapter = TaskRecyclerViewAdapter(this, data)
        binding.tasksList.adapter = recyclerViewAdapter
        binding.tasksList.layoutManager = LinearLayoutManager(context)
        return binding.root
    }

    override fun onResume() {
        super.onResume()
        //register noisy clientStatusChangeReceiver here, so only active when Activity is visible
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "TasksFragment register receiver")
        }
        requireActivity().registerReceiver(mClientStatusChangeRec, ifcsc)
        loadData()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "TasksFragment remove receiver")
        }
        requireActivity().unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    private fun loadTasks(start: Int, count: Int, isActive: Boolean): List<Result> {
        return BOINCActivity.monitor?.getTasks(start, count, isActive) ?: emptyList()
    }

    private fun loadTasks(isActive: Boolean): MutableList<Result> {
        val tasks: MutableList<Result> = ArrayList()
        var start = 0
        val count = 10

        while (true) {
            val data = loadTasks(start, count, isActive)
            tasks.addAll(data);
            if (data.size < count) break
            start += count
        }

        return tasks
    }

    private fun compareTwoListsOfActiveTasks(old: List<TaskData>, new: List<Result>): Boolean {
        if (old.size != new.size) {
            return false
        }
        return old.none { o -> new.indexOfFirst { it.name == o.id } == -1 }
    }

    private fun loadData() {
        val tasks: MutableList<Result> = ArrayList()
        val activeTasks: MutableList<TaskData> = ArrayList()
        val timestamp = System.currentTimeMillis()
        // perform full update every 10 seconds
        var fullUpdate: Boolean = (timestamp - lastFullUpdateTimeStamp) > (10 * 1000)

        for (task in data) {
            if (task.isTaskActive) {
                activeTasks.add(task)
            }
        }

        val newActiveTasks = loadTasks(true)

        if (!fullUpdate) {
            fullUpdate = !(compareTwoListsOfActiveTasks(activeTasks, newActiveTasks))
        }

        tasks.addAll(newActiveTasks)

        if (fullUpdate) {
            tasks.addAll(loadTasks(false))
            lastFullUpdateTimeStamp = timestamp
        }

        //setup list and adapter
        if (tasks.isNotEmpty()) {
            //deep copy, so ArrayList adapter actually recognizes the difference
            updateData(tasks, fullUpdate)
            recyclerViewAdapter.notifyDataSetChanged() //force list adapter to refresh
        } else {
            if (Logging.WARNING) {
                Log.w(Logging.TAG, "loadData: array is empty, rpc failed")
            }
        }
    }

    private fun updateData(newData: List<Result>, fullUpdate: Boolean) {
        //loop through all received Result items to add new results
        for (rpcResult in newData) {
            //check whether this Result is new
            val index = data.indexOfFirst { it.id == rpcResult.name }
            if (index == -1) { // result is new, add
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "new result found, id: " + rpcResult.name)
                }
                data.add(TaskData(rpcResult))
            } else { // result was present before, update its data
                data[index].updateResultData(rpcResult)
            }
        }

        if (fullUpdate) {
            //loop through the list adapter to find removed (ready/aborted) Results
            data.removeIf { item -> newData.none { it.name == item.id } }
        }
    }

    inner class TaskData(var result: Result) {
        var isExpanded = false
        var id = result.name
        var nextState = -1
        private var loopCounter = 0
        // amount of refresh, until transition times out
        private val transitionTimeout = resources.getInteger(R.integer.tasks_transistion_timeout_number_monitor_loops)

        fun updateResultData(result: Result) {
            this.result = result
            val currentState = determineState()
            if (nextState == -1) {
                return
            }
            if (currentState == nextState) {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "nextState met! $nextState")
                }
                nextState = -1
                loopCounter = 0
            } else {
                if (loopCounter < transitionTimeout) {
                    if (Logging.DEBUG) {
                        Log.d(Logging.TAG,
                                "nextState not met yet! " + nextState + " vs " + currentState + " loopCounter: " +
                                        loopCounter)
                    }
                    loopCounter++
                } else {
                    if (Logging.DEBUG) {
                        Log.d(Logging.TAG,
                                "transition timed out! " + nextState + " vs " + currentState + " loopCounter: " +
                                        loopCounter)
                    }
                    nextState = -1
                    loopCounter = 0
                }
            }
        }

        val iconClickListener = View.OnClickListener { view: View ->
            try {
                when (val operation = view.tag as Int) {
                    RpcClient.RESULT_SUSPEND -> {
                        nextState = RESULT_SUSPENDED_VIA_GUI
                        lifecycleScope.launch {
                            performResultOperation(result.projectURL, result.name, operation)
                        }
                    }
                    RpcClient.RESULT_RESUME -> {
                        nextState = PROCESS_EXECUTING
                        lifecycleScope.launch {
                            performResultOperation(result.projectURL, result.name, operation)
                        }
                    }
                    RpcClient.RESULT_ABORT -> {
                        val dialogBinding = DialogConfirmBinding.inflate(layoutInflater)
                        val dialog = Dialog(activity!!).apply {
                            requestWindowFeature(Window.FEATURE_NO_TITLE)
                            setContentView(dialogBinding.root)
                        }
                        dialogBinding.title.setText(R.string.confirm_abort_task_title)
                        dialogBinding.message.text = getString(R.string.confirm_abort_task_message, result.name)
                        dialogBinding.confirm.setText(R.string.confirm_abort_task_confirm)
                        dialogBinding.confirm.setOnClickListener {
                            nextState = RESULT_ABORTED
                            lifecycleScope.launch {
                                performResultOperation(result.projectURL, result.name, operation)
                            }
                            dialog.dismiss()
                        }
                        dialogBinding.cancel.setOnClickListener { dialog.dismiss() }
                        dialog.show()
                    }
                    else -> if (Logging.WARNING) {
                        Log.w(Logging.TAG, "could not map operation tag")
                    }
                }
                recyclerViewAdapter.notifyDataSetChanged() //force list adapter to refresh
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "failed parsing view tag")
                }
            }
        }

        fun determineState(): Int {
            if (result.isSuspendedViaGUI) {
                return RESULT_SUSPENDED_VIA_GUI
            }
            if (result.isProjectSuspendedViaGUI) {
                return RESULT_PROJECT_SUSPENDED
            }
            if (result.isReadyToReport && result.state != RESULT_ABORTED && result.state != RESULT_COMPUTE_ERROR) {
                return RESULT_READY_TO_REPORT
            }
            return if (result.isActiveTask) {
                result.activeTaskState
            } else {
                result.state
            }
        }

        val isTaskActive: Boolean
            get() = result.isActiveTask
    }

    suspend fun performResultOperation(url: String, name: String, operation: Int) = coroutineScope {
        val success = withContext(Dispatchers.Default) {
            try {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "URL: $url, Name: $name, operation: $operation")
                }
                return@withContext BOINCActivity.monitor!!.resultOp(operation, url, name)
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "performResultOperation() error: ", e)
                }
            }
            return@withContext false
        }

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "performResultOperation() error: ", e)
                }
            }
        } else if (Logging.WARNING) {
            Log.w(Logging.TAG, "performResultOperation() failed.")
        }
    }
}
