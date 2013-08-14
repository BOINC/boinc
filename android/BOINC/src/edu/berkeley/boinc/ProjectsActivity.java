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
import java.util.Iterator;

import android.app.Dialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
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
import android.view.Window;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import edu.berkeley.boinc.adapter.ProjectControlsListAdapter;
import edu.berkeley.boinc.adapter.ProjectsListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;


public class ProjectsActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView lv;
	private ProjectsListAdapter listAdapter;
	private ArrayList<ProjectData> data = new ArrayList<ProjectData>();
	private final FragmentActivity activity = this;
	private Integer numberProjects = 0;

	// controls popup dialog
	Dialog dialogControls;
	
	// Controls whether initialization of view elements of "projects_layout"
	// is required. This is the case, every time the layout switched.
	private Boolean initialSetupRequired = true; 
	
    // This is called when the connection with the service has been established, 
	// getService returns the Monitor object that is needed to call functions.
	//
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    if(Logging.VERBOSE) Log.v(Logging.TAG,"ProjectsActivity service bound");
	    }
		@Override
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
			//if(Logging.DEBUG) Log.d(Logging.TAG, "ClientStatusChange - onReceive()");
			populateLayout(false);
		}
	};

	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "ProjectsActivity onCreate()");

	    super.onCreate(savedInstanceState);

	    // Establish a connection with the service, onServiceConnected gets called when
	    // (calling within Tab needs getApplicationContext() for bindService to work!)
	    getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Service.START_STICKY_COMPATIBILITY);
	}
	
	@Override
	public void onPause() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "ProjectsActivity onPause()");

		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	@Override
	public void onResume() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "ProjectsActivity onResume()");
		super.onResume();
		
		populateLayout(true);

		registerReceiver(mClientStatusChangeRec, ifcsc);
	}
	
	@Override
	protected void onDestroy() {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "ProjectsActivity onDestroy()");

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
			
			// get server / scheduler notices to display if device does not meet
			// resource requirements
			ArrayList<Notice> serverNotices = null;
			if(mIsBound) serverNotices = monitor.getServerNotices();
			
			// Switch to a view that can actually display messages
			if (initialSetupRequired) {
				initialSetupRequired = false;
				setContentView(R.layout.projects_layout); 
				lv = (ListView) findViewById(R.id.projectsList);
		        listAdapter = new ProjectsListAdapter(ProjectsActivity.this, lv, R.id.projectsList, data);
		    }
			
			// Update Project data
			updateData(tmpA, serverNotices);
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void updateData(ArrayList<Project> newData, ArrayList<Notice> serverNotices) {
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
				if(Logging.DEBUG) Log.d(Logging.TAG,"new result found, id: " + rpcResult.master_url);
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
		
		// loop through active projects to add/remove server notices
		if(serverNotices != null) {
			int mappedServerNotices = 0;
			for(ProjectData project: data) {
				boolean noticeFound = false;
				for(Notice serverNotice: serverNotices) {
					if(project.project.project_name.equals(serverNotice.project_name)) {
						project.addServerNotice(serverNotice);
						noticeFound = true;
						mappedServerNotices++;
						continue;
					}
				}
				if(!noticeFound) project.addServerNotice(null);
			}
			if(mappedServerNotices != serverNotices.size()) if(Logging.WARNING) Log.w(Logging.TAG,"could not match notice: " + mappedServerNotices + "/" + serverNotices.size());
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
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "ProjectsActivity onCreateOptionsMenu()");

		// call BOINCActivity's onCreateOptionsMenu to combine both menus
		getParent().onCreateOptionsMenu(menu);

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.projects_menu, menu);

		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    if(Logging.VERBOSE) Log.v(Logging.TAG, "ProjectsActivity onOptionsItemSelected()");

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
		public String id = "";
		public Notice lastServerNotice = null;

		public ProjectData(Project data) {
			this.project = data;
			this.id = data.master_url;
		}
		
		public void updateProjectData(Project data) {
			this.project = data;
		}
		
		public void addServerNotice(Notice notice) {
			this.lastServerNotice = notice;
		}
		
		public Notice getLastServerNotice() {
			return lastServerNotice;
		}
		
		public final OnClickListener projectsListClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				dialogControls = new Dialog(activity);
				// layout
				dialogControls.requestWindowFeature(Window.FEATURE_NO_TITLE);
				dialogControls.setContentView(R.layout.dialog_list);
				((TextView)dialogControls.findViewById(R.id.title)).setText(R.string.projects_control_dialog_title);
				ListView list = (ListView)dialogControls.findViewById(R.id.options);
				
				// add control items depending on:
				// - client status, e.g. either suspend or resume
				// - show advanced preference
				ArrayList<ProjectControl> controls = new ArrayList<ProjectControl>();
				controls.add(new ProjectControl(project, RpcClient.PROJECT_UPDATE));
				if(project.suspended_via_gui) controls.add(new ProjectControl(project, RpcClient.PROJECT_RESUME));
				else controls.add(new ProjectControl(project, RpcClient.PROJECT_SUSPEND));
				if(Monitor.getAppPrefs().getShowAdvanced() && project.dont_request_more_work) controls.add(new ProjectControl(project, RpcClient.PROJECT_ANW));
				if(Monitor.getAppPrefs().getShowAdvanced() && !project.dont_request_more_work) controls.add(new ProjectControl(project, RpcClient.PROJECT_NNW));
				if(Monitor.getAppPrefs().getShowAdvanced()) controls.add(new ProjectControl(project, RpcClient.PROJECT_RESET));
				controls.add(new ProjectControl(project, RpcClient.PROJECT_DETACH));
				
				// list adapter
				list.setAdapter(new ProjectControlsListAdapter(activity,list,R.layout.projects_controls_listitem_layout,controls));
				if(Logging.DEBUG) Log.d(Logging.TAG,"dialog list adapter entries: " + controls.size());
				
				// buttons
				Button cancelButton = (Button) dialogControls.findViewById(R.id.cancel);
				cancelButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						dialogControls.dismiss();
					}
				});
				
				// show dialog
				dialogControls.show();
			}
		};
	}
	
	public class ProjectControl {
		public Integer operation;
		public Project project = null;
		
		public ProjectControl(Project project, Integer operation) {
			this.operation = operation;
			this.project = project;
		}

		public final OnClickListener projectCommandClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				//check whether command requires confirmation
				if(operation == RpcClient.PROJECT_DETACH || operation == RpcClient.PROJECT_RESET) {
					final Dialog dialog = new Dialog(activity);
					dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
					dialog.setContentView(R.layout.dialog_confirm);
					Button confirm = (Button) dialog.findViewById(R.id.confirm);
					TextView tvTitle = (TextView)dialog.findViewById(R.id.title);
					TextView tvMessage = (TextView)dialog.findViewById(R.id.message);
					
					// operation dependend texts
					if (operation == RpcClient.PROJECT_DETACH) {
						tvTitle.setText(R.string.projects_confirm_detach_title);
						tvMessage.setText(getString(R.string.projects_confirm_detach_message) + " "
								+ project.project_name + " " + getString(R.string.projects_confirm_detach_message2));
						confirm.setText(R.string.projects_confirm_detach_confirm);
					} else if(operation == RpcClient.PROJECT_RESET) {
						tvTitle.setText(R.string.projects_confirm_reset_title);
						tvMessage.setText(getString(R.string.projects_confirm_reset_message) + " "
								+ project.project_name + getString(R.string.projects_confirm_reset_message2));
						confirm.setText(R.string.projects_confirm_reset_confirm);
					}
					
					confirm.setOnClickListener(new OnClickListener() {
						@Override
						public void onClick(View v) {
							new ProjectOperationAsync().execute(project.master_url, "" + operation);
							dialog.dismiss();
							dialogControls.dismiss();
						}
					});
					Button cancel = (Button) dialog.findViewById(R.id.cancel);
					cancel.setOnClickListener(new OnClickListener() {
						@Override
						public void onClick(View v) {
							dialog.dismiss();
						}
					});
					dialog.show();
				} else { // command does not required confirmation
					new ProjectOperationAsync().execute(project.master_url, "" + operation);
					dialogControls.dismiss();
				}
			}
		};
	}
	
	private final class ProjectOperationAsync extends AsyncTask<String,Void,Boolean> {

		@Override
		protected void onPreExecute() {
			if(Logging.DEBUG) Log.d(Logging.TAG,"onPreExecute");
			super.onPreExecute();
		}

		@Override
		protected Boolean doInBackground(String... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"doInBackground");
			try{
				String url = params[0];
				Integer operation = Integer.parseInt(params[1]);
				if(Logging.DEBUG) Log.d(Logging.TAG,"url: " + url + " operation: " + operation + " monitor bound: " + mIsBound);
	
				if(mIsBound) return monitor.projectOperation(operation, url);
				else return false;
			} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"error in do in background",e);}
			return false;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"failed.");
		}
	}
}
