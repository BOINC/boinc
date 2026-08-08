/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.attach

import android.app.Service
import android.content.*
import android.os.Bundle
import android.os.IBinder
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.IndividualCredentialInputFragment.IndividualCredentialInputFragmentListener
import edu.berkeley.boinc.attach.ProjectAttachService.Companion.RESULT_ONGOING
import edu.berkeley.boinc.attach.ProjectAttachService.LocalBinder
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper
import edu.berkeley.boinc.databinding.AttachProjectBatchConflictsLayoutBinding
import edu.berkeley.boinc.utils.Logging
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class BatchConflictListActivity : AppCompatActivity(), IndividualCredentialInputFragmentListener {
    private lateinit var binding: AttachProjectBatchConflictsLayoutBinding

    private lateinit var recyclerViewAdapter: BatchConflictRecyclerViewAdapter
    private var attachService: ProjectAttachService? = null
    private var asIsBound = false
    private var manualUrl: String? = null

    // not related to client status change, just used to implement cyclic checking of
    // results.
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BatchConflictListActivity ClientStatusChange - onReceive()")

            if (asIsBound) {
                recyclerViewAdapter.notifyDataSetChanged()
            }
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as LocalBinder).service
            asIsBound = true

            // set data, if manual url
            if (!manualUrl.isNullOrEmpty()) {
                Logging.logDebug(Logging.Category.GUI_ACTIVITY, "BatchConflictListActivity manual URL found: $manualUrl")

                attachService!!.setManuallySelectedProject(manualUrl!!)
                manualUrl = ""
            }

            // retrieve data
            val results = attachService!!.selectedProjects
            recyclerViewAdapter = BatchConflictRecyclerViewAdapter(this@BatchConflictListActivity,
                    results)
            binding.recyclerView.apply {
                adapter = recyclerViewAdapter
                layoutManager = LinearLayoutManager(this@BatchConflictListActivity)
            }

            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "BatchConflictListActivity setup list with " + results.size + " elements.")
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BatchConflictListActivity onCreate")

        doBindService()
        // setup layout
        binding = AttachProjectBatchConflictsLayoutBinding.inflate(layoutInflater)
        setContentView(binding.root)
        // adapt text
        val conflicts = intent.getBooleanExtra("conflicts", false)
        manualUrl = intent.getStringExtra("manualUrl")
        if (conflicts) {
            binding.desc.setText(R.string.attachproject_conflicts_desc)
        } else {
            binding.desc.setText(R.string.attachproject_credential_input_desc)
        }

        binding.finishButton.setOnClickListener {
            // finally, start BOINCActivity
            val intent = Intent(this, BOINCActivity::class.java).apply {
                // add flags to return to main activity and clearing all others and clear the back stack
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                putExtra("targetFragment", R.string.tab_projects) // make activity display projects fragment
            }
            startActivity(intent)
        }
    }

    override fun onDestroy() {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BatchConflictListActivity onDestroy")

        doUnbindService()
        super.onDestroy()
    }

    public override fun onResume() {
        registerReceiver(mClientStatusChangeRec, ifcsc)
        super.onResume()
    }

    public override fun onPause() {
        unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    private fun doBindService() {
        // bind to attach service
        bindService(Intent(this, ProjectAttachService::class.java), mASConnection,
                Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection)
            asIsBound = false
        }
    }

    override fun onFinish(project: ProjectAttachWrapper, login: Boolean, email: String, name: String, pwd: String) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "BatchConflictListActivity onFinish of dialog")

        if (asIsBound && !attachService!!.verifyInput(email, name, pwd)) {
            return
        }
        lifecycleScope.launch {
            attachProject(project, login, email, name, pwd)
        }
    }

    override val defaultInput: List<String?>
        get() {
            var values: List<String?> = ArrayList()
            if (asIsBound) {
                values = attachService!!.userDefaultValues
            }
            return values
        }


    private suspend fun attachProject(project: ProjectAttachWrapper, login: Boolean, email: String,
                                      name: String, pwd: String) {
        Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): ${project.config?.name}")

        if (asIsBound) {
            project.result = RESULT_ONGOING
            // adapt layout to changed state
            recyclerViewAdapter.notifyDataSetChanged()
        } else {
            Logging.logError(Logging.Category.PROJECTS, "attachProject(): service not bound, cancel.")

            return
        }

        withContext(Dispatchers.Default) {
            // set input credentials
            attachService!!.setCredentials(email, name, pwd)

            // try attach
            project.lookupAndAttach(login)
        }

        Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): finished, result: " + project.result)

        // adapt layout to changed state
        recyclerViewAdapter.notifyDataSetChanged()
    }
}
