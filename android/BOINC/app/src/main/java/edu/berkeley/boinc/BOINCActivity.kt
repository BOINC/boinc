/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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

import android.app.Service
import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.os.RemoteException
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.view.get
import androidx.lifecycle.lifecycleScope
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import edu.berkeley.boinc.attach.SelectionListActivity
import edu.berkeley.boinc.client.ClientStatus
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.databinding.ActivityMainBinding
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.RUN_MODE_AUTO
import edu.berkeley.boinc.utils.RUN_MODE_NEVER
import edu.berkeley.boinc.utils.writeClientModeAsync
import kotlinx.coroutines.launch

class BOINCActivity : AppCompatActivity() {
    private var clientComputingStatus = -1

    private val mConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service)
            mIsBound = true
            determineStatus()
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false
            Log.e(Logging.TAG, "BOINCActivity onServiceDisconnected")
        }
    }
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (Logging.VERBOSE) {
                Log.d(Logging.TAG, "BOINCActivity ClientStatusChange - onReceive()")
            }
            determineStatus()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    private lateinit var appBarConfiguration: AppBarConfiguration

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val toolbar: Toolbar = findViewById(R.id.toolbar)
        setSupportActionBar(toolbar)

        val navController = findNavController(R.id.nav_host_fragment)
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        val navSet = setOf(R.id.nav_tasks, R.id.nav_notices, R.id.nav_projects, R.id.nav_add_project,
                R.id.nav_preferences, R.id.nav_about, R.id.nav_event_log)
        appBarConfiguration = AppBarConfiguration(navSet, binding.drawerLayout)
        setupActionBarWithNavController(navController, appBarConfiguration)
        binding.navView.setupWithNavController(navController)

        //bind monitor service
        doBindService()
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }

    override fun onDestroy() {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onDestroy()")
        }
        doUnbindService()
        super.onDestroy()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onOptionsItemSelected()")
        }

        // toggle drawer
        return when (item.itemId) {
            R.id.run_mode -> {
                when {
                    item.title == application.getString(R.string.menu_run_mode_disable) -> {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG, "run mode: disable")
                        }
                        lifecycleScope.launch { writeClientMode(RUN_MODE_NEVER) }
                    }
                    item.title == application.getString(R.string.menu_run_mode_enable) -> {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG, "run mode: enable")
                        }
                        lifecycleScope.launch { writeClientMode(RUN_MODE_AUTO) }
                    }
                    Logging.DEBUG -> {
                        Log.d(Logging.TAG, "run mode: unrecognized command")
                    }
                }
                true
            }
            R.id.projects_add -> {
                startActivity(Intent(this, SelectionListActivity::class.java))
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onPrepareOptionsMenu(menu: Menu): Boolean {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onPrepareOptionsMenu()")
        }

        // run mode, set title and icon based on status
        val runMode = menu[0]
        if (clientComputingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
            // display play button
            runMode.setTitle(R.string.menu_run_mode_enable)
            runMode.setIcon(R.drawable.ic_baseline_play_arrow_white)
        } else {
            // display stop button
            runMode.setTitle(R.string.menu_run_mode_disable)
            runMode.setIcon(R.drawable.ic_baseline_pause_white)
        }
        return super.onPrepareOptionsMenu(menu)
    }

    override fun onPause() {
        if (Logging.VERBOSE) {
            Log.v(Logging.TAG, "BOINCActivity onPause()")
        }
        super.onPause()
        unregisterReceiver(mClientStatusChangeRec)
    }

    override fun onResume() {
        super.onResume()
        registerReceiver(mClientStatusChangeRec, ifcsc)
        determineStatus()
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment)
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }

    private fun doBindService() {
        // start service to allow setForeground later on...
        startService(Intent(this, Monitor::class.java))
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(Intent(this, Monitor::class.java), mConnection, Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            unbindService(mConnection)
            mIsBound = false
        }
    }

    // tests whether status is available and whether it changed since the last event.
    private fun determineStatus() {
        try {
            if (mIsBound) {
                val newComputingStatus = monitor!!.computingStatus
                if (newComputingStatus != clientComputingStatus) {
                    // computing status has changed, update and invalidate to force adaption of action items
                    clientComputingStatus = newComputingStatus
                    invalidateOptionsMenu()
                }
            }
        } catch (e: Exception) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "BOINCActivity.determineStatus error: ", e)
            }
        }
    }

    private suspend fun writeClientMode(mode: Int) {
        val success = writeClientModeAsync(mode)

        if (success) {
            try {
                monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "BOINCActivity.writeClientMode() error: ", e)
                }
            }
        } else if (Logging.WARNING) {
            Log.w(Logging.TAG, "BOINCActivity setting run and network mode failed")
        }
    }

    companion object {
        @JvmField
        var monitor: IMonitor? = null
        var mIsBound = false
    }
}
