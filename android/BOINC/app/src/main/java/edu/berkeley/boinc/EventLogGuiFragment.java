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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

public class EventLogGuiFragment extends Fragment {

    private EventLogActivity a;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        a = ((EventLogActivity) getActivity());

        View layout = inflater.inflate(R.layout.eventlog_gui_layout, container, false);

        a.guiLogList = layout.findViewById(R.id.guiLogList);
        a.guiLogListAdapter = new ArrayAdapter<>(getActivity(), R.layout.eventlog_gui_listitem_layout, a.guiLogData);
        a.guiLogList.setAdapter(a.guiLogListAdapter);

        // read messages
        readLogcat();

        return layout;
    }

    public void init() {
        readLogcat();
    }

    public void update() {
        readLogcat();
    }

    private void readLogcat() {
        int number = getResources().getInteger(R.integer.eventlog_gui_messages);
        a.guiLogData.clear();
        try {
            String logLevelFilter = Logging.TAG;
            switch(Logging.LOGLEVEL) {
                case 0:
                    return;
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
            Process process =
                    Runtime.getRuntime().exec("logcat -d -t " + number + " -v time " + logLevelFilter + " *:S");
            // filtering logcat output by application package is not possible on command line
            // devices with SDK > 13 will automatically "session filter"
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));

            String line;
            int x = 0;
            while((line = BOINCUtils.readLineLimit(bufferedReader, 4096)) != null) {
                if(x > 1) {
                    a.guiLogData.add(0, line); // cut off first two lines, prepend to array (most current on top)
                }
                x++;
            }
            if(Logging.VERBOSE) {
                Log.v(Logging.TAG, "readLogcat read " + a.guiLogData.size() + " lines.");
            }
            a.guiLogListAdapter.notifyDataSetChanged();
        }
        catch(IOException e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "readLogcat failed", e);
            }
        }
    }
}
