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

import edu.berkeley.boinc.adapter.TasksListAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;

public class TasksActivity extends FragmentActivity {
	
	private final String TAG = "BOINC TasksActivity";

	private Monitor monitor;
	private Boolean mIsBound;

	private ClientStatus status; //client status, new information gets parsed by monitor, changes notified by "clientstatus" broadcast. read Result from here, to get information about tasks.
	
	private ListView lv;
	private TasksListAdapter listAdapter;
	
	private ArrayList<TaskData> data = new ArrayList<TaskData>(); //Adapter for list data
	private Boolean setup = false;

	// This is called when the connection with the service has been established,
	// getService returns the Monitor object that is needed to call functions.
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

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		
		private final String TAG = "TasksActivity-Receiver";
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG,"onReceive");
			loadData();
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.tasks_layout);
		// Establish a connection with the service, onServiceConnected gets called when
		// (calling within Tab needs getApplicationContext() for bindService to work!)
		getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Service.START_STICKY_COMPATIBILITY);

		//get singleton client status from monitor
		status = Monitor.getClientStatus();

		//load data model
		loadData();

		Log.d(TAG, "onCreate");
	}
	
	public void onResume() {
		super.onResume();
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		Log.d(TAG+"-onResume","register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		loadData();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
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
	
	private void loadData() {
		//setup list and adapter
		ArrayList<Result> tmpA = status.getTasks();
		if(tmpA!=null) { //can be null before first monitor status cycle (e.g. when not logged in or during startup)
		
			//deep copy, so ArrayList adapter actually recognizes the difference
			updateData(tmpA);

			if(!setup) { //first time we got proper results, setup adapter
				lv = (ListView) findViewById(R.id.tasksList);
			        listAdapter = new TasksListAdapter(TasksActivity.this,R.id.tasksList,data);
		        	lv.setAdapter(listAdapter);

				setup = true;
			}
		
			Log.d(TAG,"loadData: array contains " + data.size() + " results.");
			listAdapter.notifyDataSetChanged(); //force list adapter to refresh
		
		} else {
			Log.d(TAG, "loadData array is null");
		}
	}

	private void updateData(ArrayList<Result> newData) {
		// Create a new task data list based on new data
		ArrayList<TaskData> tdl = new ArrayList<TaskData>();
		for (Result r : newData) {
			tdl.add(new TaskData(r));
		}
		// search for the same id in the old tasks
		// maybe tasks were rearranged or updated
		for (int i = 0; i < tdl.size(); i++) {
			int j;
			for (j = 0; j < data.size(); j++) {
				if (tdl.get(i).id.equals(data.get(j).id)) {
					// this task is old.
					// retrieve expansion state
					tdl.get(i).expanded = data.get(j).expanded;
					break;
				}
			}
			if (j == data.size()) {
				// this is a new task, so add it	
				data.add(tdl.get(i));
			}
		}
		// save new task data list
		data.clear();

		for (TaskData td: tdl) {
			data.add(td);
		}
	}

	public class TaskData {
		public static final int TASK_STATE_UNKNOWN = 0;
		public static final int TASK_STATE_PAUSED = 1;
		public static final int TASK_STATE_RUNNING = 2;

		public static final int TASK_STATE_ACTIVE = 10;
		public static final int TASK_STATE_ABORTED = 11;

		public Result result = null;
		public boolean expanded = false;
		public String id = "";
		public int nextRunState = TASK_STATE_UNKNOWN;
		public int currentRunState = TASK_STATE_UNKNOWN;
		public int nextAbortState = TASK_STATE_UNKNOWN;
		public int currentAbortState = TASK_STATE_UNKNOWN;

		public TaskData(Result data) {
			this.result = data;
			this.expanded = false;
			this.id = data.name;
			this.currentRunState = determineRunningState();
			this.nextRunState = this.currentRunState;
			this.currentAbortState = determineAbortState();
			this.nextAbortState = this.currentAbortState;
		}

		public final OnClickListener taskClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				expanded = !expanded;
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};

		public final OnClickListener abortClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				ConfirmationDialog cd = ConfirmationDialog.newInstance(
					getString(R.string.confirm_abort_task_title) + "?",
					getString(R.string.confirm_abort_task_message) + " " + result.name,
					getString(R.string.confirm_abort_task_confirm));
				cd.setConfirmationClicklistener(new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						nextAbortState = TASK_STATE_ABORTED;
						monitor.abortResultAsync(result.project_url, result.name);
					}
				});
				cd.show(getSupportFragmentManager(), "");
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};

		public final OnClickListener suspendClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				nextRunState = TASK_STATE_PAUSED;
				monitor.suspendResultAsync(result.project_url, result.name);
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};

		public final OnClickListener resumeClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				nextRunState = TASK_STATE_RUNNING;
				monitor.resumeResultAsync(result.project_url, result.name);
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};

		public int determineRunningState() {
			if(result.active_task) {
				if(result.active_task_state == BOINCDefs.PROCESS_EXECUTING) {
					//running
					return TASK_STATE_RUNNING;
				} else {
					//suspended - ready to run
					return TASK_STATE_PAUSED;
				}
			} else {
				//paused or stopped
				return TASK_STATE_PAUSED;
			}
		}

		public int determineAbortState() {
			if(!result.active_task) {
				switch (result.state) {
				case 6:
					return TASK_STATE_ABORTED;
				}
			}
			return TASK_STATE_ACTIVE;
		}
	}
}
