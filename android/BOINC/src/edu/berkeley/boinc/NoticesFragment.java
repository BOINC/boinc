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
import edu.berkeley.boinc.rpc.Notice;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.TextView;

public class NoticesFragment extends Fragment {

	private ListView noticesList;
	private NoticesListAdapter noticesListAdapter;
	private ArrayList<Notice> data = new ArrayList<Notice>();
	
	private ViewGroup container;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    	if(Logging.VERBOSE) Log.v(Logging.TAG,"NoticesFragment onCreateView");
    	this.container = container;
        // Inflate the loading layout for this fragment
    	View loading = inflater.inflate(R.layout.generic_layout_loading, container, false);
        TextView loadingHeader = (TextView)loading.findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.eventlog_loading);
		return loading;
	}

	@Override
	public void onResume() {
		if(Logging.VERBOSE) Log.d(Logging.TAG, "NoticesFragment onResume()");

		super.onResume();
	    
	    // data retrieval
	    (new NoticesRetrievalAsync()).execute();
	}
	
	private void populateLayout() {
		container.removeAllViews();
		View layout = getActivity().getLayoutInflater().inflate(R.layout.notices_layout, container);
		noticesList = (ListView) layout.findViewById(R.id.noticesList);
		noticesListAdapter = new NoticesListAdapter(getActivity(), R.id.noticesList, data);
		noticesList.setAdapter(noticesListAdapter);
	}
	
	/**
	 * retrieves all notices from the client
	 * starting with seq number 0
	 */
	private final class NoticesRetrievalAsync extends AsyncTask<Void,Void,Boolean> {

		@Override
		protected Boolean doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"NoticesRetrievalAsync doInBackground");
			try{
				ArrayList<Notice> monitorList = ((BOINCActivity)getActivity()).getMonitorService().clientInterface.getNotices(0);
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
