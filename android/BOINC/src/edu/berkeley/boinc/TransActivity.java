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
import java.util.Calendar;
import java.util.Iterator;
import edu.berkeley.boinc.adapter.TransListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.utils.BOINCErrors;
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
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

public class TransActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView lv;
	private TransListAdapter listAdapter;
	private ArrayList<TransferData> data = new ArrayList<TransferData>();
	private CcStatus status;
	private FragmentActivity activity = this;
	
	// Controls whether initialization of view elements of "projects_layout"
	// is required. This is the case, every time the layout switched.
	private Boolean setup = false; 
	
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
			if(Logging.VERBOSE) Log.v(Logging.TAG, "TransActivity ClientStatusChange - onReceive()");
			
			populateLayout();
		}
	};
	
	public void onCreate(Bundle savedInstanceState) {
	    if(Logging.DEBUG) Log.d(Logging.TAG, "TransActivity onCreate()");

	    super.onCreate(savedInstanceState);

	    // Establish a connection with the service, onServiceConnected gets called when
	    // (calling within Tab needs getApplicationContext() for bindService to work!)
		getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Service.START_STICKY_COMPATIBILITY);
	}
	
	public void onPause() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "TransActivity onPause()");

		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	public void onResume() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "TransActivity onResume()");

		super.onResume();
		
		// Switch to the loading view until we have something to display
		populateLayout();

		registerReceiver(mClientStatusChangeRec, ifcsc);
	}
	
	@Override
	protected void onDestroy() {
	    if(Logging.DEBUG) Log.d(Logging.TAG, "TransActivity onDestroy()");

	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	    
	    super.onDestroy();
	}
	
	private void populateLayout() {
		try {
			// Read transfers from state saved in ClientStatus
			ArrayList<Transfer> tmpA = Monitor.getClientStatus().getTransfers(); 
			
			// Read core client status (net up/down, cpu suspended, network suspended) from 
			// state saved in ClientStatus
			status = Monitor.getClientStatus().getClientStatus();
			
			if(tmpA == null || status == null) {
				setLayoutLoading();
				return;
			}
			
			//deep copy, so ArrayList adapter actually recognizes the difference
			updateData(tmpA);

			// Switch to a view that can actually display messages
			if (!setup) {
				setup = true;
				setContentView(R.layout.trans_layout); 
				lv = (ListView) findViewById(R.id.transList);
		        listAdapter = new TransListAdapter(TransActivity.this, lv, R.id.projectsList, data, status);
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
        loadingHeader.setText(R.string.trans_loading);
        setup = false;
	}
	
	public void onTransferClicked(String url, String name) {
	    if(Logging.DEBUG) Log.d(Logging.TAG, "onTransferClicked()");
	}
	
	public void onTransferRetry(String url, String name) {
	    if(Logging.DEBUG) Log.d(Logging.TAG, "onTransferRetry()");
	    monitor.retryTransferAsync(url, name);
	}
	
	private void updateData(ArrayList<Transfer> newData) {
		//loop through all received Result items to add new results
		for(Transfer transfer: newData) {
			//check whether this Result is new
			Integer index = null;
			for(int x = 0; x < data.size(); x++) {
				if(transfer.name.equals(data.get(x).id)) {
					index = x;
					continue;
				}
			}
			if(index == null) { // result is new, add
				if(Logging.DEBUG) Log.d(Logging.TAG,"new transfer found, id: " + transfer.name);
				data.add(new TransferData(transfer));
			} else { // result was present before, update its data
				data.get(index).updateTransferData(transfer);
			}
		}
		//loop through the list adapter to find removed (ready/aborted) transfers
		// use iterator to safely remove while iterating
		Iterator<TransferData> iData = data.iterator();
		while(iData.hasNext()) {
			Boolean found = false;
			TransferData listItem = iData.next();
			for(Transfer rpcResult: newData) {
				if(listItem.id.equals(rpcResult.name)) {
					found = true;
					continue;
				}
			}
			
			if(!found) iData.remove();
		}
	}
	
	public class TransferData {
		public Transfer transfer = null;
		public boolean expanded = false;
		public String id = "";
		public int expectedState = -1;
		public int loopCounter = 0;
		public int transistionTimeout = 10; // amount of refresh, until transition times out
		
		public static final int TRANSFER_ABORTED = 0;
		public static final int TRANSFER_ONGOING = 1;
		public static final int TRANSFER_SUSPENDED = 2;
		public static final int TRANSFER_RETRYING = 3;
		public static final int TRANSFER_FAILED = 4;
		
		public TransferData(Transfer data) {
			this.transfer = data;
			this.expanded = false;
			this.id = data.name;
			this.transistionTimeout = getResources().getInteger(R.integer.tasks_transistion_timeout_number_monitor_loops);
		}
		
		public void updateTransferData(Transfer data) {
			this.transfer = data;
			Integer currentState = determineState();
			if (expectedState == -1) return;
			if(currentState == expectedState) {
				if(Logging.DEBUG) Log.d(Logging.TAG,"expectedState met! " + expectedState);
				expectedState = -1;
				loopCounter = 0;
			} else {
				if(loopCounter<transistionTimeout) {
					if(Logging.DEBUG) Log.d(Logging.TAG,"expectedState not met yet! " + expectedState + " vs " + currentState + " loopCounter: " + loopCounter);
					loopCounter++;
				} else {
					if(Logging.DEBUG) Log.d(Logging.TAG,"transition timed out! " + expectedState + " vs " + currentState + " loopCounter: " + loopCounter);
					expectedState = -1;
					loopCounter = 0;
				}
			}
		}
		
		public final OnClickListener transClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				expanded = !expanded;
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		};
		
		public final OnClickListener iconClickListener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				try {
					final Integer operation = (Integer)v.getTag();
					switch(operation) {
					case RpcClient.TRANSFER_RETRY:
						expectedState = TRANSFER_ONGOING;
						new TransferOperationAsync().execute(transfer.project_url, transfer.name, operation.toString());
						break;
					case RpcClient.TRANSFER_ABORT:
						final Dialog dialog = new Dialog(activity);
						dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
						dialog.setContentView(R.layout.dialog_confirm);
						Button confirm = (Button) dialog.findViewById(R.id.confirm);
						TextView tvTitle = (TextView)dialog.findViewById(R.id.title);
						TextView tvMessage = (TextView)dialog.findViewById(R.id.message);
						
						tvTitle.setText(R.string.confirm_abort_trans_title);
						tvMessage.setText(getString(R.string.confirm_abort_trans_message) + " "
								+ transfer.name);
						confirm.setText(R.string.confirm_abort_trans_confirm);
						confirm.setOnClickListener(new OnClickListener() {
							@Override
							public void onClick(View v) {
								expectedState = TRANSFER_ABORTED;
								new TransferOperationAsync().execute(transfer.project_url, transfer.name, operation.toString());
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
			
			Calendar nextRequest = Calendar.getInstance();
			Calendar now = Calendar.getInstance();
			nextRequest.setTimeInMillis((long)transfer.next_request_time*1000);
			
			if (nextRequest.compareTo(now) > 0) {
				return TRANSFER_RETRYING;
			} else if (transfer.status == BOINCErrors.ERR_GIVEUP_DOWNLOAD || transfer.status == BOINCErrors.ERR_GIVEUP_UPLOAD) {
				return TRANSFER_FAILED;
			} else {
				if (status.network_suspend_reason > 0) {
					return TRANSFER_SUSPENDED;
				} else {
					return TRANSFER_ONGOING;
				}
			}
		}
	}
	
	private final class TransferOperationAsync extends AsyncTask<String,Void,Boolean> {
		
		private final String TAG = "TransferOperationAsync";
		
		@Override
		protected Boolean doInBackground(String... params) {
			try{
				String url = params[0];
				String name = params[1];
				Integer operation = Integer.parseInt(params[2]);
				Log.d(TAG,"url: " + url + " Name: " + name + " operation: " + operation);
				
				if(mIsBound) return monitor.transferOperation(url, name, operation);
				else return false;
			} catch(Exception e) {Log.w(TAG,"error in do in background",e);}
			
			return false;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else Log.w(TAG,"failed.");
		}
	}
}
