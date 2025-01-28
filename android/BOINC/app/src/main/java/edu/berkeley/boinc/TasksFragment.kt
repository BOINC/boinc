/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2023 University of California
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

import android.app.AlertDialog
import android.app.Dialog
import android.content.BroadcastReceiver
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.content.IntentFilter
import android.net.Uri
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.os.Bundle
import android.os.PowerManager
import android.os.RemoteException
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.preference.PreferenceManager
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.adapter.TaskRecyclerViewAdapter
import edu.berkeley.boinc.databinding.DialogConfirmBinding
import edu.berkeley.boinc.databinding.TasksLayoutBinding
import edu.berkeley.boinc.rpc.Result
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.utils.*
import kotlin.collections.ArrayList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class TasksFragment : Fragment() {
    private lateinit var recyclerViewAdapter: TaskRecyclerViewAdapter
    private val data: MutableList<TaskData> = ArrayList()
    private var lastFullUpdateTimeStamp: Long = 0
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Logging.logVerbose(Logging.Category.GUI_VIEW, "TasksActivity onReceive")
            loadData()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    private fun showBatterySaverOptions() {
        try {
            val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(requireContext())
            val optionName = "batterySaverOptionsShown"
            if (optionName !in sharedPreferences) {
                sharedPreferences.edit().putBoolean(optionName, true).apply()
                val intent = Intent()
                if (VERSION.SDK_INT >= VERSION_CODES.M) {
                    val packageName = requireActivity().packageName
                    val powerManager =
                        requireActivity().getSystemService(AppCompatActivity.POWER_SERVICE) as PowerManager
                    if (!powerManager.isIgnoringBatteryOptimizations(packageName)) {
                        intent.action = Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS
                        intent.data = Uri.parse("package:$packageName")
                    }
                } else {
                    if (intent.resolveActivity(requireActivity().packageManager) != null) {
                        intent.action = Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS
                    }
                }
                if (intent.action != null) {
                    val builder = AlertDialog.Builder(requireActivity())
                    builder.setTitle("Battery Optimization")
                    builder.setMessage("This application have been optimized for battery usage. Do you want to open Battery Optimization Settings?")
                    builder.setPositiveButton("Yes",
                        DialogInterface.OnClickListener { _, _ ->
                            startActivity(intent)
                        })
                    builder.setNegativeButton("No",
                        DialogInterface.OnClickListener { dialog, _ ->
                            dialog.dismiss()
                        })

                    val dialog: AlertDialog = builder.create()
                    dialog.show()
                }
            }
        }
        catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "showBatterySaverOptions() error: ", e)
        }
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
            Logging.logVerbose(Logging.Category.GUI_VIEW, "TasksFragment onCreateView")
        showBatterySaverOptions()
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
        Logging.logVerbose(Logging.Category.GUI_VIEW, "TasksFragment register receiver")

        requireActivity().registerReceiver(mClientStatusChangeRec, ifcsc)
        loadData()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        Logging.logVerbose(Logging.Category.GUI_VIEW, "TasksFragment remove receiver")

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
        //deep copy, so ArrayList adapter actually recognizes the difference
        updateData(tasks, fullUpdate)
        recyclerViewAdapter.notifyDataSetChanged() //force list adapter to refresh
    }

    private fun updateData(newData: List<Result>, fullUpdate: Boolean) {
        //loop through all received Result items to add new results
        for (rpcResult in newData) {
            //check whether this Result is new
            val index = data.indexOfFirst { it.id == rpcResult.name }
            if (index == -1) { // result is new, add
                Logging.logDebug(Logging.Category.GUI_VIEW, "new result found, id: " + rpcResult.name)

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
        var sharedPreferences = PreferenceManager.getDefaultSharedPreferences(requireContext())
        var isExpanded = sharedPreferences.getBoolean("expandWuData", false)
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
                Logging.logDebug(Logging.Category.GUI_VIEW, "nextState met! $nextState")

                nextState = -1
                loopCounter = 0
            } else {
                if (loopCounter < transitionTimeout) {
                    Logging.logDebug(Logging.Category.GUI_VIEW,
                            "nextState not met yet! " + nextState + " vs " + currentState + " loopCounter: " +
                            loopCounter)

                    loopCounter++
                } else {
                    Logging.logDebug(Logging.Category.GUI_VIEW,
                            "transition timed out! " + nextState + " vs " + currentState + " loopCounter: " +
                            loopCounter)

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
                    else -> {
                        Logging.logError(Logging.Category.GUI_VIEW, "could not map operation tag")
                    }
                }
                recyclerViewAdapter.notifyDataSetChanged() //force list adapter to refresh
            } catch (e: Exception) {
                Logging.logError(Logging.Category.GUI_VIEW, "failed parsing view tag")
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
                Logging.logDebug(Logging.Category.GUI_VIEW, "URL: $url, Name: $name, operation: $operation")

                return@withContext BOINCActivity.monitor!!.resultOp(operation, url, name)
            } catch (e: Exception) {
                Logging.logException(Logging.Category.GUI_VIEW, "performResultOperation() error: ", e)

            }
            return@withContext false
        }

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                Logging.logException(Logging.Category.GUI_VIEW, "performResultOperation() error: ", e)
            }
        } else {
            Logging.logError(Logging.Category.GUI_VIEW, "performResultOperation() failed.")
        }
    }
}
