/*
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
 */
package edu.berkeley.boinc;

import edu.berkeley.boinc.adapter.ClientLogListAdapter;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.utils.*;

import java.util.ArrayList;
import java.util.List;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;

public class EventLogClientFragment extends Fragment {

    // message retrieval
    private Integer pastMsgsLoadingRange = 50; // amount messages loaded when end of list is reached
    private EventLogActivity a;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        a = ((EventLogActivity) getActivity());

        View layout = inflater.inflate(R.layout.eventlog_client_layout, container, false);

        a.clientLogList = layout.findViewById(R.id.clientLogList);
        a.clientLogListAdapter =
                new ClientLogListAdapter(getActivity(), a.clientLogList, R.id.clientLogList, a.clientLogData);
        a.clientLogList.setOnScrollListener(new EndlessScrollListener(5));

        return layout;
    }

    public void init() {
        new RetrievePastClientMsgs().execute(); // read messages
    }

    public void update() {
        new RetrieveRecentClientMsgs().execute(); // refresh messages
    }

    // appends older messages to data list
    private void loadPastMsgs(List<edu.berkeley.boinc.rpc.Message> tmpA) {
        // Append old messages to the event log
        try {
            for(int x = tmpA.size() - 1; x >= 0; x--) {
                a.clientLogData.add(tmpA.get(x));
            }
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "EventLogClientFragment.loadPastMsgs error: ", e);
            }
        } //IndexOutOfBoundException

        a.clientLogListAdapter.notifyDataSetChanged();
    }

    // updates data list with most recent messages
    private void loadRecentMsgs(ArrayList<edu.berkeley.boinc.rpc.Message> tmpA) {
        // Prepend new messages to the event log
        try {
            int y = 0;
            for(int x = tmpA.size() - 1; x >= 0; x--) {
                a.clientLogData.add(y, tmpA.get(x));
                y++;
            }
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "EventLogClientFragment.loadRecentMsgs error: ", e);
            }
        } //IndexOutOfBoundException
        a.clientLogListAdapter.notifyDataSetChanged();
    }

    // onScrollListener for list view, implementing "endless scrolling"
    public final class EndlessScrollListener implements OnScrollListener {

        private int visibleThreshold;
        private int previousTotal = 0;
        private boolean loading = true;

        public EndlessScrollListener(int visibleThreshold) {
            this.visibleThreshold = visibleThreshold;
        }

        @Override
        public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount) {
            if(loading) {
                if(totalItemCount > previousTotal) {
                    loading = false;
                    previousTotal = totalItemCount;
                }
            }
            if(!loading && (totalItemCount - visibleItemCount) <= (firstVisibleItem + visibleThreshold)) {
                new RetrievePastClientMsgs().execute();
                loading = true;
            }
        }

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
        }
    }

    private final class RetrieveRecentClientMsgs extends AsyncTask<Void, Void, ArrayList<edu.berkeley.boinc.rpc.Message>> {

        private Integer mostRecentSeqNo = 0;

        @Override
        protected void onPreExecute() {
            if(!a.clientLogData.isEmpty()) {
                mostRecentSeqNo = a.clientLogData.get(0).seqno;
            }
        }

        @Override
        protected ArrayList<edu.berkeley.boinc.rpc.Message> doInBackground(Void... params) {
            try {
                return (ArrayList<edu.berkeley.boinc.rpc.Message>) ((EventLogActivity) getActivity()).getMonitorService().getMessages(mostRecentSeqNo);
            }
            catch(RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return new ArrayList<>();
            }
        }

        @Override
        protected void onPostExecute(ArrayList<edu.berkeley.boinc.rpc.Message> result) {
            // back in UI thread
            loadRecentMsgs(result);
        }
    }

    private final class RetrievePastClientMsgs extends AsyncTask<Void, Void, List<edu.berkeley.boinc.rpc.Message>> {

        //private int mostRecentSeqNo = 0; // most recent (highest) seqNo
        private int pastSeqNo = -1; // oldest (lowest) seqNo currently loaded to GUI
        //private int lastclientLogDataListIndex = 0; // index of last element (oldest message) in clientLogData

        @Override
        protected void onPreExecute() {
            if(!a.clientLogData.isEmpty()) {
                pastSeqNo = a.clientLogData.get(a.clientLogData.size() - 1).seqno;
                if(pastSeqNo == 0) {
                    if(Logging.DEBUG) {
                        Log.d("RetrievePastMsgs", "cancel, oldest messages already loaded");
                    }
                    cancel(true); // cancel if all past messages are present
                }
            }
        }

        @Override
        protected List<Message> doInBackground(Void... params) {
            if(Logging.DEBUG) {
                Log.d("RetrievePastMsgs", "calling monitor with: " + pastSeqNo + " / " + pastMsgsLoadingRange);
            }
            try {
                return ((EventLogActivity) getActivity()).getMonitorService().getEventLogMessages(pastSeqNo, pastMsgsLoadingRange);
            }
            catch(RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return new ArrayList<>();
            }
        }

        @Override
        protected void onPostExecute(List<edu.berkeley.boinc.rpc.Message> result) {
            // back in UI thread
            loadPastMsgs(result);
        }
    }
}
