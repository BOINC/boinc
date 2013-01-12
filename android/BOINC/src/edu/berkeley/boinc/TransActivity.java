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

import edu.berkeley.boinc.adapter.TransListAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Transfer;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.widget.ListView;

public class TransActivity extends Activity {
	
	private final String TAG = "TransActivity";
	
	private ClientStatus status; //client status, new information gets parsed by monitor, changes notified by "clientstatus" broadcast. read Result from here, to get information about tasks.

	private ListView lv;
	private TransListAdapter listAdapter;
	
	private ArrayList<Transfer> data = new ArrayList<Transfer>(); //Adapter for list data
	private Boolean setup = false;

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		
		private final String TAG = "TransActivity-Receiver";
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG,"onReceive");
			loadData(); // refresh list view
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.trans_layout); 
		
		//get singleton client status from monitor
		status = Monitor.getClientStatus();
		
        //load data model
		loadData();
		
        Log.d(TAG,"onCreate");
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
	
	
	private void loadData() {
		//setup list and adapter
		ArrayList<Transfer> tmpA = status.getTransfers();
		if(tmpA!=null) { //can be null before first monitor status cycle (e.g. when not logged in or during startup)
				
			//deep copy, so ArrayList adapter actually recognizes the difference
			data.clear();
			for (Transfer tmp: tmpA) {
				data.add(tmp);
			}
		
			if(!setup) {// first time we got proper results, setup adapter
				lv = (ListView) findViewById(R.id.transList);
		        listAdapter = new TransListAdapter(TransActivity.this,R.id.transList,data);
		        lv.setAdapter(listAdapter);
		        
		        setup = true;
			}
		
			Log.d(TAG,"loadData: array contains " + data.size() + " results.");
			listAdapter.notifyDataSetChanged(); //force list adapter to refresh
		} else {
			Log.d(TAG, "loadData array is null");
		}
		
	}
}
