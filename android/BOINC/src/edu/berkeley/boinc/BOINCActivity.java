/*******************************************************************************
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
 ******************************************************************************/
package edu.berkeley.boinc;

import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.client.*;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.annotation.TargetApi;
import android.app.Dialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle; 
import android.os.IBinder;
import android.util.Log;  
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.support.v7.app.*;
import android.support.v7.app.ActionBar.Tab;

public class BOINCActivity extends ActionBarActivity {
	
	private Monitor monitor;
	private Integer clientSetupStatus = ClientStatus.SETUP_STATUS_LAUNCHING;
	private Integer clientComputingStatus = -1;
	private Boolean intialStart = true;
	private Boolean actionBarSetupCompleted = false;
	
	private Boolean mIsBound = false;

	private ActionBar actionBar;
	private Resources res;

	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    determineStatus();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	// This should not happen
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			//if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity ClientStatusChange - onReceive()"); 

			determineStatus();
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
<<<<<<< HEAD
        if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity onCreate()"); 

=======
        if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity onCreate(), dummy jni: " + getDummyString()); 
>>>>>>> android: new layout, initial commit.
        super.onCreate(savedInstanceState);

        // setup layout, loading without action bar
        actionBar = getSupportActionBar();
        actionBar.hide();
        setContentView(R.layout.main); 
         
