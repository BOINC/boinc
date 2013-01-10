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
import java.util.ListIterator;

import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.receiver.LogReceiver;
import edu.berkeley.boinc.rpc.Message;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class DebugActivity extends Activity {
	
	private final String TAG = "BOINC DebugActivity";
	
	private ClientStatus status; //client status, new information gets parsed by monitor, changes notified by "clientstatus" broadcast. read Result from here, to get information about tasks.
	
	private TextView clientLog;
	private TextView managerLog;
	private Integer logHubPointer = 0;

	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private ArrayList<Message> msgs;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.debug_layout);
	
		//get monitor
		doBindService();

		//get singleton client status from monitor
		status = Monitor.getClientStatus();
		
		//configure layout
		TextView clHeader = (TextView)this.findViewById(R.id.clientLogHeader);
		clHeader.setText("BOINC client messages:");
		clientLog=(TextView)this.findViewById(R.id.clientLog);
		TextView mlHeader = (TextView)this.findViewById(R.id.managerLogHeader);
		mlHeader.setText("AndroidBOINC manager messages:");
		managerLog=(TextView)this.findViewById(R.id.managerLog);
		Log.d(TAG,"onCreate finished");
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 * currently in Prefs- and DebugActivity
	 * 
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
	        mIsBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
		}
	}

	private void doUnbindService() {
	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	public void onResume() {
		super.onResume();
		
		populateText();
	}
	
	public void clear(View view) {
		Log.d(TAG,"clear");
		clientLog.setText("");
		managerLog.setText("");
		logHubPointer = 0;
		LogReceiver.logHub.clear();
		if(status!=null) {
			status.resetMessages();
		}
	}
	
	public void refresh(View view) {
		Log.d(TAG,"refresh");
		populateText();
	}
	
	public void killClient(View view) {
		Log.d(TAG,"killClient");
		if(!mIsBound) {
			Log.d(TAG, "sorry, service not bound yet");
			return;
		}
		monitor.quitClient();
	}
	
	private void populateText() {
		Log.d(TAG,"populateText");
		
		//write client messages
		if(status!=null) {
			msgs = status.getMessages();
			if(msgs!=null) {
				clientLog.setText("");
				for (Message msg: msgs) {
					clientLog.append(msg.body+"\n");
				}
			}
		}
		
		//write manager messages
		ListIterator<String> hubIt = LogReceiver.logHub.listIterator(logHubPointer);
		while(hubIt.hasNext()){
			String message = hubIt.next();
			Log.d(TAG,"new Entry: " + message);
			managerLog.append(message+"\n");
			logHubPointer++;
		}
	}
	
	@Override
	protected void onDestroy() {
	    super.onDestroy();
	    Log.d(TAG,"onDestroy()");
	    doUnbindService();
	}
}
