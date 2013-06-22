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
import edu.berkeley.boinc.adapter.TasksListAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.BOINCDefs;
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
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

public class TasksActivity extends FragmentActivity {

	private Monitor monitor;
	private Boolean mIsBound = false;

	private ClientStatus status; //client status, new information gets parsed by monitor, changes notified by "clientstatus" broadcast. read Result from here, to get information about tasks.
	
	private ListView lv;
	private TasksListAdapter listAdapter;
	private FragmentActivity activity = this;
	
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
		
		//private final String TAG = "TasksActivity-Receiver";
		@Override
		public void onReceive(Context context,Intent intent) {
			//if(Logging.DEBUG) Log.d(Logging.TAG,"TasksActivity onReceive");
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
	}
	
	public void onResume() {
		super.onResume();
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		if(Logging.DEBUG) Log.d(Logging.TAG,"TasksActivity register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		loadData();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		if(Logging.DEBUG) Log.d(Logging.TAG,"TasksActivity remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	@Override
	protected void onDestroy() {
		if(Logging.VERBOSE) Log.v(Logging.TAG, "TasksActivity onDestroy()");

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
		        lv.setOnItemClickListener(itemClickListener);

				setup = true;
			}
		
			//if(Logging.DEBUG) Log.d(Logging.TAG,"loadData: data set contains " + data.size() + " results.");
			listAdapter.notifyDataSetChanged(); //force list adapter to refresh
		
		} else {
			if(Logging.WARNING) Log.w(Logging.TAG, "loadData: array is null, rpc failed");
		}
	}
	
	private void updateData(ArrayList<Result> newData) {
		//loop through all received Result items to add new results
		for(Result rpcResult: newData) {
			//check whether this Result is new
			Integer index = null;
			for(int x = 0; x < data.size(); x++) {
				if(rpcResult.name.equals(data.get(x).id)) {
					index = x;
					continue;
				}
			}
			if(index == null) { // result is new, add
				if(Logging.DEBUG) Log.d(Logging.TAG,"new result found, id: " + rpcResult.name);
				data.add(new TaskData(rpcResult));
			} else { // result was present before, update its data
				data.get(index).updateResultData(rpcResult);
			}
		}
		
		//loop through the list adapter to find removed (ready/aborted) Results
		// use iterator to safely remove while iterating
		Iterator<TaskData> iData = data.iterator();
		while(iData.hasNext()) {
			Boolean found = false;
			TaskData listItem = iData.next();
			for(Result rpcResult: newData) {
				if(listItem.id.equals(rpcResult.name)) {
					found = true;
					continue;
				}
			}
			if(!found) iData.remove();
		}
	}

	public class TaskData {
		public Result result = null;
		public boolean expanded = false;
		public String id = "";
		public int nextState = -1;
		public int loopCounter = 0;
		public int transistionTimeout = 10; // amount of refresh, until transition times out

		public TaskData(Result data) {
			this.result = data;
			this.expanded = false;
			this.id = data.name;
			this.transistionTimeout = getResources().getInteger(R.integer.tasks_transistion_timeout_number_monitor_loops);
		}
		
		public void updateResultData(Result data) {
			this.result = data;
			Integer currentState = determineState();
			if (nextState == -1) return;
			if(currentState == nextState) {
				if(Logging.DEBUG) Log.d(Logging.TAG,"nextState met! " + nextState);
				nextState = -1;
				loopCounter = 0;
			} else {
				if(loopCounter<transistionTimeout) {
					if(Logging.DEBUG) Log.d(Logging.TAG,"nextState not met yet! " + nextState + " vs " + currentState + " loopCounter: " + loopCounter);
					loopCounter++;
				} else {
					if(Logging.DEBUG) Log.d(Logging.TAG,"transition timed out! " + nextState + " vs " + currentState + " loopCounter: " + loopCounter);
					nextState = -1;
					loopCounter = 0;
				}
			}
		}

		public final OnClickListener iconClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				try {
					final Integer operation = (Integer)v.getTag();
					switch(operation) {
					case RpcClient.RESULT_SUSPEND:
						nextState = BOINCDefs.RESULT_SUSPENDED_VIA_GUI;
						new ResultOperationAsync().execute(result.project_url, result.name, operation.toString());
						break;
					case RpcClient.RESULT_RESUME:
						nextState = BOINCDefs.PROCESS_EXECUTING;
						new ResultOperationAsync().execute(result.project_url, result.name, operation.toString());
						break;
					case RpcClient.RESULT_ABORT:
						final Dialog dialog = new Dialog(activity);
						dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
						dialog.setContentView(R.layout.dialog_confirm);
						Button confirm = (Button) dialog.findViewById(R.id.confirm);
						TextView tvTitle = (TextView)dialog.findViewById(R.id.title);
						TextView tvMessage = (TextView)dialog.findViewById(R.id.message);
						
						tvTitle.setText(R.string.confirm_abort_task_title);
						tvMessage.setText(getString(R.string.confirm_abort_task_message) + " "
								+ result.name);
						confirm.setText(R.string.confirm_abort_task_confirm);
						confirm.setOnClickListener(new OnClickListener() {
							@Override
							public void onClick(View v) {
								nextState = BOINCDefs.RESULT_ABORTED;
								new ResultOperationAsync().execute(result.project_url, result.name, operation.toString());
								dialog.dismiss();
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
						break;
					default:
						if(Logging.WARNING) Log.w(Logging.TAG,"could not map operation tag");
					}
					listAdapter.notifyDataSetChanged(); //force list adapter to refresh
				} catch (Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"failed parsing view tag");}
			}
		};
		
		public int determineState() {
			if(result.suspended_via_gui) return BOINCDefs.RESULT_SUSPENDED_VIA_GUI;
			if(result.project_suspended_via_gui) return BOINCDefs.RESULT_PROJECT_SUSPENDED;
			if(result.ready_to_report && result.state != BOINCDefs.RESULT_ABORTED && result.state != BOINCDefs.RESULT_COMPUTE_ERROR) return BOINCDefs.RESULT_READY_TO_REPORT;
			if(result.active_task){
				return result.active_task_state;
			} else {
				return result.state;
			}
		}
		
		public boolean isTaskActive() {
			return result.active_task;
		}
	}
	
	public final OnItemClickListener itemClickListener = new OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> arg0, View view, int position, long arg3) {
			TaskData task = listAdapter.getItem(position);
			task.expanded = !task.expanded;
			listAdapter.notifyDataSetChanged();
		}
	};
	
	private final class ResultOperationAsync extends AsyncTask<String,Void,Boolean> {

		@Override
		protected Boolean doInBackground(String... params) {
			try{
				String url = params[0];
				String name = params[1];
				Integer operation = Integer.parseInt(params[2]);
				if(Logging.DEBUG) Log.d(Logging.TAG,"url: " + url + " Name: " + name + " operation: " + operation);
	
				if(mIsBound) return monitor.resultOperation(url, name, operation);
				else return false;
			} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"SuspendResultAsync error in do in background",e);}
			return false;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"SuspendResultAsync failed.");
		}
	}
}
