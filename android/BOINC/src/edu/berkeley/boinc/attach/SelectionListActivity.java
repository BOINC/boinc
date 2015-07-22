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

package edu.berkeley.boinc.attach;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import java.util.ArrayList;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.ProjectInfo;
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
import android.os.RemoteException;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.Toast;

public class SelectionListActivity extends FragmentActivity{

	private ListView lv;
	ArrayList<ProjectListEntry> entries = new ArrayList<ProjectListEntry>();
	ArrayList<ProjectInfo> selected = new ArrayList<ProjectInfo>();
	
	// services
	private IMonitor monitor = null;
	private boolean mIsBound = false;
	private ProjectAttachService attachService = null;
	private boolean asIsBound = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
         
        if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity onCreate"); 
        
        doBindService();
		
		// setup layout
        setContentView(R.layout.attach_project_list_layout);  
		lv = (ListView) findViewById(R.id.listview);
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectListActivity onDestroy");
    	doUnbindService();
	    super.onDestroy();
	}
	
	// check whether user has checked at least a single project
	// shows toast otherwise
	private Boolean checkProjectChecked() {
		for(ProjectListEntry tmp: entries) {
			if(tmp.checked) return true;
		}
    	Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_list_header, Toast.LENGTH_SHORT);
    	toast.show();
    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity no project selected, stop!"); 
		return false;
	}
	
	// check whether device is online before starting connection attempt
	// as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
	// note: available internet does not guarantee connection to project server
	// is possible!
	private Boolean checkDeviceOnline() {
	    ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
	    NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
	    Boolean online = activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
	    if(!online) {
	    	Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT);
	    	toast.show();
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity not online, stop!"); 
	    }
	    return online;
	}
	
	// triggered by continue button
	public void continueClicked(View v) {
		if(!checkProjectChecked()) return;
		if(!checkDeviceOnline()) return;
		
		String selectedProjectsDebug = "";
		// get selected projects
		selected.clear();
		for(ProjectListEntry tmp: entries) {
			if(tmp.checked) {
				selected.add(tmp.info);
				selectedProjectsDebug += tmp.info.name + ",";
			}
		}
		if(Logging.DEBUG) Log.d(Logging.TAG, "SelectionListActivity: selected projects: " + selectedProjectsDebug);
		
		attachService.setSelectedProjects(selected); // returns immediately
		
		// start credential input activity
		startActivity(new Intent(this, CredentialInputActivity.class));
	}
	
	private ServiceConnection mMonitorConnection = new ServiceConnection() {
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
	
	private ServiceConnection mASConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        attachService = ((ProjectAttachService.LocalBinder)service).getService();
		    asIsBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	// This should not happen
	    	attachService = null;
	    	asIsBound = false;
	    }
	};
	
	private void doBindService() {
		// start service to allow setForeground later on...
		startService(new Intent(this, Monitor.class));
	    // Establish a connection with the service, onServiceConnected gets called when
		bindService(new Intent(this, Monitor.class), mMonitorConnection, Service.BIND_AUTO_CREATE);
		// bind to attach service
		bindService(new Intent(this, ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
	}

	private void doUnbindService() {
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mMonitorConnection);
	        mIsBound = false;
	    }
	    if (asIsBound) {
	        // Detach existing connection.
	        unbindService(mASConnection);
	        asIsBound = false;
	    }
	}
	
	private class UpdateProjectListAsyncTask extends AsyncTask<Void, Void, ArrayList<ProjectInfo>> {

		@Override
		protected ArrayList<ProjectInfo> doInBackground(Void... arg0) {
			
			ArrayList<ProjectInfo> data = null;
			Boolean retry = true;
			while(retry) {
				try{data = (ArrayList<ProjectInfo>) monitor.getAttachableProjects();} catch (RemoteException e){}
				if(data == null) {
					if(Logging.WARNING) Log.w(Logging.TAG,"UpdateProjectListAsyncTask: failed to retrieve data, retry....");
					try{Thread.sleep(500);} catch(Exception e) {}
				} else retry = false;
			}
			if(Logging.DEBUG) Log.d( Logging.TAG,"monitor.getAttachableProjects returned with " + data.size() + " elements");
			return data;
		}
		
		protected void onPostExecute(ArrayList<ProjectInfo> result) {
	        if (result != null) {
	        	entries.clear();
	        	for(ProjectInfo tmp: result) {
	        		entries.add(new ProjectListEntry(tmp));
	        	}

	        	entries.add(new ProjectListEntry()); // add account manager option to bottom of list
		        SelectionListAdapter listAdapter = new SelectionListAdapter(SelectionListActivity.this,R.id.listview,entries);
		        lv.setAdapter(listAdapter);
	         } 
	    }
	}
	
	class ProjectListEntry {
		public ProjectInfo info;
		public boolean checked;
		public boolean am; //indicates that element is account manager entry
		
		public ProjectListEntry(ProjectInfo info) {
			this.info = info;
			this.checked = false;
		}
		
		/**
		 * Creates Account manager list object
		 */
		public ProjectListEntry() {
			this.am = true;
		}
	}
}
