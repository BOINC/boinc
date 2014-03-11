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
import java.util.ArrayList;
import edu.berkeley.boinc.adapter.AttachProjectListAdapter;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.rpc.ProjectInfo;
import android.app.Dialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcelable;
import android.support.v4.app.NavUtils;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class AttachProjectListActivity extends ActionBarActivity implements android.view.View.OnClickListener{

	private ListView lv;
	private AttachProjectListAdapter listAdapter;
	private Dialog manualUrlInputDialog;
	private Boolean acctMgrPresent = false;
	private IMonitor monitor = null;
	private boolean mIsBound = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
         
        if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity onCreate"); 
        
        doBindService();
		//get supported projects
		// try to get current client status from monitor
		//ClientStatus status;
// Move to background update
//		ArrayList<ProjectInfo> data = new ArrayList<ProjectInfo>();
//		try{
//			//status  = Monitor.getClientStatus();
//			acctMgrPresent = SplashActivity.monitor.getAcctMgrInfoPresent();
//			data = (ArrayList<ProjectInfo>) SplashActivity.monitor.getSupportedProjects();
//			if(Logging.DEBUG) Log.d( Logging.TAG,"monitor.getAndroidProjectsList returned with " + data.size() + " elements");
//		} catch (Exception e){
//			if(Logging.WARNING) Log.w(Logging.TAG,"AttachProjectListActivity: Could not load supported projects, clientStatus not initialized.");
//			finish();
//			return;
//		}
		
		// setup layout
        setContentView(R.layout.attach_project_list_layout);  
		lv = (ListView) findViewById(R.id.listview);
        listAdapter = new AttachProjectListAdapter(AttachProjectListActivity.this,R.id.listview,new ArrayList<ProjectInfo>());
        lv.setAdapter(listAdapter);
        
        // setup action bar
        ActionBar actionBar = getSupportActionBar();
        actionBar.setTitle(R.string.attachproject_list_header);
        
        // disable up as home if explicitly requested in intent (e.g. when navigating directly from splash screen)
        Intent i = getIntent();
        actionBar.setDisplayHomeAsUpEnabled(i.getBooleanExtra("showUp", true));
        
        
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectListActivity onDestroy");
    	doUnbindService();
	    super.onDestroy();
	}	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectListActivity onCreateOptionsMenu()");

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.attach_list_menu, menu);

        // disable "add account manager" button, if account manager already present
        if(acctMgrPresent) {
        	MenuItem item = menu.findItem(R.id.acctmgr_add);
        	item.setVisible(false);
        }
        
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectListActivity onOptionsItemSelected()");

	    switch (item.getItemId()) {
	    	case R.id.acctmgr_add:
	    		Intent intent = new Intent(this, AttachProjectAcctMgrActivity.class);
	    		startActivity(intent);
	    		return true;
	    	case R.id.projects_add_url:
	    		showDialog(R.id.projects_add_url);
	    		return true;
	    	case android.R.id.home:
	    	    if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity onOptionsItemSelected(): navigate to logical parent");
	    		// navigate to logical parent (manifest) when home/up/appicon is clicked
	    	    NavUtils.navigateUpFromSameTask(this);
	            return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}
	
	// check whether device is online before starting connection attempt
	// as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
	// note: available internet does not imply connection to project server
	// is possible!
	private Boolean checkDeviceOnline() {
	    ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
	    NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
	    return activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
	}
	
	// gets called by showDialog
	@Override
	protected Dialog onCreateDialog(int id) {
		manualUrlInputDialog = new Dialog(this); //instance new dialog
		manualUrlInputDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
		manualUrlInputDialog.setContentView(R.layout.attach_project_list_layout_manual_dialog);
		Button button = (Button) manualUrlInputDialog.findViewById(R.id.buttonUrlSubmit);
		button.setOnClickListener(this);
		((TextView)manualUrlInputDialog.findViewById(R.id.title)).setText(R.string.attachproject_list_manual_dialog_title);
		return manualUrlInputDialog;
	}

	// gets called by dialog button
	@Override
	public void onClick(View v) {
		try {
			String url = ((EditText)manualUrlInputDialog.findViewById(R.id.Input)).getText().toString();

			if(url == null) { // error while parsing
				showErrorToast(R.string.attachproject_list_manual_no_url);
			}
			else if(url.length()== 0) { //nothing in edittext
				showErrorToast(R.string.attachproject_list_manual_no_url);
			}
			else if(!checkDeviceOnline()) {
				showErrorToast(R.string.attachproject_list_no_internet);
			} else {
				manualUrlInputDialog.dismiss();
				startAttachProjectLoginActivity(null, url);
			}
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG,"error parsing edit text",e);
		}
	}
	
	// gets called by project list item
	public void onProjectClick(View view) {
		if(!checkDeviceOnline()) {
			showErrorToast(R.string.attachproject_list_no_internet);
			return;
		}
		try {
			ProjectInfo project = (ProjectInfo) view.getTag();
			startAttachProjectLoginActivity(project, null); 
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG,"error parsing view tag",e);
			showErrorToast(R.string.attachproject_list_manual_no_url);
		}
	}
	
	private void startAttachProjectLoginActivity(ProjectInfo project, String url) {
		Intent intent = new Intent(this, AttachProjectLoginActivity.class);
		intent.putExtra("projectInfo", (Parcelable)project);
		intent.putExtra("url", url);
		startActivity(intent);
	}

	private void showErrorToast(int resourceId) {
		Toast toast = Toast.makeText(getApplicationContext(), resourceId, Toast.LENGTH_SHORT);
		toast.show();
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        monitor = IMonitor.Stub.asInterface(service);
		    mIsBound = true;
		    
			UpdateProjectListAsyncTask task = new UpdateProjectListAsyncTask();
			task.execute();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	// This should not happen
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
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
	
	private class UpdateProjectListAsyncTask extends AsyncTask<Void, Void, ArrayList<ProjectInfo>> {

		@Override
		protected ArrayList<ProjectInfo> doInBackground(Void... arg0) {
			
			ArrayList<ProjectInfo> data = null;
			while(acctMgrPresent == null || data == null) {
				try{
					//status  = Monitor.getClientStatus();
					acctMgrPresent = monitor.getAcctMgrInfoPresent();
					data = (ArrayList<ProjectInfo>) monitor.getSupportedProjects();
					if(Logging.DEBUG) Log.d( Logging.TAG,"monitor.getAndroidProjectsList returned with " + data.size() + " elements");
				} catch (Exception e){
					if(Logging.WARNING) Log.w(Logging.TAG,"AttachProjectListActivity: Could not load supported projects: " + e.getLocalizedMessage());
					finish();
				}
				if(acctMgrPresent == null || data == null) {
					if(Logging.WARNING) Log.w(Logging.TAG,"AttachProjectListActivity: UpdateProjectListAsyncTask failed to retrieve data, retry....");
					try{Thread.sleep(500);} catch(Exception e) {}
				}
			}
			return data;
		}
		
		protected void onPostExecute(ArrayList<ProjectInfo> result) {
	        if (listAdapter!=null && result != null) {
				for(ProjectInfo tmp: result) { // addAll only in API 11
					listAdapter.add(tmp);
				}
	         }
	        	 
	     }
	}

}
