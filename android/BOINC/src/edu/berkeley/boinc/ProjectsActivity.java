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
import edu.berkeley.boinc.rpc.AcctMgrInfo;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;

public class ProjectsActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView lv;
	private ProjectsListAdapter listAdapter;
	private ArrayList<ProjectsListData> data = new ArrayList<ProjectsListData>();
	private final FragmentActivity activity = this;

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
			populateLayout();
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
		
		populateLayout();

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
	
	private void populateLayout() {
		try {
			// read projects from state saved in ClientStatus
			ArrayList<Project> tmpA = Monitor.getClientStatus().getProjects();
			AcctMgrInfo tmpB = Monitor.getClientStatus().getAcctMgrInfo();
			
			if(tmpA == null || tmpB == null) {
				Boolean aNull = tmpA == null;
				Boolean bNull = tmpB == null;
				if(Logging.ERROR) Log.d(Logging.TAG,"ProjectsActiviy data retrieval failed: tmpA null: " + aNull + " ; tmpB null: " + bNull);
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
			updateData(tmpA, tmpB, serverNotices);
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
			if(Logging.ERROR) Log.d(Logging.TAG,"ProjectsActiviy data retrieval failed.");
		}
	}
	
	private void updateData(ArrayList<Project> latestRpcProjectsList, AcctMgrInfo acctMgrInfo, ArrayList<Notice> serverNotices) {
		
		//loop through list adapter array to find index of account manager entry (0 || 1 manager possible)
		int mgrIndex = -1;
		for(int x = 0; x < data.size(); x++) {
			if(data.get(x).isMgr) {
				mgrIndex = x;
				continue;
			}
		}
		if(mgrIndex < 0) { // no manager present until now
			if(Logging.VERBOSE) Log.d(Logging.TAG,"no manager found in layout list. new entry available: " + acctMgrInfo.present);
			if(acctMgrInfo.present) {
				// add new manager entry, at top of the list
				data.add(new ProjectsListData(null,acctMgrInfo));
				if(Logging.DEBUG) Log.d(Logging.TAG,"new acct mgr found: " + acctMgrInfo.acct_mgr_name);
			}
		} else { // manager found in existing list
			if(Logging.VERBOSE) Log.d(Logging.TAG,"manager found in layout list at index: " + mgrIndex);
			if(!acctMgrInfo.present) {
				// manager got detached, remove from list
				data.remove(mgrIndex);
				if(Logging.DEBUG) Log.d(Logging.TAG,"acct mgr removed from list.");
			}
		}
		
		//loop through all received Result items to add new results
		for(Project rpcResult: latestRpcProjectsList) {
			//check whether this Result is new
			Integer index = null;
			for(int x = 0; x < data.size(); x++) {
				if(rpcResult.master_url.equals(data.get(x).id)) {
					index = x;
					continue;
				}
			}
			if(index == null) { // result is new, add
				if(Logging.DEBUG) Log.d(Logging.TAG,"new result found, id: " + rpcResult.master_url + ", managed: " + rpcResult.attached_via_acct_mgr);
				if(rpcResult.attached_via_acct_mgr) data.add(new ProjectsListData(rpcResult,null)); // append to end of list (after manager)
				else data.add(0, new ProjectsListData(rpcResult,null)); // put at top of list (before manager)
			} else { // result was present before, update its data
				data.get(index).updateProjectData(rpcResult,null);
			}
		}
		
		//loop through the list adapter to find removed (ready/aborted) Results
		// use iterator to safely remove while iterating
		Iterator<ProjectsListData> iData = data.iterator();
		while(iData.hasNext()) {
			Boolean found = false;
			ProjectsListData listItem = iData.next();
			if(listItem.isMgr) continue;
			for(Project rpcResult: latestRpcProjectsList) {
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
			for(ProjectsListData project: data) {
				if(project.isMgr) continue; // do not seek notices in manager entries (crashes)
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

	// data wrapper for list view
	public class ProjectsListData {
		// can be either project or account manager
		public Project project = null;
		public Notice lastServerNotice = null;
		public AcctMgrInfo acctMgrInfo = null;
		public String id = ""; // == url
		public boolean isMgr;
		public ProjectsListData listEntry = this;

		public ProjectsListData(Project data, AcctMgrInfo acctMgrInfo) {
			this.project = data;
			this.acctMgrInfo = acctMgrInfo;
			if (this.project == null && this.acctMgrInfo != null) isMgr = true;
			if(isMgr) {
				this.id = acctMgrInfo.acct_mgr_url;
			} else {
				this.id = data.master_url;
			}
		}
		
		public void updateProjectData(Project data, AcctMgrInfo acctMgrInfo) {
			if(isMgr){
				this.acctMgrInfo = acctMgrInfo;
			} else {
				this.project = data;
			}
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
				ListView list = (ListView)dialogControls.findViewById(R.id.options);
				
				// add control items depending on:
				// - type, account manager vs. project
				// - client status, e.g. either suspend or resume
				// - show advanced preference
				// - project attached via account manager (e.g. hide Remove)
				ArrayList<ProjectControl> controls = new ArrayList<ProjectControl>();
				if(isMgr) {
					((TextView)dialogControls.findViewById(R.id.title)).setText(R.string.projects_control_dialog_title_acctmgr);
					
					controls.add(new ProjectControl(listEntry, ProjectControl.MGR_SYNC));
					controls.add(new ProjectControl(listEntry, ProjectControl.MGR_DETACH));
				} else {
					((TextView)dialogControls.findViewById(R.id.title)).setText(R.string.projects_control_dialog_title);
					
					controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_UPDATE));
					if(project.suspended_via_gui) controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_RESUME));
					else controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_SUSPEND));
					if(Monitor.getAppPrefs().getShowAdvanced() && project.dont_request_more_work) controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_ANW));
					if(Monitor.getAppPrefs().getShowAdvanced() && !project.dont_request_more_work) controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_NNW));
					if(Monitor.getAppPrefs().getShowAdvanced()) controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_RESET));
					if(!project.attached_via_acct_mgr)controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_DETACH));
				}
				
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
		public ProjectsListData data;
		public Integer operation;
		
		public static final int MGR_DETACH = -1;
		public static final int MGR_SYNC = -2;
		
		public ProjectControl(ProjectsListData data, Integer operation) {
			this.operation = operation;
			this.data = data;
		}

		public final OnClickListener projectCommandClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				
				//check whether command requires confirmation
				if(operation == RpcClient.PROJECT_DETACH
						|| operation == RpcClient.PROJECT_RESET
						|| operation == ProjectControl.MGR_DETACH) {
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
								+ data.project.project_name + " " + getString(R.string.projects_confirm_detach_message2));
						confirm.setText(R.string.projects_confirm_detach_confirm);
					} else if(operation == RpcClient.PROJECT_RESET) {
						tvTitle.setText(R.string.projects_confirm_reset_title);
						tvMessage.setText(getString(R.string.projects_confirm_reset_message) + " "
								+ data.project.project_name + getString(R.string.projects_confirm_reset_message2));
						confirm.setText(R.string.projects_confirm_reset_confirm);
					} else if(operation == ProjectControl.MGR_DETACH) {
						tvTitle.setText(R.string.projects_confirm_remove_acctmgr_title);
						tvMessage.setText(getString(R.string.projects_confirm_remove_acctmgr_message) + " "
								+ data.acctMgrInfo.acct_mgr_name + getString(R.string.projects_confirm_remove_acctmgr_message2));
						confirm.setText(R.string.projects_confirm_remove_acctmgr_confirm);
					}
					
					confirm.setOnClickListener(new OnClickListener() {
						@Override
						public void onClick(View v) {
							new ProjectOperationAsync().execute(data, operation);
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
					new ProjectOperationAsync().execute(data, operation);
					dialogControls.dismiss();
				}
			}
		};
	}
	
	private final class ProjectOperationAsync extends AsyncTask<Object,Void,Boolean> {

		@Override
		protected void onPreExecute() {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectOperationAsync onPreExecute");
			super.onPreExecute();
		}

		@Override
		protected Boolean doInBackground(Object... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectOperationAsync doInBackground");
			try{
				ProjectsListData data = (ProjectsListData) params[0];
				Integer operation = (Integer) params[1];
				if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectOperationAsync isMgr: " + data.isMgr + "url: " + data.id + " operation: " + operation + " monitor bound: " + mIsBound);
	
				if(mIsBound) {
					if(data.isMgr) {
						switch(operation) {
						case ProjectControl.MGR_SYNC:
							return monitor.synchronizeAcctMgr(data.acctMgrInfo.acct_mgr_url);
						case ProjectControl.MGR_DETACH:
							return monitor.addAcctMgr("", "", "").error_num == BOINCErrors.ERR_OK;
						}
						
					} else {
						return monitor.projectOperation(operation, data.id);
					}
				}
				else return false;
			} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"ProjectOperationAsync error in do in background",e);}
			return false;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"ProjectOperationAsync failed.");
		}
	}
}
