/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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
package edu.berkeley.boinc.ui.eventlog;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.adapter.GuiLogRecyclerViewAdapter;
import edu.berkeley.boinc.databinding.EventLogGuiLayoutBinding;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class EventLogGuiFragment extends Fragment {
    private EventLogActivity a;
    private EventLogGuiLayoutBinding binding;
    private GuiLogRecyclerViewAdapter adapter;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        a = ((EventLogActivity) getActivity());

        binding = EventLogGuiLayoutBinding.inflate(inflater, container, false);

        adapter = new GuiLogRecyclerViewAdapter(a.getGuiLogData());
        binding.guiLogList.setLayoutManager(new LinearLayoutManager(getContext()));
        binding.guiLogList.setAdapter(adapter);
        binding.getRoot().setOnRefreshListener(this::readLogcat);

        // read messages
        readLogcat();

        return binding.getRoot();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

    public void update() {
        readLogcat();
    }

    private void readLogcat() {
        int number = getResources().getInteger(R.integer.eventlog_gui_messages);
        a.getGuiLogData().clear();
        try {
            String logLevelFilter = Logging.TAG;
            switch(Logging.getLogLevel()) {
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
                    a.getGuiLogData().add(0, line); // cut off first two lines, prepend to array (most current on top)
                }
                x++;
            }

            Logging.logVerbose(Logging.Category.DEVICE, "readLogcat read " + a.getGuiLogData().size() + " lines.");

            adapter.notifyDataSetChanged();
            binding.getRoot().setRefreshing(false);
        }
        catch(IOException e) {
            Logging.logException(Logging.Category.DEVICE, "readLogcat failed", e);
        }
    }
}
