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
package edu.berkeley.boinc

import android.app.ActivityManager.TaskDescription
import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.ServiceConnection
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.os.Bundle
import android.os.IBinder
import androidx.appcompat.app.AppCompatActivity
import edu.berkeley.boinc.attach.AttachActivity
import edu.berkeley.boinc.client.ClientStatus
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.client.MonitorAsync
import edu.berkeley.boinc.databinding.ActivitySplashBinding
import edu.berkeley.boinc.ui.eventlog.EventLogActivity
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.Logging.setLogCategories
import edu.berkeley.boinc.utils.Logging.setLogLevel
import edu.berkeley.boinc.utils.TaskRunner
import edu.berkeley.boinc.utils.getBitmapFromVectorDrawable

/**
 * Activity shown at start. Forwards to BOINCActivity automatically, once Monitor has connected to Client and received first data via RPCs.
 * This Activity can not be navigated to, it is also not part of the history stack.
 * Is also shown during shutdown.
 * Long click on the BOINC logo brings up the EventLog, in case their is a problem with the RPC connection that needs to be debugged.
 *
 * @author Joachim Fritzsch
 */
class SplashActivity : AppCompatActivity() {
    private var binding: ActivitySplashBinding? = null
    private var mIsBound = false
    private val mConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established
            mIsBound = true
            monitor = MonitorAsync(IMonitor.Stub.asInterface(service))
            try {
                // check whether BOINC was able to acquire mutex
                if (!isMutexAcquiredAsync().await()) {
                    showNotExclusiveDialog()
                }
                // Read log level from monitor preferences and adjust accordingly
                setLogLevel(monitor!!.logLevel)
                setLogCategories(monitor!!.logCategories)
            } catch (e: Exception) {
                Logging.logException(Logging.Category.GUI_ACTIVITY, "initializing log level failed.", e)
            }
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            mIsBound = false
            monitor = null
        }
    }
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (mIsBound) {
                try {
                    when (monitor!!.setupStatus) {
                        ClientStatus.SETUP_STATUS_AVAILABLE -> {
                            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity SETUP_STATUS_AVAILABLE")
                            // forward to BOINCActivity
                            val startMain = Intent(this@SplashActivity, BOINCActivity::class.java)
                            startActivity(startMain)
                        }
                        ClientStatus.SETUP_STATUS_NOPROJECT -> {
                            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity SETUP_STATUS_NOPROJECT")
                            // run benchmarks to speed up project initialization
                            monitor!!.runBenchmarksAsync { benchmarks: Boolean ->
                                Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity: runBenchmarks returned: $benchmarks")
                            }

                            // forward to PROJECTATTACH
                            val startAttach =
                                Intent(this@SplashActivity, AttachActivity::class.java)
                            startActivity(startAttach)
                        }
                        ClientStatus.SETUP_STATUS_ERROR -> {
                            Logging.logError(Logging.Category.GUI_ACTIVITY, "SplashActivity SETUP_STATUS_ERROR")
                        }
                    }
                } catch (e: Exception) {
                    Logging.logException(Logging.Category.GUI_ACTIVITY, "SplashActivity.BroadcastReceiver.onReceive() error: ", e)
                }
            }
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySplashBinding.inflate(layoutInflater)
        setContentView(binding!!.root)

        // Use BOINC logo in Recent Apps Switcher
        if (VERSION.SDK_INT >= VERSION_CODES.LOLLIPOP) { // API 21
            val label = title.toString()
            val taskDescription: TaskDescription = if (VERSION.SDK_INT < VERSION_CODES.P) { // API 28
                val icon = this.getBitmapFromVectorDrawable(R.drawable.ic_boinc)
                @Suppress("DEPRECATION")
                TaskDescription(label, icon)
            } else if (VERSION.SDK_INT < VERSION_CODES.TIRAMISU) { // API 28 < x < API 33
                @Suppress("DEPRECATION")
                TaskDescription(label, R.drawable.ic_boinc)
            } else { // > API 33
                TaskDescription.Builder().setLabel(label).setIcon(R.drawable.ic_boinc).build()
            }
            setTaskDescription(taskDescription)
        }

        //initialize logging with highest verbosity, read actual value when monitor connected.
        setLogLevel(5)

        //bind monitor service
        doBindService()

        // set long click listener to go to eventlog
        binding!!.logo.setOnLongClickListener {
            startActivity(Intent(this@SplashActivity, EventLogActivity::class.java))
            true
        }
    }

    override fun onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity onResume()")

        super.onResume()
        registerReceiver(mClientStatusChangeRec, ifcsc)
    }

    override fun onPause() { // gets called by system every time activity loses focus.
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity onPause()")

        super.onPause()
        unregisterReceiver(mClientStatusChangeRec)
    }

    override fun onDestroy() {
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "SplashActivity onDestroy()")

        super.onDestroy()
        doUnbindService()
    }

    private fun doBindService() {
        // start service to allow setForeground later on...
        startService(Intent(this, Monitor::class.java))
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(Intent(this, Monitor::class.java), mConnection, BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            unbindService(mConnection)
            mIsBound = false
        }
    }

    private fun showNotExclusiveDialog() {
        Logging.logError(Logging.Category.GUI_ACTIVITY, "SplashActivity: another BOINC app found, show dialog.")

        val notExclusiveDialogIntent = Intent()
        notExclusiveDialogIntent.setClassName(
            "edu.berkeley.boinc",
            "edu.berkeley.boinc.BoincNotExclusiveDialog"
        )
        startActivity(notExclusiveDialogIntent)
        finish()
    }

    private fun isMutexAcquiredAsync(callback: ((Boolean) -> Unit)? = null) =
        TaskRunner(callback) { isMutexAcquired() }

    private fun isMutexAcquired(): Boolean {
        var retryCount = 5
        while(!monitor!!.boincMutexAcquired() && retryCount > 0) {
            Thread.sleep(1000)
            --retryCount
        }
        return monitor!!.boincMutexAcquired()
    }

    companion object {
        private var monitor: MonitorAsync? = null
    }
}
