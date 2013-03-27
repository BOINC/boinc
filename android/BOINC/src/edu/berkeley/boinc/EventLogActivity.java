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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.widget.ListView;
import android.widget.TextView;


public class EventLogActivity extends FragmentActivity {

	private final String TAG = "BOINC EventLogActivity";

	private ListView lv;
	private EventLogListAdapter listAdapter;
	private ArrayList<Message> data = new ArrayList<Message>();
	private int lastSeqno = 0;

	// Controls whether initialization of view elements of "projects_layout"
	// is required. This is the case, every time the layout switched.
	private Boolean initialSetupRequired = true; 
	
	// BroadcastReceiver event is used to update the UI with updated information from 
	// the client.  This is generally called once a second.
	//
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "ClientStatusChange - onReceive()");
			
			populateLayout();
		}
	};

	
	//
	// Message Activity
	//
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    Log.d(TAG, "onCreate()");

	    super.onCreate(savedInstanceState);
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
		
		populateLayout();
		
		registerReceiver(mClientStatusChangeRec, ifcsc);
	}
	
	@Override
	protected void onDestroy() {
	    Log.d(TAG, "onDestroy()");
	    
	    super.onDestroy();
	}
	
	private void populateLayout() {
		try {
			// Read messages from state saved in ClientStatus
			ArrayList<Message> tmpA = Monitor.getClientStatus().getMessages(); 
			
			if(tmpA == null) {
				setLayoutLoading();
				return;
			}

			// Switch to a view that can actually display messages
			if (initialSetupRequired) {
				initialSetupRequired = false;
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
			
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void setLayoutLoading() {
		setContentView(R.layout.generic_layout_loading); 
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.eventlog_loading);
        initialSetupRequired = true;
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
