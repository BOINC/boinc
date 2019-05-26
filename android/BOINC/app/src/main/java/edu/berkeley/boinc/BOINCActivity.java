/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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
package edu.berkeley.boinc;

import android.app.Dialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.support.v4.app.ActionBarDrawerToggle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;

import edu.berkeley.boinc.adapter.NavDrawerListAdapter;
import edu.berkeley.boinc.adapter.NavDrawerListAdapter.NavDrawerItem;
import edu.berkeley.boinc.attach.SelectionListActivity;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.utils.BOINCDefs;
import edu.berkeley.boinc.utils.Logging;

import java.util.ArrayList;

public class BOINCActivity extends AppCompatActivity {

    public static IMonitor monitor;
    private Integer clientComputingStatus = -1;
    private Integer numberProjectsInNavList = 0;
    static Boolean mIsBound = false;

    // app title (changes with nav bar selection)
    private CharSequence mTitle;
    // nav drawer title
    private CharSequence mDrawerTitle;

    private DrawerLayout mDrawerLayout;
    private ListView mDrawerList;
    private ActionBarDrawerToggle mDrawerToggle;
    private NavDrawerListAdapter mDrawerListAdapter;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;
            determineStatus();
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            monitor = null;
            mIsBound = false;

            Log.e(Logging.TAG, "BOINCActivity onServiceDisconnected");
        }
    };

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "BOINCActivity ClientStatusChange - onReceive()");
            }
            determineStatus();
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

    @Override
    public void onCreate(Bundle savedInstanceState) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onCreate()");
        }
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // setup navigation bar
        mTitle = mDrawerTitle = getTitle();
        mDrawerLayout = findViewById(R.id.drawer_layout);
        mDrawerList = findViewById(R.id.list_slidermenu);
        mDrawerList.setOnItemClickListener(new ListView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                // display view for selected nav drawer item
                dispatchNavBarOnClick(mDrawerListAdapter.getItem(position), false);
            }
        });
        mDrawerListAdapter = new NavDrawerListAdapter(getApplicationContext());
        mDrawerList.setAdapter(mDrawerListAdapter);
        // enabling action bar app icon and behaving it as toggle button
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_drawer);

        mDrawerToggle = new ActionBarDrawerToggle(this, mDrawerLayout,
                                                  R.drawable.ic_drawer, //nav menu toggle icon
                                                  R.string.app_name, // nav drawer open - description for accessibility
                                                  R.string.app_name // nav drawer close - description for accessibility
        ) {
            public void onDrawerClosed(View view) {
                getSupportActionBar().setTitle(mTitle);
                // calling onPrepareOptionsMenu() to show action bar icons
                supportInvalidateOptionsMenu();
            }

            public void onDrawerOpened(View drawerView) {
                getSupportActionBar().setTitle(mDrawerTitle);
                mDrawerListAdapter.notifyDataSetChanged(); // force redraw of all items (adapter.getView()) in order to adapt changing icons or number of tasks/notices
                // calling onPrepareOptionsMenu() to hide action bar icons
                supportInvalidateOptionsMenu();
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);


        // pre-select fragment
        // 1. check if explicitly requested fragment present
        // e.g. after initial project attach.
        int targetFragId = getIntent().getIntExtra("targetFragment", -1);

        // 2. if no explicit request, try to restore previous selection
        if(targetFragId < 0 && savedInstanceState != null) {
            targetFragId = savedInstanceState.getInt("navBarSelectionId");
        }

        NavDrawerItem item;
        if(targetFragId < 0) {
            // if non of the above, go to default
            item = mDrawerListAdapter.getItem(0);
        }
        else {
            item = mDrawerListAdapter.getItemForId(targetFragId);
        }

        if(item != null) {
            dispatchNavBarOnClick(item, true);
        }
        else if(Logging.WARNING) {
            Log.w(Logging.TAG, "onCreate: fragment selection returned null");
        }

        //bind monitor service
        doBindService();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putInt("navBarSelectionId", mDrawerListAdapter.selectedMenuId);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onDestroy()");
        }
        doUnbindService();
        super.onDestroy();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onNewIntent()");
        }
        // onNewIntent gets called if activity is brought to front via intent, but was still alive, so onCreate is not called again
        // getIntent always returns the intent activity was created of, so this method is the only hook to receive an updated intent
        // e.g. after (not initial) project attach
        super.onNewIntent(intent);
        // navigate to explicitly requested fragment (e.g. after project attach)
        int id = intent.getIntExtra("targetFragment", -1);
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onNewIntent() for target fragment: " + id);
        }
        NavDrawerItem item = mDrawerListAdapter.getItemForId(id);
        if(item != null) {
            dispatchNavBarOnClick(item, false);
        }
        else if(Logging.WARNING) {
            Log.w(Logging.TAG, "onNewIntent: requested target fragment is null, for id: " + id);
        }
    }

    @Override
    protected void onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
        super.onResume();
        registerReceiver(mClientStatusChangeRec, ifcsc);
        determineStatus();
    }

    @Override
    protected void onPause() { // gets called by system every time activity loses focus.
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "BOINCActivity onPause()");
        }
        super.onPause();
        unregisterReceiver(mClientStatusChangeRec);
    }

    private void doBindService() {
        // start service to allow setForeground later on...
        startService(new Intent(this, Monitor.class));
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(new Intent(this, Monitor.class), mConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(mIsBound) {
            // Detach existing connection.
            unbindService(mConnection);
            mIsBound = false;
        }
    }
	/*
	public IMonitor getMonitorService() {
		if(!mIsBound) if(Logging.WARNING) Log.w(Logging.TAG, "Fragment trying to obtain serive reference, but Monitor not bound in BOINCActivity");
		return monitor;
	}*/

    public void startAttachProjectListActivity() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity attempt to start ");
        }
        startActivity(new Intent(this, SelectionListActivity.class));
    }

    /**
     * React to selection of nav bar item
     *
     * @param item Nav bar item
     * @param init Initialize
     */
    private void dispatchNavBarOnClick(NavDrawerItem item, boolean init) {
        // update the main content by replacing fragments
        if(item == null) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "dispatchNavBarOnClick returns, item null.");
            }
            return;
        }
        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "dispatchNavBarOnClick for item with id: " + item.getId() + " title: " + item.getTitle() +
                  " is project? " + item.isProjectItem());
        }

        FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
        Boolean fragmentChanges = false;
        if(init) {
            // if init, setup status fragment
            ft.replace(R.id.status_container, new StatusFragment());
        }
        if(!item.isProjectItem()) {
            switch(item.getId()) {
                case R.string.tab_tasks:
                    ft.replace(R.id.frame_container, new TasksFragment());
                    fragmentChanges = true;
                    break;
                case R.string.tab_notices:
                    ft.replace(R.id.frame_container, new NoticesFragment());
                    fragmentChanges = true;
                    break;
                case R.string.tab_projects:
                    ft.replace(R.id.frame_container, new ProjectsFragment());
                    fragmentChanges = true;
                    break;
                case R.string.menu_help:
                    startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://boinc.berkeley.edu/wiki/BOINC_Help")));
                    break;
                case R.string.menu_report_issue:
                    startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://boinc.berkeley.edu/trac/wiki/ReportBugs")));
                    break;
                case R.string.menu_about:
                    final Dialog dialog = new Dialog(this);
                    dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                    dialog.setContentView(R.layout.dialog_about);
                    Button returnB = dialog.findViewById(R.id.returnB);
                    TextView tvVersion = dialog.findViewById(R.id.version);
                    try {
                        tvVersion.setText(getString(R.string.about_version) + " "
                                          + getPackageManager().getPackageInfo(getPackageName(), 0).versionName);
                    }
                    catch(NameNotFoundException e) {
                        if(Logging.WARNING) {
                            Log.w(Logging.TAG, "version name not found.");
                        }
                    }

                    returnB.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            dialog.dismiss();
                        }
                    });
                    dialog.show();
                    break;
                case R.string.menu_eventlog:
                    startActivity(new Intent(this, EventLogActivity.class));
                    break;
                case R.string.projects_add:
                    startActivity(new Intent(this, SelectionListActivity.class));
                    break;
                case R.string.tab_preferences:
                    ft.replace(R.id.frame_container, new PrefsFragment());
                    fragmentChanges = true;
                    break;

                default:
                    if(Logging.ERROR) {
                        Log.d(Logging.TAG,
                              "dispatchNavBarOnClick() could not find corresponding fragment for " + item.getTitle());
                    }
                    break;
            }

        }
        else {
            // ProjectDetailsFragment. Data shown based on given master URL
            Bundle args = new Bundle();
            args.putString("url", item.getProjectMasterUrl());
            Fragment frag = new ProjectDetailsFragment();
            frag.setArguments(args);
            ft.replace(R.id.frame_container, frag);
            fragmentChanges = true;
        }

        mDrawerLayout.closeDrawer(mDrawerList);

        if(fragmentChanges) {
            ft.commit();
            setTitle(item.getTitle());
            mDrawerListAdapter.selectedMenuId = item.getId(); //highlight item persistently
            mDrawerListAdapter.notifyDataSetChanged(); // force redraw
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "displayFragmentForNavDrawer() " + item.getTitle());
        }
    }

    // tests whether status is available and whether it changed since the last event.
    private void determineStatus() {
        try {
            if(mIsBound) {
                Integer newComputingStatus = monitor.getComputingStatus();
                if(!newComputingStatus.equals(clientComputingStatus)) {
                    // computing status has changed, update and invalidate to force adaption of action items
                    clientComputingStatus = newComputingStatus;
                    supportInvalidateOptionsMenu();
                }
                if(numberProjectsInNavList != monitor.getProjects().size()) {
                    numberProjectsInNavList =
                            mDrawerListAdapter.compareAndAddProjects((ArrayList<Project>) monitor.getProjects());
                }
                //setAppTitle();
            }
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "BOINCActivity.determineStatus error: ", e);
            }
        }
    }

    public final boolean onKeyDown(final int keyCode, final KeyEvent keyEvent) {
        if(keyCode == KeyEvent.KEYCODE_MENU) {
            if(this.mDrawerLayout.isDrawerOpen(this.mDrawerList)) {
                this.mDrawerLayout.closeDrawer(this.mDrawerList);
            }
            else {
                this.mDrawerLayout.openDrawer(this.mDrawerList);
            }
            return true;
        }
        return super.onKeyDown(keyCode, keyEvent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onCreateOptionsMenu()");
        }

        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onPrepareOptionsMenu()");
        }

        // run mode, set title and icon based on status
        MenuItem runMode = menu.findItem(R.id.run_mode);
        if(clientComputingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
            // display play button
            runMode.setTitle(R.string.menu_run_mode_enable);
            runMode.setIcon(R.drawable.playw);
        }
        else {
            // display stop button
            runMode.setTitle(R.string.menu_run_mode_disable);
            runMode.setIcon(R.drawable.pausew);
        }

        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BOINCActivity onOptionsItemSelected()");
        }

        // toggle drawer
        if(mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        switch(item.getItemId()) {
            case R.id.run_mode:
                if(item.getTitle().equals(getApplication().getString(R.string.menu_run_mode_disable))) {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG, "run mode: disable");
                    }
                    new WriteClientModeAsync().execute(BOINCDefs.RUN_MODE_NEVER);
                }
                else if(item.getTitle().equals(getApplication().getString(R.string.menu_run_mode_enable))) {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG, "run mode: enable");
                    }
                    new WriteClientModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
                }
                else if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "run mode: unrecognized command");
                }
                return true;
            case R.id.projects_add:
                startActivity(new Intent(this, SelectionListActivity.class));
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Sync the toggle state after onRestoreInstanceState has occurred.
        mDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Pass any configuration change to the drawer toggls
        mDrawerToggle.onConfigurationChanged(newConfig);
    }

    @Override
    public void setTitle(CharSequence title) {
        mTitle = title;
        getSupportActionBar().setTitle(mTitle);
    }

    private final class WriteClientModeAsync extends AsyncTask<Integer, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Integer... params) {
            // setting provided mode for both, CPU computation and network.
            Boolean runMode;
            try {
                runMode = monitor.setRunMode(params[0]);
            }
            catch(RemoteException e) {
                runMode = false;
            }
            Boolean networkMode;
            try {
                networkMode = monitor.setNetworkMode(params[0]);
            }
            catch(RemoteException e) {
                networkMode = false;
            }
            return runMode && networkMode;
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(success) {
                try {
                    monitor.forceRefresh();
                }
                catch(RemoteException e) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "BOINCActivity.onKeyDown.onPostExecute() error: ", e);
                    }
                }
            }
            else if(Logging.WARNING) {
                Log.w(Logging.TAG, "setting run and network mode failed");
            }
        }
    }
}
