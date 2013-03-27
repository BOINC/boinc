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
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Result;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.widget.ListView;
import android.widget.TextView;

public class TasksActivity extends Activity {
	
	private final String TAG = "BOINC TasksActivity";
	
	private ListView lv;
	private TasksListAdapter listAdapter;
	
	private ArrayList<Result> data = new ArrayList<Result>(); //Adapter for list data
	
	// Controls whether initialization of view elements of "tasks_layout"
	// is required. This is the case, every time the layout switched.
	private Boolean initialSetupRequired = true;

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		
		private final String TAG = "TasksActivity-Receiver";
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG,"onReceive");
			populateLayout();
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        
        Log.d(TAG,"onCreate");
	}
	
	public void onResume() {
		super.onResume();
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		Log.d(TAG+"-onResume","register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		populateLayout();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	
	private void populateLayout() {
		try {
			//setup list and adapter
			ArrayList<Result> tmpA = Monitor.getClientStatus().getTasks();
			
			if(tmpA == null) {
				setLayoutLoading();
				return;
			}
			
			//deep copy, so ArrayList adapter actually recognizes the difference
			data.clear();
			for (Result tmp: tmpA) {
				data.add(tmp);
			}
			
			if(initialSetupRequired) {// first time we got proper results, setup adapter
				initialSetupRequired = false;
				setContentView(R.layout.tasks_layout); 
				lv = (ListView) findViewById(R.id.tasksList);
		        listAdapter = new TasksListAdapter(TasksActivity.this,R.id.tasksList,data);
		        lv.setAdapter(listAdapter);
			} 
		
			Log.d(TAG,"loadData: array contains " + data.size() + " results.");
			listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void setLayoutLoading() {
		setContentView(R.layout.generic_layout_loading); 
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.tasks_loading);
        initialSetupRequired = true;
	}
}
