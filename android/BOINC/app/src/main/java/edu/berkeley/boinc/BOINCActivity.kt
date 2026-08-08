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
import android.app.Service
import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.content.pm.PackageManager.PackageInfoFlags
import android.content.res.Configuration
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.os.Bundle
import android.os.IBinder
import android.os.RemoteException
import android.view.KeyEvent
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.Window
import android.widget.AdapterView
import android.widget.AdapterView.OnItemClickListener
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.net.toUri
import androidx.core.os.bundleOf
import androidx.fragment.app.replace
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.adapter.NavDrawerListAdapter
import edu.berkeley.boinc.adapter.NavDrawerListAdapter.NavDrawerItem
import edu.berkeley.boinc.attach.AboutActivity
import edu.berkeley.boinc.attach.AttachAccountManagerActivity
import edu.berkeley.boinc.attach.SelectionListActivity
import edu.berkeley.boinc.client.ClientStatus
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.client.MonitorAsync
import edu.berkeley.boinc.databinding.MainBinding
import edu.berkeley.boinc.ui.eventlog.EventLogActivity
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.RUN_MODE_AUTO
import edu.berkeley.boinc.utils.RUN_MODE_NEVER
import edu.berkeley.boinc.utils.getColorCompat
import edu.berkeley.boinc.utils.writeClientModeAsync
import kotlinx.coroutines.launch

class BOINCActivity : AppCompatActivity() {
    private var clientComputingStatus = -1
    private var numberProjectsInNavList = 0

    // app title (changes with nav bar selection)
    private var mTitle: CharSequence? = null

    private lateinit var binding: MainBinding

    // nav drawer title
    private var mDrawerTitle: CharSequence? = null
    private lateinit var mDrawerToggle: ActionBarDrawerToggle
    private lateinit var mDrawerListAdapter: NavDrawerListAdapter
    private val mConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = MonitorAsync(IMonitor.Stub.asInterface(service))
            mIsBound = true
            determineStatus()
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false

