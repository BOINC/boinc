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
import java.util.Collections;
import edu.berkeley.boinc.adapter.NoticesListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Notice;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

public class NoticesActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;

	private ListView noticesList;
	private NoticesListAdapter noticesListAdapter;
	private ArrayList<Notice> data = new ArrayList<Notice>();
	
	@Override
	public void onCreate(Bundle savedInstanceState) {

		setLayoutLoading();
		doBindService();

	    super.onCreate(savedInstanceState);
	}

	@Override
	public void onResume() {
		if(Logging.VERBOSE) Log.d(Logging.TAG, "NoticesActivity onResume()");

		super.onResume();
	    
	    // data retrieval
	    (new NoticesRetrievalAsync()).execute();
	}
	
	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	if(Logging.VERBOSE) Log.v(Logging.TAG,"EventLogActivity onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    
		    // data retrieval
		    (new NoticesRetrievalAsync()).execute();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
	        mIsBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0);
		}
	}

	private void doUnbindService() {
	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	private void populateLayout() {
		try {
				
			setContentView(R.layout.notices_layout); 
			
			noticesList = (ListView) findViewById(R.id.noticesList);
			noticesListAdapter = new NoticesListAdapter(NoticesActivity.this, R.id.noticesList, data);
			noticesList.setAdapter(noticesListAdapter);
			
			
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
	
	public void noticeClick(View v) {
		String url = (String) v.getTag();
		if(Logging.DEBUG) Log.d(Logging.TAG,"noticeClick: " + url);
		
		if(url != null && !url.isEmpty()){ 
    		Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
    		startActivity(i);
		}
	}
	
	private final class NoticesRetrievalAsync extends AsyncTask<Void,Void,Boolean> {

		@Override
		protected void onPreExecute() {
			if(Logging.DEBUG) Log.d(Logging.TAG,"NoticesRetrievalAsync onPreExecute, mIsBound: " + mIsBound);
			if(!mIsBound) cancel(false);
			super.onPreExecute();
		}

		@Override
		protected Boolean doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"NoticesRetrievalAsync doInBackground");
			try{
				ArrayList<Notice> monitorList = monitor.getNotices();
				// remove client and scheduler notices
				ArrayList<Notice> rssNotices = new ArrayList<Notice>();
				for(Notice notice: monitorList) {
					if(!notice.isClientNotice && !notice.isServerNotice) rssNotices.add(notice);
				}
				
				// reverse to have most current on top
				Collections.reverse(rssNotices);
				data = rssNotices;
			} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"NoticesRetrievalAsync error in do in background",e);}
			if(data != null) return true;
			else return false;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"NoticesRetrievalAsync success: " + success + " data elements: " + data.size());
			if(success) populateLayout();
			else if(Logging.WARNING) Log.w(Logging.TAG,"NoticesRetrievalAsync failed.");
		}
	}
}
