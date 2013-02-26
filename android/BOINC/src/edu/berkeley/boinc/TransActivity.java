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
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.Transfer;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
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
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.widget.ListView;

public class TransActivity extends FragmentActivity {
	
	private final String TAG = "BOINC TransActivity";
	
	private Monitor monitor;
	private Boolean mIsBound;

	private ListView lv;
	private TransListAdapter listAdapter;
	private ArrayList<Transfer> data = new ArrayList<Transfer>();
	private CcStatus status;
	

	// Controls when to display the proper projects activity, by default we display a
	// view that says we are loading projects.  When initialSetup is false, we have
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
			
			// Read transfers from state saved in ClientStatus
			ArrayList<Transfer> tmpA = Monitor.getClientStatus().getTransfers(); 
			if(tmpA == null) {
				return;
			}
			
			// Read core client status (net up/down, cpu suspended, network suspended) from 
			// state saved in ClientStatus
			status = Monitor.getClientStatus().getClientStatus();

			// Switch to a view that can actually display messages
			if (initialSetup) {
				initialSetup = false;
				setContentView(R.layout.trans_layout); 
				lv = (ListView) findViewById(R.id.transList);
		        listAdapter = new TransListAdapter(TransActivity.this, lv, R.id.projectsList, data, status);
		    }
			
			// Update Transfer data
			data.clear();
			for (Transfer tmp: tmpA) {
				data.add(tmp);
			}
			
			// Force list adapter to refresh
			listAdapter.notifyDataSetChanged(); 
		}
	};
	
	public void onCreate(Bundle savedInstanceState) {
	    Log.d(TAG, "onCreate()");

	    super.onCreate(savedInstanceState);

	    // Establish a connection with the service, onServiceConnected gets called when
	    // (calling within Tab needs getApplicationContext() for bindService to work!)
		getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Service.START_STICKY_COMPATIBILITY);
	}
	
	public void onPause() {
		Log.d(TAG, "onPause()");

		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	public void onResume() {
		Log.d(TAG, "onResume()");

		super.onResume();
		
		// Switch to the loading view until we have something to display
		initialSetup = true;
		setContentView(R.layout.trans_layout_loading);

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
	
	public void onTransferClicked(String url, String name) {
	    Log.d(TAG, "onTransferClicked()");
	}
	
	public void onTransferRetry(String url, String name) {
	    Log.d(TAG, "onTransferRetry()");
	    monitor.retryTransferAsync(url, name);
	}
	
	public void onTransferAbort(String url, String name) {
	    Log.d(TAG, "ononTransferAbort() - Name: " + name + ", URL: " + url);
		(new ConfirmAbortDialogFragment(name, url)).show(getSupportFragmentManager(), "confirm_transfer_abort");
	}
	
	public class ConfirmAbortDialogFragment extends DialogFragment {
		
		private final String TAG = "ConfirmAbortDialogFragment";
		
		private String url = "";
		private String name = "";
		
		public ConfirmAbortDialogFragment(String url, String name) {
			this.url = url;
			this.name = name;
		}
		
	    @Override
	    public Dialog onCreateDialog(Bundle savedInstanceState) {
	        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
	        String dialogTitle = getString(R.string.confirm_abort) + " " + name + "?";
	        builder.setMessage(dialogTitle)
	               .setPositiveButton(R.string.confirm_abort_confirm, new DialogInterface.OnClickListener() {
	                   public void onClick(DialogInterface dialog, int id) {
	                       Log.d(TAG, "confirm clicked.");
	                       //asynchronous call to detach project with given url.
	                       monitor.abortTransferAsync(url, name);
	                   }
	               })
	               .setNegativeButton(R.string.confirm_abort_cancel, new DialogInterface.OnClickListener() {
	                   public void onClick(DialogInterface dialog, int id) {
	                       Log.d(TAG, "dialog canceled.");
	                   }
	               });
	        // Create the AlertDialog object and return it
	        return builder.create();
	    }
	}
}