        //bind monitor service
        doBindService();
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity onDestroy()");
	    doUnbindService();
	    super.onDestroy();
	}

	@Override
	protected void onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onResume()");
	    super.onResume();
	    registerReceiver(mClientStatusChangeRec, ifcsc);
	    determineStatus();
	}

	@Override
	protected void onPause() { // gets called by system every time activity loses focus.
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onPause()");
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
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	public Monitor getMonitorService() {
		if(!mIsBound) if(Logging.WARNING) Log.w(Logging.TAG, "Fragment trying to obtain serive reference, but Monitor not bound in BOINCActivity");
		return monitor;
	}
	
	public void startAttachProjectListActivity() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity attempt to start ");
		startActivity(new Intent(this,AttachProjectListActivity.class));
	}
    
    // tests whether status is available and whether it changed since the last event.
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
	private void determineStatus() {
    	Integer newStatus = -1;
    	try {
			if(mIsBound) { 
				newStatus = Monitor.getClientStatus().setupStatus;
				if(newStatus != clientSetupStatus) { //only act, when status actually different form old status
					if(Logging.DEBUG) Log.d(Logging.TAG,"determineStatus() client setup status changed! old clientSetupStatus: " + clientSetupStatus + " - new: " + newStatus);
					clientSetupStatus = newStatus;
					layout(); 
				}
				if(intialStart && (clientSetupStatus == ClientStatus.SETUP_STATUS_NOPROJECT)) { // if it is first start and no project attached, show login activity
					startActivity(new Intent(this,AttachProjectListActivity.class));
					intialStart = false;
				}
				Integer newComputingStatus = Monitor.getClientStatus().computingStatus;
				if(newComputingStatus != clientComputingStatus) {
					// computing status has changed, update and invalidate to force adaption of action items
					clientComputingStatus = newComputingStatus;
					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
						invalidateOptionsMenu();
					}
					// if below honeycomb, onprepareoptionsmenu will be called.
				}
				//setAppTitle();
			} 
    	} catch (Exception e) {}
    }
    
    private void layout() {
    	LinearLayout loadingLayout = (LinearLayout) findViewById(R.id.main_loading);
    	LinearLayout errorLayout = (LinearLayout) findViewById(R.id.main_error);
    	RelativeLayout okLayout = (RelativeLayout) findViewById(R.id.main_ok);
    	TextView launchingHeader = (TextView) findViewById(R.id.loading_header);
    	switch (clientSetupStatus) {
    	case ClientStatus.SETUP_STATUS_AVAILABLE:
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.GONE);
    		setupActionBar();
        	okLayout.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_ERROR:
    		actionBar.hide();
    		okLayout.setVisibility(View.GONE); 
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_LAUNCHING:
    		actionBar.hide();
    		okLayout.setVisibility(View.GONE); 
        	errorLayout.setVisibility(View.GONE);
        	loadingLayout.setVisibility(View.VISIBLE);
        	launchingHeader.setText(R.string.status_launching);
    		break;
    	case ClientStatus.SETUP_STATUS_NOPROJECT:
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.GONE);
    		setupActionBar();
        	okLayout.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_CLOSING:
    		okLayout.setVisibility(View.GONE); 
        	errorLayout.setVisibility(View.GONE);
    		actionBar.hide();
        	loadingLayout.setVisibility(View.VISIBLE);
        	launchingHeader.setText(R.string.status_closing);
    		break;
    	case ClientStatus.SETUP_STATUS_CLOSED:
    		finish(); // close application
    		break;
    	default:
    		if(Logging.WARNING) Log.w(Logging.TAG, "could not layout status: " + clientSetupStatus);
    		break;
    	}
    	
    }
    
    private void setupActionBar() {
    	actionBar.show();
    	if(!actionBarSetupCompleted) {
	    	// setup action bar
    		if(Logging.DEBUG) Log.d(Logging.TAG, "setupActionBar()");
    		
	        actionBar.setDisplayShowTitleEnabled(false);
	        
	        actionBar.addTab(createTab(new BOINCActivityTabListener<TasksFragment>(this, getString(R.string.tab_tasks), TasksFragment.class), R.string.tab_tasks, R.drawable.tabtask));
	        actionBar.addTab(createTab(new BOINCActivityTabListener<ProjectsFragment>(this, getString(R.string.tab_projects), ProjectsFragment.class), R.string.tab_projects, R.drawable.projects));
	        actionBar.addTab(createTab(new BOINCActivityTabListener<NoticesFragment>(this, getString(R.string.tab_notices), NoticesFragment.class), R.string.tab_notices, R.drawable.mailw));
	        
	        // set navigation mode AFTER adding tabs to prevent list view
	        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
	        
	        actionBarSetupCompleted = true;
    	}
    }
    
    private Tab createTab(BOINCActivityTabListener<?> listener, int titleStringId, int iconDrawableId) {
        Tab tab = actionBar.newTab()
                .setTabListener(listener)
                .setCustomView(R.layout.main_tab_layout);
        
        TextView title = (TextView)tab.getCustomView().findViewById(R.id.tabsText);
        title.setText(titleStringId);
        ImageView icon = (ImageView)tab.getCustomView().findViewById(R.id.tabsIcon);
        icon.setImageDrawable(getResources().getDrawable(iconDrawableId));
        
        return tab;
    	
    }
    
    /*
    // set app title to status string of ClientStatus
    private void setAppTitle() {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"TasksActivity: Could not load data, clientStatus not initialized.");
			return;
		}
		TextView statusTV = (TextView) findViewById(R.id.titleStatus);
		statusTV.setText(status.getCurrentStatusString());
    }*/

	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onCreateOptionsMenu()");

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main_menu, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onCreateOptionsMenu()");
		// run mode, set title and icon based on status
		MenuItem runMode = menu.findItem(R.id.run_mode);
		if(clientComputingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
			// display play button
			runMode.setTitle(R.string.menu_run_mode_enable);
			runMode.setIcon(R.drawable.playw);
		} else {
			// display stop button
			runMode.setTitle(R.string.menu_run_mode_disable);
			runMode.setIcon(R.drawable.pausew);
		}
		
		return super.onPrepareOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onOptionsItemSelected()");

	    switch (item.getItemId()) {
	    	case R.id.help:
	    		Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse("http://boinc.berkeley.edu/wiki/BOINC_Help"));
	    		startActivity(i);
	    		return true;
	    	case R.id.about:
				final Dialog dialog = new Dialog(this);
				dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
				dialog.setContentView(R.layout.dialog_about);
				Button returnB = (Button) dialog.findViewById(R.id.returnB);
				TextView tvVersion = (TextView)dialog.findViewById(R.id.version);
				try {
					tvVersion.setText(getString(R.string.about_version) + " "
							+ getPackageManager().getPackageInfo(getPackageName(), 0).versionName);
				} catch (NameNotFoundException e) {if(Logging.WARNING) Log.w(Logging.TAG, "version name not found.");}
				
				returnB.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						dialog.dismiss();
					}
				});
				dialog.show();
	    		return true;
			case R.id.exit_boinc:
				if(Logging.DEBUG) Log.d(Logging.TAG,"exit BOINC");
				new QuitClientAsync().execute();
				return true;
			case R.id.run_mode:
				if(item.getTitle().equals(getApplication().getString(R.string.menu_run_mode_disable))) {
					if(Logging.DEBUG) Log.d(Logging.TAG,"run mode: disable");
					new WriteClientModeAsync().execute(BOINCDefs.RUN_MODE_NEVER);
				} else if (item.getTitle().equals(getApplication().getString(R.string.menu_run_mode_enable))) {
					if(Logging.DEBUG) Log.d(Logging.TAG,"run mode: enable");
					new WriteClientModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
				} else if(Logging.DEBUG) Log.d(Logging.TAG,"run mode: unrecognized command");
				return true;
			case R.id.event_log:
				startActivity(new Intent(this,EventLogActivity.class));
				return true;
			case R.id.projects_add:
				startActivity(new Intent(this, AttachProjectListActivity.class));
				return true;
			case R.id.prefs:
				startActivity(new Intent(this, PrefsActivity.class));
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}

	// monitor.quitClient is blocking (Thread.sleep)
	// execute in AsyncTask to maintain UI responsiveness
	private final class QuitClientAsync extends AsyncTask<Void, Void, Void> {

		@Override
		protected Void doInBackground(Void... params) {
			monitor.quitClient();
			return null;
		}
	}
	
	private final class WriteClientModeAsync extends AsyncTask<Integer, Void, Boolean> {
		
		@Override
		protected Boolean doInBackground(Integer... params) {
			// setting provided mode for both, CPU computation and network.
			Boolean runMode = monitor.clientInterface.setRunMode(params[0]);
			Boolean networkMode = monitor.clientInterface.setNetworkMode(params[0]);
			return runMode && networkMode;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"setting run and network mode failed");
		}
	}
}
