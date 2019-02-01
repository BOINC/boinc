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

import edu.berkeley.boinc.utils.*;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import edu.berkeley.boinc.adapter.NoticesListAdapter;
import edu.berkeley.boinc.rpc.Notice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

public class NoticesFragment extends Fragment {

    private ListView noticesList;
    private NoticesListAdapter noticesListAdapter;
    private ArrayList<Notice> data = new ArrayList<>();

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "NoticesFragment onCreateView");
        }
        View layout = inflater.inflate(R.layout.notices_layout, container, false);
        noticesList = layout.findViewById(R.id.noticesList);
        updateNotices();
        noticesListAdapter = new NoticesListAdapter(getActivity(), R.id.noticesList, data);
        noticesList.setAdapter(noticesListAdapter);
        return layout;
    }

    @Override
    public void onResume() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "NoticesFragment onResume()");
        }
        getActivity().registerReceiver(mClientStatusChangeRec, ifcsc);

        // clear notice notification
        try {
            BOINCActivity.monitor.cancelNoticeNotification();
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "NoticesFragment.onResume error: ", e);
            }
        }
        super.onResume();
    }

    @Override
    public void onPause() {
        //unregister receiver, so there are not multiple intents flying in
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "NoticesFragment remove receiver");
        }
        getActivity().unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "NoticesFragment ClientStatusChange - onReceive()");
            }

            // data retrieval
            updateNotices();
            noticesListAdapter.clear();
            for(Notice tmp : data) { // addAll only in API 11
                noticesListAdapter.add(tmp);
            }
            noticesListAdapter.notifyDataSetChanged();
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

    private void updateNotices() {
        try {
            data = (ArrayList<Notice>) BOINCActivity.monitor.getRssNotices();
            // sorting policy:
            // latest arrival first.
            Collections.sort(data, new Comparator<Notice>() {
                @Override
                public int compare(Notice lhs, Notice rhs) {
                    return ((Double) (rhs.create_time - lhs.create_time)).intValue();
                }
            });
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "NoticesFragment.updateNotices error: ", e);
            }
        }
    }
}
