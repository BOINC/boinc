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
import java.lang.StringBuffer;

import edu.berkeley.boinc.adapter.EventLogListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
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
import android.view.MenuItem;
import android.view.MenuInflater;
import android.widget.ListView;


public class EventLogActivity extends FragmentActivity {

	private final String TAG = "BOINC EventLogActivity";
		
	private Monitor monitor;
	private Boolean mIsBound;

	private ListView lv;
	private EventLogListAdapter listAdapter;
	private ArrayList<Message> data = new ArrayList<Message>();
	private int lastSeqno = 0;

	// Controls when to display the proper messages activity, by default we display a
	// view that says we are loading messages.  When initialSetup is false, we have
	// something to display.
	//
	private Boolean initialSetup; 
	
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
			
			// Read messages from state saved in ClientStatus
			ArrayList<Message> tmpA = Monitor.getClientStatus().getMessages(); 
			if(tmpA == null) {
				return;
			}

			// Switch to a view that can actually display messages
			if (initialSetup) {
				initialSetup = false;
				setContentView(R.layout.eventlog_layout); 
				lv = (ListView) findViewById(R.id.eventlogList);
		        listAdapter = new EventLogListAdapter(EventLogActivity.this, lv, R.id.eventlogList, data);
		    }
			
			// Add new messages to the event log
			for (Message msg: tmpA) {
				if (msg.seqno > lastSeqno) {
					data.add(msg);
					lastSeqno = msg.seqno; 
				}
			}
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
		}
	};

	
	//
	// Message Activity
	//
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
		
		// Switch to the loading view until we have something to display
		initialSetup = true;
		setContentView(R.layout.eventlog_layout_loading); 
		
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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    Log.d(TAG, "onCreateOptionsMenu()");

	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.eventlog_menu, menu);

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
		Boolean copySelection = false;
		
		// Determine what kind of email operation we are going to use
	    for (int index = 0; index < lv.getCount(); index++) {
	    	if (lv.isItemChecked(index)) {
	    		copySelection = true;
	    	}
	    }

	    // Put together the email intent		
		emailIntent.setType("plain/text");
		emailIntent.putExtra(android.content.Intent.EXTRA_SUBJECT, "Event Log for BOINC on Android");

		// Construct the message body
		emailText.append("\n\nContents of the Event Log:\n\n");
		if (copySelection) {
			// Copy selected items
		    for (int index = 0; index < lv.getCount(); index++) {
		    	if (lv.isItemChecked(index)) {
					emailText.append(listAdapter.getDate(index));
					emailText.append("|");
					emailText.append(listAdapter.getProject(index));
					emailText.append("|");
					emailText.append(listAdapter.getMessage(index));
					emailText.append("\r\n");
		    	}
		    }
		} else {
			// Copy all items
		    for (int index = 0; index < lv.getCount(); index++) {
				emailText.append(listAdapter.getDate(index));
				emailText.append("|");
				emailText.append(listAdapter.getProject(index));
				emailText.append("|");
				emailText.append(listAdapter.getMessage(index));
				emailText.append("\r\n");
			}
		}
		
		emailIntent.putExtra(android.content.Intent.EXTRA_TEXT, emailText.toString());
		
		// Send it off to the Activity-Chooser
		startActivity(Intent.createChooser(emailIntent, "Send mail..."));		

	}
}
