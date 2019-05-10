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
import edu.berkeley.boinc.attach.SelectionListActivity;
import edu.berkeley.boinc.client.ClientStatus;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class StatusFragment extends Fragment {

    // keep computingStatus and suspend reason to only adapt layout when changes occur
    private Integer computingStatus = -1;
    private Integer computingSuspendReason = -1;
    private Integer networkSuspendReason = -1;
    private Integer setupStatus = -1;

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "StatusFragment ClientStatusChange - onReceive()");
            }
            loadLayout(false);
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

    public void onResume() {
        //register noisy clientStatusChangeReceiver here, so only active when Activity is visible
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment register receiver");
        }
        getActivity().registerReceiver(mClientStatusChangeRec, ifcsc);

        //loadLayout(true);
        super.onResume();
    }

    public void onPause() {
        //unregister receiver, so there are not multiple intents flying in
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment remove receiver");
        }
        getActivity().unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment onCreateView");
        }
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.status_layout, container, false);
    }

    private void loadLayout(Boolean forceUpdate) {
        //load layout, if if ClientStatus can be accessed.
        //if this is not the case, the broadcast receiver will call "loadLayout" again
        try {

            int currentSetupStatus = BOINCActivity.monitor.getSetupStatus();
            int currentComputingStatus = BOINCActivity.monitor.getComputingStatus();
            int currentComputingSuspendReason = BOINCActivity.monitor.getComputingSuspendReason();
            int currentNetworkSuspendReason = BOINCActivity.monitor.getNetworkSuspendReason();

            // layout only if client RPC connection is established
            // otherwise BOINCActivity does not start Tabs
            if(currentSetupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) {
                // return in cases nothing has changed
                if(forceUpdate ||
                   computingStatus != currentComputingStatus ||
                   currentComputingSuspendReason != computingSuspendReason ||
                   currentNetworkSuspendReason != networkSuspendReason) {

                    // set layout and retrieve elements
                    LinearLayout statusWrapper = getView().findViewById(R.id.status_wrapper);
                    LinearLayout centerWrapper = getView().findViewById(R.id.center_wrapper);
                    LinearLayout restartingWrapper = getView().findViewById(R.id.restarting_wrapper);
                    TextView statusHeader = getView().findViewById(R.id.status_header);
                    ImageView statusImage = getView().findViewById(R.id.status_image);
                    TextView statusDescriptor = getView().findViewById(R.id.status_long);

                    restartingWrapper.setVisibility(View.GONE);

                    // adapt to specific computing status
                    switch(currentComputingStatus) {
                        case ClientStatus.COMPUTING_STATUS_NEVER:
                            statusWrapper.setVisibility(View.VISIBLE);
                            statusHeader.setText(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusHeader.setVisibility(View.VISIBLE);
                            statusImage.setImageResource(R.drawable.playb48);
                            statusImage.setContentDescription(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                            centerWrapper.setVisibility(View.VISIBLE);
                            centerWrapper.setOnClickListener(runModeOnClickListener);
                            break;
                        case ClientStatus.COMPUTING_STATUS_SUSPENDED:
                            statusWrapper.setVisibility(View.VISIBLE);
                            statusHeader.setText(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusHeader.setVisibility(View.VISIBLE);
                            statusImage.setImageResource(R.drawable.pauseb48);
                            statusImage.setContentDescription(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusImage.setClickable(false);
                            centerWrapper.setVisibility(View.VISIBLE);
                            switch(currentComputingSuspendReason) {
                                case BOINCDefs.SUSPEND_REASON_BATTERIES:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    statusImage.setImageResource(R.drawable.notconnectedb48);
                                    statusHeader.setVisibility(View.GONE);
                                    break;
                                case BOINCDefs.SUSPEND_REASON_USER_ACTIVE:
                                    Boolean suspendDueToScreenOn = false;
                                    try {
                                        suspendDueToScreenOn = BOINCActivity.monitor.getSuspendWhenScreenOn();
                                    }
                                    catch(RemoteException e) {
                                        if(Logging.ERROR) {
                                            Log.e(Logging.TAG, "StatusFragment.loadLayout error: ", e);
                                        }
                                    }
                                    if(suspendDueToScreenOn) {
                                        statusImage.setImageResource(R.drawable.screen48b);
                                        statusHeader.setVisibility(View.GONE);
                                    }
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_USER_REQ:
                                    // state after user stops and restarts computation
                                    centerWrapper.setVisibility(View.GONE);
                                    restartingWrapper.setVisibility(View.VISIBLE);
                                    break;
                                case BOINCDefs.SUSPEND_REASON_TIME_OF_DAY:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_BENCHMARKS:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    statusImage.setImageResource(R.drawable.watchb48);
                                    statusHeader.setVisibility(View.GONE);
                                    break;
                                case BOINCDefs.SUSPEND_REASON_DISK_SIZE:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_CPU_THROTTLE:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_NO_RECENT_INPUT:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_INITIAL_DELAY:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_CPU_USAGE:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_OS:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_WIFI_STATE:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                                case BOINCDefs.SUSPEND_REASON_BATTERY_CHARGING:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    statusImage.setImageResource(R.drawable.batteryb48);
                                    statusHeader.setVisibility(View.GONE);
                                    break;
                                case BOINCDefs.SUSPEND_REASON_BATTERY_OVERHEATED:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    statusImage.setImageResource(R.drawable.batteryb48);
                                    statusHeader.setVisibility(View.GONE);
                                    break;
                                default:
                                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                                    break;
                            }
                            break;
                        case ClientStatus.COMPUTING_STATUS_IDLE:
                            statusWrapper.setVisibility(View.VISIBLE);
                            centerWrapper.setVisibility(View.VISIBLE);
                            statusHeader.setText(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusHeader.setVisibility(View.VISIBLE);
                            statusImage.setImageResource(R.drawable.pauseb48);
                            statusImage.setContentDescription(BOINCActivity.monitor.getCurrentStatusTitle());
                            statusImage.setClickable(false);
                            statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusDescription());
                            break;
                        case ClientStatus.COMPUTING_STATUS_COMPUTING:
                            statusWrapper.setVisibility(View.GONE);
                            break;
                    }
                    //save new computing status
                    computingStatus = currentComputingStatus;
                    computingSuspendReason = currentComputingSuspendReason;
                    networkSuspendReason = currentNetworkSuspendReason;
                    setupStatus = -1; // invalidate to force update next time no project
                }
            }
            else if(currentSetupStatus == ClientStatus.SETUP_STATUS_NOPROJECT) {

                if(setupStatus != ClientStatus.SETUP_STATUS_NOPROJECT) {
                    // set layout and retrieve elements
                    LinearLayout statusWrapper = getView().findViewById(R.id.status_wrapper);
                    LinearLayout centerWrapper = getView().findViewById(R.id.center_wrapper);
                    LinearLayout restartingWrapper = getView().findViewById(R.id.restarting_wrapper);
                    TextView statusHeader = getView().findViewById(R.id.status_header);
                    ImageView statusImage = getView().findViewById(R.id.status_image);
                    TextView statusDescriptor = getView().findViewById(R.id.status_long);

                    statusWrapper.setVisibility(View.VISIBLE);
                    restartingWrapper.setVisibility(View.GONE);
                    centerWrapper.setVisibility(View.VISIBLE);
                    centerWrapper.setOnClickListener(addProjectOnClickListener);
                    statusImage.setImageResource(R.drawable.projectsb48);
                    statusHeader.setVisibility(View.GONE);
                    statusDescriptor.setText(BOINCActivity.monitor.getCurrentStatusTitle());
                    setupStatus = ClientStatus.SETUP_STATUS_NOPROJECT;
                    computingStatus = -1;
                }
            }
            else { // BOINC client is not available
                //invalid computingStatus, forces layout on next event
                setupStatus = -1;
                computingStatus = -1;
            }
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "StatusFragment.loadLayout error: ", e);
            }
        }
    }

    private OnClickListener runModeOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            new WriteClientRunModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
        }
    };

    private OnClickListener addProjectOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            startActivity(new Intent(getActivity(), SelectionListActivity.class));
        }
    };

    private final class WriteClientRunModeAsync extends AsyncTask<Integer, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Integer... params) {
            // setting provided mode for both, CPU computation and network.
            Boolean runMode;
            try {
                runMode = BOINCActivity.monitor.setRunMode(params[0]);
            }
            catch(RemoteException e) {
                runMode = false;
            }
            Boolean networkMode;
            try {
                networkMode = BOINCActivity.monitor.setNetworkMode(params[0]);
            }
            catch(RemoteException e) {
                networkMode = false;
            }
            return runMode && networkMode;
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(success) {
                try {
                    BOINCActivity.monitor.forceRefresh();
                }
                catch(RemoteException e) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "StatusFragment.WriteClientRunModeAsync.onPostExecute() error: ", e);
                    }
                }
            }
            else if(Logging.WARNING) {
                Log.w(Logging.TAG, "StatusFragment: setting run mode failed");
            }
        }
    }
}
