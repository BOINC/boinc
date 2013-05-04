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

import java.util.ArrayList;

import edu.berkeley.boinc.adapter.ProjectsListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


public class ProjectsActivity extends FragmentActivity {
	
	private final String TAG = "BOINC ProjectsActivity";
	
	private Monitor monitor;
	private Boolean mIsBound;

	private ListView lv;
	private ProjectsListAdapter listAdapter;
	private ArrayList<Project> data = new ArrayList<Project>();
	
	// Controls whether initialization of view elements of "projects_layout"
	// is required. This is the case, every time the layout switched.
	private Boolean initialSetupRequired = true; 
	
    // This is called when the connection with the service has been established, 
	// getService returns the Monitor object that is needed to call functions.
	//
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
	// BroadcastReceiver event is used to update the UI with updated information from 
	// the client.  This is generally called once a second.
	//
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "ClientStatusChange - onReceive()");
			populateLayout();
		}
	};

	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    Log.d(TAG, "onCreate()");

	    super.onCreate(savedInstanceState);

	    // Establish a connection with the service, onServiceConnected gets called when
	    // (calling within Tab needs getApplicationContext() for bindService to work!)
		getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Service.START_STICKY_COMPATIBILITY);
	}
	
	@Override
	public void onPause() {
		Log.d(TAG, "onPause()");

		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	@Override
	public void onResume() {
		Log.d(TAG, "onResume()");

		super.onResume();
		
		populateLayout();

		registerReceiver(mClientStatusChangeRec, ifcsc);
	}
	
	@Override
	protected void onDestroy() {
	    Log.d(TAG, "onDestroy()");

	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	    
	    super.onDestroy();
	}
	
	private void populateLayout() {
		try {
			// read projects from state saved in ClientStatus
			ArrayList<Project> tmpA = Monitor.getClientStatus().getProjects();
			
			if(tmpA == null) {
				setLayoutLoading();
				return;
			}

			// Switch to a view that can actually display messages
			if (initialSetupRequired) {
				initialSetupRequired = false;
				setContentView(R.layout.projects_layout); 
				lv = (ListView) findViewById(R.id.projectsList);
		        listAdapter = new ProjectsListAdapter(ProjectsActivity.this, lv, R.id.projectsList, data);
		    }
			
			// Update Project data
			data.clear();
			for (Project tmp: tmpA) {
				data.add(tmp);
			}
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void setLayoutLoading() {
		setContentView(R.layout.generic_layout_loading); 
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.projects_loading);
        initialSetupRequired = true;
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    Log.d(TAG, "onCreateOptionsMenu()");

		// call BOINCActivity's onCreateOptionsMenu to combine both menus
		getParent().onCreateOptionsMenu(menu);

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.projects_menu, menu);

		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    Log.d(TAG, "onOptionsItemSelected()");

	    switch (item.getItemId()) {
			case R.id.projects_add:
				onProjectAdd();
				return true;
			default:
				return getParent().onOptionsItemSelected(item); // if item id can not be mapped, call parents method
		}
	}
	
	public void onProjectClicked(String url, String name) {
	    Log.d(TAG, "onProjectClicked()");
	}

	public void addProjectClicked(View view) {
		onProjectAdd();
	}
	
	public void onProjectAdd() {
		Log.d(TAG, "onProjectAdd()");
		startActivity(new Intent(this,AttachProjectListActivity.class));
	}

	public void onProjectUpdate(String url, String name) {
	    Log.d(TAG, "onProjectUpdate()");
	    monitor.updateProjectAsync(url);
	}
	
	public void onProjectMore(String url, String name) {
	    Log.d(TAG, "onProjectMore() - Name: " + name + ", URL: " + url);
		Toast toast = Toast.makeText(getApplicationContext(), "not implemented yet...", Toast.LENGTH_LONG);
		toast.show();
	}
}
