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
import java.util.Iterator;
import edu.berkeley.boinc.adapter.ProjectsListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;
import android.app.AlertDialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;
import android.widget.TextView;


public class ProjectsActivity extends FragmentActivity {
	
	private final String TAG = "BOINC ProjectsActivity";
	
	private Monitor monitor;
	private Boolean mIsBound;

	private ListView lv;
	private ProjectsListAdapter listAdapter;
	private ArrayList<ProjectData> data = new ArrayList<ProjectData>();
	private final FragmentActivity activity = this;
	private Integer numberProjects = 0;
	
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
			//Log.d(TAG, "ClientStatusChange - onReceive()");
			populateLayout(false);
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
		
		populateLayout(true);

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
	
	private void populateLayout(Boolean force) {
		try {
			// read projects from state saved in ClientStatus
			ArrayList<Project> tmpA = Monitor.getClientStatus().getProjects();
			
			if(tmpA == null) {
				setLayoutLoading();
				return;
			}
			
			// limit layout update on when project number changes.
			if(!force && tmpA.size() == numberProjects) return;

			// Switch to a view that can actually display messages
			if (initialSetupRequired) {
				initialSetupRequired = false;
				setContentView(R.layout.projects_layout); 
				lv = (ListView) findViewById(R.id.projectsList);
		        listAdapter = new ProjectsListAdapter(ProjectsActivity.this, lv, R.id.projectsList, data);
		    }
			
			// Update Project data
			updateData(tmpA);
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void updateData(ArrayList<Project> newData) {
		//loop through all received Result items to add new results
		for(Project rpcResult: newData) {
			//check whether this Result is new
			Integer index = null;
			for(int x = 0; x < data.size(); x++) {
				if(rpcResult.master_url.equals(data.get(x).id)) {
					index = x;
					continue;
				}
			}
			if(index == null) { // result is new, add
				Log.d(TAG,"new result found, id: " + rpcResult.master_url);
				data.add(new ProjectData(rpcResult));
			} else { // result was present before, update its data
				data.get(index).updateProjectData(rpcResult);
			}
		}
		
		//loop through the list adapter to find removed (ready/aborted) Results
		// use iterator to safely remove while iterating
		Iterator<ProjectData> iData = data.iterator();
		while(iData.hasNext()) {
			Boolean found = false;
			ProjectData listItem = iData.next();
			for(Project rpcResult: newData) {
				if(listItem.id.equals(rpcResult.master_url)) {
					found = true;
					continue;
				}
			}
			if(!found) iData.remove();
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
				addProject(null);
				return true;
			default:
				return getParent().onOptionsItemSelected(item); // if item id can not be mapped, call parents method
		}
	}
	
	// on click of project add button
	public void addProject(View view) {
		startActivity(new Intent(this,AttachProjectListActivity.class));
	}

	public class ProjectData {
		public Project project = null;
		public boolean expanded = false;
		public String id = "";

		public ProjectData(Project data) {
			this.project = data;
			this.expanded = false;
			this.id = data.master_url;
		}

		public final OnClickListener projectClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				expanded = !expanded;
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};
		
		public void updateProjectData(Project data) {
			this.project = data;
		}

		public final OnClickListener iconClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				try {
					final Integer operation = (Integer)v.getTag();
					switch(operation) {
					case RpcClient.PROJECT_UPDATE:
						new ProjectOperationAsync().execute(project.master_url, operation.toString());
						break;
					case RpcClient.PROJECT_DETACH:
						ConfirmationDialog cd = ConfirmationDialog.newInstance(
							getString(R.string.projects_confirm_detach_title) + "?",
							getString(R.string.projects_confirm_detach_message) + " " + project.project_name,
							getString(R.string.projects_confirm_detach_confirm));
						cd.setConfirmationClicklistener(new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_DETACH);
							}
						});
						cd.show(getSupportFragmentManager(), "");
						break;
					case RpcClient.PROJECT_ADVANCED:
						AlertDialog.Builder builder = new AlertDialog.Builder(activity);
					    builder.setTitle(R.string.projects_confirm_advanced_title);
					    builder.setItems(R.array.projects_advanced_controls, new DialogInterface.OnClickListener() {
					               public void onClick(DialogInterface dialog, int which) {
						                switch(which) {
						                case 0:
											new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_SUSPEND);
						                	break;
						                case 1:
											new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_RESUME);
						                	break;
						                case 2:
											new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_NNW);
						                	break;
						                case 3:
											new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_ANW);
						                	break;
						                case 4:
											new ProjectOperationAsync().execute(project.master_url, ""+RpcClient.PROJECT_RESET);
						                	break;
						                default:
						                	break;
						                }
					           }});
					    builder.create().show();
						break;
					default:
						Log.w(TAG,"could not map operation tag");
					}
					listAdapter.notifyDataSetChanged(); //force list adapter to refresh
				} catch (Exception e) {Log.w(TAG,"failed parsing view tag");}
			}
		};
	}
	
	private final class ProjectOperationAsync extends AsyncTask<String,Void,Boolean> {

		private final String TAG = "ProjectOperationAsync";

		@Override
		protected Boolean doInBackground(String... params) {
			try{
				String url = params[0];
				Integer operation = Integer.parseInt(params[1]);
				Log.d(TAG,"url: " + url + " operation: " + operation);
	
				if(mIsBound) return monitor.projectOperation(operation, url);
				else return false;
			} catch(Exception e) {Log.w(TAG,"error in do in background",e);}
			return false;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else Log.w(TAG,"failed.");
		}
	}
}
