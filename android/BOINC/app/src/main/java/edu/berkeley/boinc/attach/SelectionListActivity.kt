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
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.net.ConnectivityManager
import android.os.Bundle
import android.os.IBinder
import android.os.RemoteException
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.getSystemService
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.R
import edu.berkeley.boinc.adapter.ProjectListEntry
import edu.berkeley.boinc.adapter.SelectionRecyclerViewAdapter
import edu.berkeley.boinc.attach.ProjectAttachService.LocalBinder
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.databinding.AttachProjectListLayoutBinding
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.isOnline
import kotlinx.coroutines.*

class SelectionListActivity : AppCompatActivity() {
    private lateinit var binding: AttachProjectListLayoutBinding

    private val entries: MutableList<ProjectListEntry> = ArrayList()
    private val selected: MutableList<ProjectInfo?> = ArrayList()

    // services
    private var monitor: IMonitor? = null
    private var mIsBound = false
    private var attachService: ProjectAttachService? = null
    private var asIsBound = false

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Logging.logVerbose(Logging.Category.GUI_VIEW, "AttachProjectListActivity onCreate")

        doBindService()

        // setup layout
        binding = AttachProjectListLayoutBinding.inflate(layoutInflater)
        setContentView(binding.root)
    }

    override fun onDestroy() {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "AttachProjectListActivity onDestroy")

        doUnbindService()
        super.onDestroy()
    }

    // check whether user has checked at least a single project
    // shows toast otherwise
    private fun checkProjectChecked(): Boolean {
        val checked = entries.any { it.isChecked }
        if (!checked) {
            val toast = Toast.makeText(applicationContext, R.string.attachproject_list_header, Toast.LENGTH_SHORT)
            toast.show()

            Logging.logDebug(Logging.Category.GUI_VIEW, "AttachProjectListActivity no project selected, stop!")
        }
        return checked
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not guarantee connection to project server
    // is possible!
    private fun checkDeviceOnline(): Boolean {
        val connectivityManager = getSystemService<ConnectivityManager>()!!
        val online = connectivityManager.isOnline
        if (!online) {
            val toast = Toast.makeText(applicationContext, R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT)
            toast.show()

            Logging.logDebug(Logging.Category.GUI_VIEW, "AttachProjectListActivity not online, stop!")
        }
        return online
    }

    // triggered by continue button
    fun continueClicked(@Suppress("UNUSED_PARAMETER") v: View) {
        if (!checkProjectChecked() || !checkDeviceOnline()) {
            return
        }
        val selectedProjectsDebug = StringBuilder()
        // get selected projects
        selected.clear()
        for (tmp in entries) {
            if (tmp.isChecked) {
                selected.add(tmp.info)
                selectedProjectsDebug.append(tmp.info!!.name).append(",")
            }
        }

        Logging.logDebug(Logging.Category.GUI_VIEW, "SelectionListActivity: selected projects: $selectedProjectsDebug")

        attachService!!.setSelectedProjects(selected) // returns immediately

        // start credential input activity
        startActivity(Intent(this, CredentialInputActivity::class.java))
    }

    private fun onCancel() {
        finish()
    }

    override fun onBackPressed() {
        onCancel()
    }

    // triggered by cancel button
    fun cancelClicked(@Suppress("UNUSED_PARAMETER") v: View) {
        onCancel()
    }

    private val mMonitorConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service)
            mIsBound = true
            lifecycleScope.launch {
                updateProjectList()
            }
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false
        }
    }
    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as LocalBinder).service
            asIsBound = true
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }

    private fun doBindService() {
        // start service to allow setForeground later on...
        startService(Intent(this, Monitor::class.java))
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(Intent(this, Monitor::class.java), mMonitorConnection, Service.BIND_AUTO_CREATE)
        // bind to attach service
        bindService(Intent(this, ProjectAttachService::class.java), mASConnection, Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            unbindService(mMonitorConnection)
            mIsBound = false
        }
        if (asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection)
            asIsBound = false
        }
    }

    private suspend fun updateProjectList() {
        retrieveProjectList() ?: return

        binding.projectsRecyclerView.adapter = SelectionRecyclerViewAdapter(this, entries)
        binding.projectsRecyclerView.layoutManager = LinearLayoutManager(this)
    }

    private suspend fun retrieveProjectList() = withContext(Dispatchers.Default) {
        var data: List<ProjectInfo>? = null
        var retry = true
        // Try to get the project list for as long as the coroutine has not been canceled
        while (retry) {
            try {
                data = monitor!!.attachableProjects
            } catch (e: RemoteException) {
                Logging.logWarning(Logging.Category.MONITOR,
                        "retrieveProjectList(): failed to get attachable projects: $e"
                    )
            }
            if (!coroutineContext.isActive) {
                return@withContext data // Does not matter if data == null or not
            }
            if (data == null) {
                Logging.logWarning(Logging.Category.MONITOR, "retrieveProjectList(): failed to retrieve data, retry....")

                delay(500)
            } else {
                retry = false
            }
        }

        Logging.logDebug(Logging.Category.MONITOR, "monitor.getAttachableProjects returned with ${data!!.size} elements")

        @Suppress("SENSELESS_COMPARISON")
        if (data != null) {
            // Clear current ProjectListEntries since we successfully have got new ProjectInfos
            entries.clear()
            // Transform ProjectInfos into ProjectListEntries
            for (i in data.indices.reversed()) {
                if (!coroutineContext.isActive) {
                    return@withContext data
                }
                entries.add(ProjectListEntry(data[i]))
            }

            // Sort ProjectListEntries off the UI thread
            entries.sortWith(Comparator { e1, e2 ->
                e1.info!!.name.compareTo(
                    e2.info!!.name,
                    ignoreCase = true
                )
            })
        }
        return@withContext data
    }
}