            Logging.logError(Logging.Category.GUI_ACTIVITY, "BOINCActivity onServiceDisconnected")
        }
    }
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Logging.logVerbose(Logging.Category.CLIENT, "BOINCActivity ClientStatusChange - onReceive()")

            determineStatus()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    public override fun onCreate(savedInstanceState: Bundle?) {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BOINCActivity onCreate()")

        super.onCreate(savedInstanceState)
        binding = MainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // setup navigation bar
        mDrawerTitle = title
        mTitle = mDrawerTitle
        binding.drawerList.onItemClickListener =
                OnItemClickListener { _: AdapterView<*>?, _: View?, position: Int, _: Long ->
            // display view for selected nav drawer item
            dispatchNavBarOnClick(mDrawerListAdapter.getItem(position), false)
        }
        mDrawerListAdapter = NavDrawerListAdapter(this)
        binding.drawerList.adapter = mDrawerListAdapter
        // enabling action bar app icon and behaving it as toggle button
        supportActionBar!!.setDisplayHomeAsUpEnabled(true)
        supportActionBar!!.setHomeButtonEnabled(true)

        mDrawerToggle = object : ActionBarDrawerToggle(this, binding.drawerLayout,
                R.string.app_name,  // nav drawer openapplicationContext - description for accessibility
                R.string.app_name // nav drawer close - description for accessibility
        ) {
            override fun onDrawerClosed(view: View) {
                supportActionBar!!.title = mTitle
                // calling onPrepareOptionsMenu() to show action bar icons
                invalidateOptionsMenu()
            }

            override fun onDrawerOpened(drawerView: View) {
                supportActionBar!!.title = mDrawerTitle
                // force redraw of all items (adapter.getView()) in order to adapt changing icons or number of tasks/notices
                mDrawerListAdapter.notifyDataSetChanged()
                // calling onPrepareOptionsMenu() to hide action bar icons
                invalidateOptionsMenu()
            }
        }
        mDrawerToggle.drawerArrowDrawable.color = getColorCompat(R.color.white)
        binding.drawerLayout.addDrawerListener(mDrawerToggle)

        // pre-select fragment
        // 1. check if explicitly requested fragment present
        // e.g. after initial project attach.
        var targetFragId = intent.getIntExtra("targetFragment", -1)

        // 2. if no explicit request, try to restore previous selection
        if (targetFragId < 0 && savedInstanceState != null) {
            targetFragId = savedInstanceState.getInt("navBarSelectionId")
        }
        val item: NavDrawerItem? = if (targetFragId < 0) {
            // if none of the above, go to default
            mDrawerListAdapter.getItem(0)
        } else {
            mDrawerListAdapter.getItemForId(targetFragId)
        }
        if (item != null) {
            dispatchNavBarOnClick(item, true)
        } else {
            Logging.logWarning(Logging.Category.GUI_ACTIVITY, "onCreate: fragment selection returned null")
        }

        //bind monitor service
        doBindService()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putInt("navBarSelectionId", mDrawerListAdapter.selectedMenuId)
        super.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BOINCActivity onDestroy()")

        doUnbindService()
        super.onDestroy()
    }

    override fun onNewIntent(intent: Intent) {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BOINCActivity onNewIntent()")

        // onNewIntent gets called if activity is brought to front via intent, but was still alive, so onCreate is not called again
        // getIntent always returns the intent activity was created of, so this method is the only hook to receive an updated intent
        // e.g. after (not initial) project attach
        super.onNewIntent(intent)
        // navigate to explicitly requested fragment (e.g. after project attach)
        val id = intent.getIntExtra("targetFragment", -1)
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "BOINCActivity onNewIntent() for target fragment: $id")

        val item: NavDrawerItem? = if (id < 0) {
            // if ID is -1, go to default
            mDrawerListAdapter.getItem(0)
        } else {
            mDrawerListAdapter.getItemForId(id)
        }
        dispatchNavBarOnClick(item, false)
    }

    override fun onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
        super.onResume()
        registerReceiver(mClientStatusChangeRec, ifcsc)
        determineStatus()
    }

    override fun onPause() { // gets called by system every time activity loses focus.
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "BOINCActivity onPause()")

        super.onPause()
        unregisterReceiver(mClientStatusChangeRec)
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

    /**
     * React to selection of nav bar item
     *
     * @param item Nav bar item
     * @param init Initialize
     */
    private fun dispatchNavBarOnClick(item: NavDrawerItem?, init: Boolean) {
        // update the main content by replacing fragments
        if (item == null) {
            Logging.logWarning(Logging.Category.USER_ACTION, "dispatchNavBarOnClick returns, item null.")

            return
        }
            Logging.logVerbose(Logging.Category.USER_ACTION,
                    "dispatchNavBarOnClick for item with id: ${item.id} title: ${item.title}" +
                            " is project? ${item.isProjectItem}")

        val ft = supportFragmentManager.beginTransaction()
        var fragmentChanges = false
        if (init) {
            // if init, setup status fragment
            ft.replace<StatusFragment>(R.id.status_container)
        }
        if (!item.isProjectItem) {
            when (item.id) {
                R.string.tab_tasks -> {
                    ft.replace<TasksFragment>(R.id.frame_container)
                    fragmentChanges = true
                }
                R.string.tab_notices -> {
                    ft.replace<NoticesFragment>(R.id.frame_container)
                    fragmentChanges = true
                }
                R.string.tab_projects -> {
                    ft.replace<ProjectsFragment>(R.id.frame_container)
                    fragmentChanges = true
                }
                R.string.menu_help -> startActivity(Intent(Intent.ACTION_VIEW, "https://boinc.berkeley.edu/wiki/BOINC_Help".toUri()))
                R.string.menu_report_issue -> startActivity(Intent(Intent.ACTION_VIEW, "https://boinc.berkeley.edu/trac/wiki/ReportBugs".toUri()))
                R.string.menu_about -> startActivity(Intent(this, AboutActivity::class.java))
                R.string.menu_eventlog -> startActivity(Intent(this, EventLogActivity::class.java))
                R.string.projects_add -> startActivity(Intent(this, SelectionListActivity::class.java))
                R.string.attachproject_acctmgr_header -> startActivity(Intent(this, AttachAccountManagerActivity::class.java))
                R.string.tab_preferences -> {
                    ft.replace<SettingsFragment>(R.id.frame_container)
                    fragmentChanges = true
                }
                else -> Logging.logError(Logging.Category.USER_ACTION,
                            "dispatchNavBarOnClick() could not find corresponding fragment for" +
                            " ${item.title}")
            }
        } else {
            // ProjectDetailsFragment. Data shown based on given master URL
            ft.replace<ProjectDetailsFragment>(R.id.frame_container,
                    args = bundleOf("url" to item.projectMasterUrl))
            fragmentChanges = true
        }
        binding.drawerLayout.closeDrawer(binding.drawerList)
        if (fragmentChanges) {
            ft.commit()
            title = item.title
            mDrawerListAdapter.selectedMenuId = item.id //highlight item persistently
            mDrawerListAdapter.notifyDataSetChanged() // force redraw
        }
        Logging.logDebug(Logging.Category.USER_ACTION, "displayFragmentForNavDrawer() " + item.title)
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
                if (numberProjectsInNavList != monitor!!.projects.size) {
                    numberProjectsInNavList = mDrawerListAdapter.compareAndAddProjects(monitor!!.projects)
                }
                mDrawerListAdapter.updateUseAccountManagerItem()
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_ACTIVITY, "BOINCActivity.determineStatus error: ", e)
        }
    }

    override fun onKeyDown(keyCode: Int, keyEvent: KeyEvent): Boolean {
        if (keyCode == KeyEvent.KEYCODE_MENU) {
            if (binding.drawerLayout.isDrawerOpen(binding.drawerList)) {
                binding.drawerLayout.closeDrawer(binding.drawerList)
            } else {
                binding.drawerLayout.openDrawer(binding.drawerList)
            }
            return true
        }
        return super.onKeyDown(keyCode, keyEvent)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BOINCActivity onCreateOptionsMenu()")

        val inflater = menuInflater
        inflater.inflate(R.menu.main_menu, menu)
        return true
    }

    override fun onPrepareOptionsMenu(menu: Menu): Boolean {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BOINCActivity onPrepareOptionsMenu()")

        // run mode, set title and icon based on status
        val runMode = menu.findItem(R.id.run_mode)
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

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        Logging.logVerbose(Logging.Category.USER_ACTION, "BOINCActivity onOptionsItemSelected()")

        // toggle drawer
        return if (mDrawerToggle.onOptionsItemSelected(item)) {
            true
        } else when (item.itemId) {
            R.id.run_mode -> {
                when {
                    item.title == application.getString(R.string.menu_run_mode_disable) -> {
                        Logging.logDebug(Logging.Category.USER_ACTION, "run mode: disable")

                        lifecycleScope.launch {
                            writeClientMode(RUN_MODE_NEVER)
                        }
                    }
                    item.title == application.getString(R.string.menu_run_mode_enable) -> {
                        Logging.logDebug(Logging.Category.USER_ACTION, "run mode: enable")

                        lifecycleScope.launch {
                            writeClientMode(RUN_MODE_AUTO)
                        }
                    }
                    else -> {
                        Logging.logDebug(Logging.Category.USER_ACTION, "run mode: unrecognized command")
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

    override fun onPostCreate(savedInstanceState: Bundle?) {
        super.onPostCreate(savedInstanceState)
        // Sync the toggle state after onRestoreInstanceState has occurred.
        mDrawerToggle.syncState()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        // Pass any configuration change to the drawer toggls
        mDrawerToggle.onConfigurationChanged(newConfig)
    }

    override fun setTitle(title: CharSequence) {
        mTitle = title
        supportActionBar!!.title = mTitle
    }

    private fun writeClientMode(mode: Int) {
        val success = writeClientModeAsync(mode)

        if (success) {
            try {
                monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                    Logging.logException(Logging.Category.GUI_ACTIVITY, "BOINCActivity.writeClientMode() error: ", e)
            }
        } else {
            Logging.logWarning(Logging.Category.GUI_ACTIVITY, "BOINCActivity setting run and network mode failed")
        }
    }

    companion object {
        @JvmField
        var monitor: MonitorAsync? = null
        var mIsBound = false
    }
}
