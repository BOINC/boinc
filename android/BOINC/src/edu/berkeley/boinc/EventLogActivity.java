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
import java.util.List;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.StringBuffer;
import edu.berkeley.boinc.adapter.ClientLogListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.text.ClipboardManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.view.View;
import android.view.Window;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


public class EventLogActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView clientLogList;
	private ClientLogListAdapter clientLogListAdapter;
	private ArrayList<Message> clientLogData = new ArrayList<Message>();

	private ListView guiLogList;
	private ArrayAdapter<String> guiLogListAdapter;
	private ArrayList<String> guiLogData = new ArrayList<String>();
	
	// message retrieval
	private Integer pastMsgsLoadingRange = 50; // amount messages loaded when end of list is reached
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);

		doBindService();
		setLayoutLoading();
        
        // adapt to custom title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);

	    super.onCreate(savedInstanceState);
	}

	@Override
	public void onResume() {
		if(Logging.VERBOSE) Log.v(Logging.TAG, "EventLogActivity onResume()");

		super.onResume();
		
		new RetrieveRecentClientMsgs().execute(); // refresh messages
	}
	
	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	if(Logging.VERBOSE) Log.v(Logging.TAG,"EventLogActivity onServiceConnected");
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
				clientLogData.add(y, tmpA.get(x));
				y++;
			}
		} catch (Exception e) {} //IndexOutOfBoundException
		clientLogListAdapter.notifyDataSetChanged();
	}
	
	// appends older messages to data list
	private void loadPastMsgs(List<Message> tmpA) {
		// Append old messages to the event log
		try {
			for(int x = tmpA.size()-1; x >= 0; x--) {
				clientLogData.add(tmpA.get(x));
			}
		} catch (Exception e) {} //IndexOutOfBoundException
		
		clientLogListAdapter.notifyDataSetChanged();
	}
	
	private void initializeLayout() {
		try {
			// check whether monitor is bound
			if(!mIsBound) {
				setLayoutLoading();
				return;
			}
				
			setContentView(R.layout.eventlog_layout); 
			
			clientLogList = (ListView) findViewById(R.id.clientLogList);
			clientLogListAdapter = new ClientLogListAdapter(EventLogActivity.this, clientLogList, R.id.clientLogList, clientLogData);
			clientLogList.setOnScrollListener(new EndlessScrollListener(5));
			
			guiLogList = (ListView) findViewById(R.id.guiLogList);
			guiLogListAdapter = new ArrayAdapter<String>(EventLogActivity.this, R.layout.eventlog_gui_listitem_layout, guiLogData);
			guiLogList.setAdapter(guiLogListAdapter);
			
			// initial data retrieval
			new RetrievePastClientMsgs().execute();
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
	    MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.eventlog_menu, menu);

		return true;
	}
	
	public void onClientLog(View v) {
		//adapt header
		v.setBackgroundColor(getResources().getColor(android.R.color.transparent));
		TextView guiLogHeader = (TextView) findViewById(R.id.guiLogHeader);
		guiLogHeader.setBackgroundResource(R.drawable.shape_light_blue_background);
		// change lists
		guiLogList.setVisibility(View.GONE);
		clientLogList.setVisibility(View.VISIBLE);
	}
	
	public void onGuiLog(View v) {
		//adapt header
		v.setBackgroundColor(getResources().getColor(android.R.color.transparent));
		TextView clientLogHeader = (TextView) findViewById(R.id.clientLogHeader);
		clientLogHeader.setBackgroundResource(R.drawable.shape_light_blue_background);
		// change lists
		clientLogList.setVisibility(View.GONE);
		guiLogList.setVisibility(View.VISIBLE);
		// read messages
		readLogcat();
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
			case R.id.refresh:
				new RetrieveRecentClientMsgs().execute();
				readLogcat();
				return true;
			case R.id.email_to:
				onEmailTo();
				return true;
			case R.id.copy:
				onCopy();
				return true;
		}
		return true;
	}
	
	private void onCopy() {
		try {
			ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE); 
			clipboard.setText(getLogDataAsString(clientLogList.getVisibility() == View.VISIBLE));
			Toast.makeText(getApplicationContext(), R.string.eventlog_copy_toast, Toast.LENGTH_SHORT).show();
		} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"onCopy failed");}
	}
	
	private void onEmailTo() {
		try {
			String emailText = getLogDataAsString(clientLogList.getVisibility() == View.VISIBLE);
			
			Intent emailIntent = new Intent(android.content.Intent.ACTION_SEND);
	
		    // Put together the email intent		
			emailIntent.setType("plain/text");
			emailIntent.putExtra(android.content.Intent.EXTRA_SUBJECT, getString(R.string.eventlog_email_subject));
	
			emailIntent.putExtra(android.content.Intent.EXTRA_TEXT, emailText);
			
			// Send it off to the Activity-Chooser
			startActivity(Intent.createChooser(emailIntent, "Send mail..."));	
		} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"onEmailTo failed");}
	}
	
	// returns the content of the log as string
	// clientLog = true: client log
	// clientlog = false: gui log
	private String getLogDataAsString(Boolean clientLog) {
		StringBuffer text = new StringBuffer();
		if(clientLog) {
			text.append(getString(R.string.eventlog_client_header) + "\n\n");
		    for (int index = 0; index < clientLogList.getCount(); index++) {
				text.append(clientLogListAdapter.getDate(index));
				text.append("|");
				text.append(clientLogListAdapter.getProject(index));
				text.append("|");
				text.append(clientLogListAdapter.getMessage(index));
				text.append("\n");
			}
		} else {
			text.append(getString(R.string.eventlog_gui_header) + "\n\n");
		    for (String line: guiLogData) {
				text.append(line);
				text.append("\n");
			}
		}
		return text.toString();
	}
	
	private void readLogcat() {
		int number = getResources().getInteger(R.integer.eventlog_gui_messages);
		guiLogData.clear();
		try {
			String logLevelFilter = Logging.TAG;
			switch(Logging.LOGLEVEL){
			case 0: return;
			case 1:
				logLevelFilter += ":E";
				break;
			case 2:
				logLevelFilter += ":W";
				break;
			case 3:
				logLevelFilter += ":I";
				break;
			case 4:
				logLevelFilter += ":D";
				break;
			case 5:
				logLevelFilter += ":V";
				break;
			}
			Process process = Runtime.getRuntime().exec("logcat -d -t " + number + " -v time " + logLevelFilter + " *:S");
			// filtering logcat output by application package is not possible on command line
			// devices with SDK > 13 will automatically "session filter"
			BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));

			String line = "";
			int x = 0;
			while ((line = BOINCUtils.readLineLimit(bufferedReader, 4096)) != null) {
				if(x > 1) guiLogData.add(0,line); // cut off first two lines, prepend to array (most current on top)
				x++;
			}
			if(Logging.VERBOSE) Log.v(Logging.TAG, "readLogcat read " + guiLogData.size() + " lines.");
			guiLogListAdapter.notifyDataSetChanged();
		} catch (IOException e) {if(Logging.WARNING) Log.w(Logging.TAG, "readLogcat failed", e);}
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
                new RetrievePastClientMsgs().execute();
                loading = true;
            }
        }

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
        }
    }
	
	private final class RetrieveRecentClientMsgs extends AsyncTask<Void,Void,ArrayList<Message>> {
		
		private Integer mostRecentSeqNo = 0;

		@Override
		protected void onPreExecute() {
			if(!mIsBound) cancel(true); // cancel execution if monitor is not bound yet
			if(!clientLogData.isEmpty()) mostRecentSeqNo = clientLogData.get(0).seqno;
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
	
	private final class RetrievePastClientMsgs extends AsyncTask<Void,Void,List<Message>> {
		
		//private int mostRecentSeqNo = 0; // most recent (highest) seqNo
		private int pastSeqNo = -1; // oldest (lowest) seqNo currently loaded to GUI
		//private int lastclientLogDataListIndex = 0; // index of last element (oldest message) in clientLogData

		@Override
		protected void onPreExecute() {
			if(!mIsBound) cancel(true); // cancel execution if monitor is not bound yet
			if(!clientLogData.isEmpty()) {
				pastSeqNo = clientLogData.get(clientLogData.size() - 1).seqno;
				if(pastSeqNo==0) {
					if(Logging.DEBUG) Log.d("RetrievePastMsgs", "cancel, oldest messages already loaded");
					cancel(true); // cancel if all past messages are present
				}
			}
		}
		
		@Override
		protected List<Message> doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d("RetrievePastMsgs", "calling monitor with: " + pastSeqNo + " / " + pastMsgsLoadingRange);
			return monitor.getEventLogMessages(pastSeqNo, pastMsgsLoadingRange); 
		}

		@Override
		protected void onPostExecute(List<Message> result) {
			// back in UI thread
			loadPastMsgs(result);
		}
	}
}
