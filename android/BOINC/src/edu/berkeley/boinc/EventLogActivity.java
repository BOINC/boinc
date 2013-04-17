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
import java.util.List;
import java.lang.StringBuffer;
import edu.berkeley.boinc.adapter.EventLogListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.ListView;
import android.widget.TextView;


public class EventLogActivity extends FragmentActivity {

	private final String TAG = "BOINC EventLogActivity";
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView lv;
	private EventLogListAdapter listAdapter;
	private ArrayList<Message> data = new ArrayList<Message>();
	
	// message retrieval
	private Integer pastMsgsLoadingRange = 50; // amount messages loaded when end of list is reached
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    Log.d(TAG, "onCreate()");

		doBindService();
		setLayoutLoading();
		
	    super.onCreate(savedInstanceState);
	}
	
	@Override
	public void onResume() {
		Log.d(TAG, "onResume()");

		super.onResume();
		
		new RetrieveRecentMsgs().execute(); // refresh messages
	}
	
	@Override
	protected void onDestroy() {
	    Log.d(TAG, "onDestroy()");
	    doUnbindService();
	    super.onDestroy();
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    initializeLayout();
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
	
	// updates data list with most recent messages
	private void loadRecentMsgs(ArrayList<Message> tmpA) {
		// Prepend new messages to the event log
		try {
			int y = 0;
			for (int x = tmpA.size()-1; x >= 0; x--) {
				data.add(y, tmpA.get(x));
				y++;
			}
		} catch (Exception e) {} //IndexOutOfBoundException
		listAdapter.notifyDataSetChanged();
	}
	
	// appends older messages to data list
	private void loadPastMsgs(List<Message> tmpA) {
		// Append old messages to the event log
		try {
			for(int x = tmpA.size()-1; x >= 0; x--) {
				data.add(tmpA.get(x));
			}
		} catch (Exception e) {} //IndexOutOfBoundException
		
		listAdapter.notifyDataSetChanged();
	}
	
	private void initializeLayout() {
		try {
			// check whether monitor is bound
			if(!mIsBound) {
				setLayoutLoading();
				return;
			}
				
			setContentView(R.layout.eventlog_layout); 
			lv = (ListView) findViewById(R.id.eventlogList);
		    listAdapter = new EventLogListAdapter(EventLogActivity.this, lv, R.id.eventlogList, data);
		    lv.setOnScrollListener(new EndlessScrollListener(5));
			
			// initial data retrieval
			new RetrievePastMsgs().execute();
		} catch (Exception e) {
			// data retrieval failed, set layout to loading...
			setLayoutLoading();
		}
	}
	
	private void setLayoutLoading() {
		setContentView(R.layout.generic_layout_loading); 
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.eventlog_loading);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    Log.d(TAG, "onCreateOptionsMenu()");

		// call BOINCActivity's onCreateOptionsMenu to combine both menus
		getParent().onCreateOptionsMenu(menu);
		
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
				return getParent().onOptionsItemSelected(item);
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
	
	// onScrollListener for list view, implementing "endless scrolling"
	public final class EndlessScrollListener implements OnScrollListener {

        private int visibleThreshold = 5;
        private int previousTotal = 0;
        private boolean loading = true;

        public EndlessScrollListener(int visibleThreshold) {
            this.visibleThreshold = visibleThreshold;
        }

        @Override
        public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount) {
            if (loading) {
                if (totalItemCount > previousTotal) {
                    loading = false;
                    previousTotal = totalItemCount;
                }
            }
            if (!loading && (totalItemCount - visibleItemCount) <= (firstVisibleItem + visibleThreshold)) {
                new RetrievePastMsgs().execute();
                loading = true;
            }
        }

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
        }
    }
	
	private final class RetrieveRecentMsgs extends AsyncTask<Void,Void,ArrayList<Message>> {
		
		private Integer mostRecentSeqNo = 0;

		@Override
		protected void onPreExecute() {
			if(!mIsBound) cancel(true); // cancel execution if monitor is not bound yet
			try {
				mostRecentSeqNo = data.get(0).seqno;
			} catch (Exception e) {} //IndexOutOfBoundException
		}
		
		@Override
		protected ArrayList<Message> doInBackground(Void... params) {
			return monitor.getEventLogMessages(mostRecentSeqNo); 
		}

		@Override
		protected void onPostExecute(ArrayList<Message> result) {
			// back in UI thread
			loadRecentMsgs(result);
		}
	}
	
	private final class RetrievePastMsgs extends AsyncTask<Void,Void,List<Message>> {
		
		private Integer mostRecentSeqNo = null;
		private Integer pastSeqNo = null;

		@Override
		protected void onPreExecute() {
			if(!mIsBound) cancel(true); // cancel execution if monitor is not bound yet
			try {
				mostRecentSeqNo = data.get(0).seqno;
				pastSeqNo = data.get(data.size()-1).seqno;
				Log.d("RetrievePastMsgs","mostRecentSeqNo: " + mostRecentSeqNo + " ; pastSeqNo: " + pastSeqNo);
				if(pastSeqNo==0) {
					Log.d("RetrievePastMsgs", "cancel, all past messages are present");
					cancel(true); // cancel if all past messages are present
				}
			} catch (Exception e) {} //IndexOutOfBoundException
		}
		
		@Override
		protected List<Message> doInBackground(Void... params) {
			Integer startIndex = 0;
			if(mostRecentSeqNo != null && pastSeqNo != null && mostRecentSeqNo != 0 && pastSeqNo != 0) startIndex = mostRecentSeqNo - pastSeqNo + 1;
			Integer lastIndexOfList = 0;
			if(mostRecentSeqNo != null) lastIndexOfList = mostRecentSeqNo - 1;
			//Log.d("RetrievePastMsgs", "calling monitor with: " + startIndex + lastIndexOfList);
			return monitor.getEventLogMessages(startIndex, pastMsgsLoadingRange, lastIndexOfList); 
		}

		@Override
		protected void onPostExecute(List<Message> result) {
			// back in UI thread
			loadPastMsgs(result);
		}
	}
}
