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

import edu.berkeley.boinc.adapter.MessagesListAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.widget.ListView;


public class MsgsActivity extends Activity {

	private final String TAG = "BOINC MsgsActivity";
	
	private ListView lv;
	private MessagesListAdapter listAdapter;
	private ArrayList<Message> data = new ArrayList<Message>();

	
	/*
	 * Receiver is necessary, because writing of prefs has to be done asynchroneously. 
	 * PrefsActivity will change to "loading" layout, until monitor read new results.
	 */
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "ClientStatusChange - onReceive()");
			
			// Read messages from state saved in ClientStatus
			ArrayList<Message> tmpA = Monitor.getClientStatus().getMessages(); 
			if(tmpA == null) {
				return;
			}
			
			// Deep copy, so ArrayList adapter actually recognizes the difference
			data.clear();
			for (Message tmp: tmpA) {
				data.add(tmp);
			}
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

	
	/*
	 * Message Activity
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    Log.d(TAG, "onCreate()");

	    super.onCreate(savedInstanceState);

		setContentView(R.layout.msgs_layout); 
		lv = (ListView) findViewById(R.id.msgsList);
        listAdapter = new MessagesListAdapter(MsgsActivity.this, R.id.listview, data);
        lv.setAdapter(listAdapter);
	}

	@Override
	public void onPause() {
		Log.d(TAG, "onPause() - Unregister Receiver");

		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	@Override
	public void onResume() {
		Log.d(TAG, "onResume() - Register Receiver");

		super.onResume();
		registerReceiver(mClientStatusChangeRec, ifcsc);
	}
	
	@Override
	protected void onDestroy() {
	    Log.d(TAG, "onDestroy()");

	    super.onDestroy();
	}
	
}
