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
import java.util.Date;
import java.lang.StringBuffer;

import edu.berkeley.boinc.adapter.MessagesListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.widget.ListView;


public class MsgsActivity extends Activity {

	private final String TAG = "BOINC MsgsActivity";
	
	private ListView lv;
	private MessagesListAdapter listAdapter;
	private ArrayList<Message> data = new ArrayList<Message>();

	
	/*
	 * Receiver is necessary, because writing of preferences has to be done asynchronously. 
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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    Log.d(TAG, "onCreateOptionsMenu()");
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.msgs_menu, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    Log.d(TAG, "onOptionsItemSelected()");
		switch (item.getItemId()) {
			case R.id.email_to:
				onEmailTo();
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}
	
	public void onEmailTo() {

		Intent emailIntent = new Intent(android.content.Intent.ACTION_SEND);
		StringBuffer emailText = new StringBuffer();
		
		emailIntent.setType("plain/text");
		emailIntent.putExtra(android.content.Intent.EXTRA_SUBJECT, "Event Log for BOINC on Android");

		// Construct the message body
		emailText.append("\n\nContents of the Event Log:\n\n");
		for (Message msg: data) {
			emailText.append(new Date(msg.timestamp).toString());
			emailText.append("|");
			emailText.append(msg.project);
			emailText.append("|");
			emailText.append(msg.body);
			emailText.append("\n");
		}
		
		emailIntent.putExtra(android.content.Intent.EXTRA_TEXT, emailText.toString());
		
		// Send it off to the Activity-Chooser
		startActivity(Intent.createChooser(emailIntent, "Send mail..."));		

	}
}
