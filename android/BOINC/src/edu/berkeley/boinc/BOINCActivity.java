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
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.app.Dialog;
import android.app.Service;
import android.app.TabActivity;
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
import android.os.Bundle; 
import android.os.IBinder;
import android.util.Log;  
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import android.widget.TextView;

public class BOINCActivity extends TabActivity {
	
	private Monitor monitor;
	private Integer clientSetupStatus = ClientStatus.SETUP_STATUS_LAUNCHING;
	private Boolean intialStart = true;
	
	private Boolean mIsBound = false;
	
	private TabHost tabHost;
	private Resources res;
	
	// dummy jni to trigger PlayStore filter for CPU architecture
	static{
		System.loadLibrary("dummyjni");
	}
	private native String getDummyString();
	// ---

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
        if(Logging.DEBUG) Log.d(Logging.TAG, "BOINCActivity onCreate(), dummy jni: " + getDummyString()); 

        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.main);  
         
        //bind monitor service
        doBindService();
        
        // adapt to custom title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);
        
        // get tab host and setup layout
    	res = getResources();
    	tabHost = getTabHost();
        setupTabLayout();
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
	    layout();
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
    
    // tests whether status is available and whether it changed since the last event.
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
				setAppTitle();
			} 
    	} catch (Exception e) {}
    }
    
    private void layout() {
    	TabHost tabLayout = (TabHost) findViewById(android.R.id.tabhost);
    	LinearLayout loadingLayout = (LinearLayout) findViewById(R.id.main_loading);
    	LinearLayout errorLayout = (LinearLayout) findViewById(R.id.main_error);
    	//TextView noProjectWarning = (TextView) findViewById(R.id.noproject_warning);
    	HorizontalScrollView noProjectWarning = (HorizontalScrollView) findViewById(R.id.noproject_warning_wrapper);
    	TextView launchingHeader = (TextView) findViewById(R.id.loading_header);
    	switch (clientSetupStatus) {
    	case ClientStatus.SETUP_STATUS_AVAILABLE:
    		noProjectWarning.setVisibility(View.GONE);
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.GONE);
    		tabLayout.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_ERROR:
    		tabLayout.setVisibility(View.GONE); 
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_LAUNCHING:
    		tabLayout.setVisibility(View.GONE); 
        	errorLayout.setVisibility(View.GONE);
        	loadingLayout.setVisibility(View.VISIBLE);
        	launchingHeader.setText(R.string.status_launching);
    		break;
    	case ClientStatus.SETUP_STATUS_NOPROJECT:
    		loadingLayout.setVisibility(View.GONE);
        	errorLayout.setVisibility(View.GONE);
    		tabLayout.setVisibility(View.VISIBLE);
    		noProjectWarning.setVisibility(View.VISIBLE);
    		break;
    	case ClientStatus.SETUP_STATUS_CLOSING:
    		tabLayout.setVisibility(View.GONE); 
        	errorLayout.setVisibility(View.GONE);
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
    
    /*
     * setup tab layout.
     * which tabs should be set up is defined in resources file: /res/values/configuration.xml
     */
    private void setupTabLayout() {
    	// set tabs
    	if(res.getBoolean(R.bool.tab_status))
    		setupTab(new TextView(this), getResources().getString(R.string.tab_status), R.drawable.icon_status_tab, StatusActivity.class);
    	if(res.getBoolean(R.bool.tab_projects))
    		setupTab(new TextView(this), getResources().getString(R.string.tab_projects), R.drawable.icon_projects_tab, ProjectsActivity.class);
    	if(res.getBoolean(R.bool.tab_tasks))
    		setupTab(new TextView(this), getResources().getString(R.string.tab_tasks), R.drawable.icon_tasks_tab, TasksActivity.class);
    	if(res.getBoolean(R.bool.tab_transfers))
    		setupTab(new TextView(this), getResources().getString(R.string.tab_transfers), R.drawable.icon_trans_tab, TransActivity.class);
    	if(res.getBoolean(R.bool.tab_preferences))
    		setupTab(new TextView(this), getResources().getString(R.string.tab_preferences), R.drawable.icon_prefs_tab, PrefsActivity.class);

        if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity tab layout setup done");
    }
    
    private void setupTab(final View view, final String tag, int icon, Class<?> target) {
    	View tabview = createTabView(tabHost.getContext(), tag, icon);
        TabSpec tabSpec = tabHost.newTabSpec(tag);
        tabSpec.setIndicator(tabview);
        tabSpec.setContent(new Intent(this, target));
    	tabHost.addTab(tabSpec);
    }

    private static View createTabView(final Context context, final String text, int icon) {
    	View view = LayoutInflater.from(context).inflate(R.layout.main_tab_layout, null);
    	TextView tv = (TextView) view.findViewById(R.id.tabsText);
    	tv.setText(text);
    	ImageView iv = (ImageView) view.findViewById(R.id.tabsIcon);
    	iv.setImageResource(icon);
    	return view;
    }
    
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
    }

	// triggered by click on noproject_warning, starts login activity
	public void noProjectClicked(View view) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "noProjectClicked()");
		startActivity(new Intent(this, AttachProjectListActivity.class));
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "BOINCActivity onCreateOptionsMenu()");

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main_menu, menu);

		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"TasksActivity: Could not load data, clientStatus not initialized.");
			return false;
		}
		// run mode, set title and icon based on status
		MenuItem runMode = menu.findItem(R.id.run_mode);
		if(status.computingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
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
			Boolean runMode = monitor.setRunMode(params[0]);
			Boolean networkMode = monitor.setNetworkMode(params[0]);
			return runMode && networkMode;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"setting run and network mode failed");
		}
	}
}
