/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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

import android.app.Dialog;
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
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import edu.berkeley.boinc.adapter.PrefsListAdapter;
import edu.berkeley.boinc.adapter.PrefsListItemWrapper;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperBool;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperNumber;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperText;
import edu.berkeley.boinc.adapter.PrefsSelectionDialogListAdapter;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import edu.berkeley.boinc.utils.Logging;

import java.text.NumberFormat;
import java.util.ArrayList;

public class PrefsFragment extends Fragment {

    private ListView lv;
    private PrefsListAdapter listAdapter;

    // Data for the PrefsListAdapter. This is should be HashMap!
    private ArrayList<PrefsListItemWrapper> data = new ArrayList<>();
    // Android specific preferences of the client, read on every onResume via RPC
    private GlobalPreferences clientPrefs = null;
    private HostInfo hostinfo = null;

    private boolean layoutSuccessful = false;

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "PrefsFragment ClientStatusChange - onReceive()");
            }
            try {
                if(!layoutSuccessful) {
                    populateLayout();
                }
            }
            catch(RemoteException e) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "PrefsFragment.BroadcastReceiver: onReceive() error: ", e);
                }
            }
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

    // fragment lifecycle: 2.
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "ProjectsFragment onCreateView");
        }

        // Inflate the layout for this fragment
        View layout = inflater.inflate(R.layout.prefs_layout, container, false);
        lv = layout.findViewById(R.id.listview);
        listAdapter = new PrefsListAdapter(getActivity(), this, R.id.listview, data);
        lv.setAdapter(listAdapter);
        return layout;
    }

    // fragment lifecycle: 1.
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    // fragment lifecycle: 3.
    @Override
    public void onResume() {
        try {
            populateLayout();
        }
        catch(RemoteException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "PrefsFragment.onResume error: ", e);
            }
        }
        getActivity().registerReceiver(mClientStatusChangeRec, ifcsc);
        super.onResume();
    }

    @Override
    public void onPause() {
        getActivity().unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    private Boolean getPrefs() {
        // Try to get current client status from monitor
        try {
            clientPrefs = BOINCActivity.monitor.getPrefs(); // Read preferences from client via rpc
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "PrefsActivity: Could not load data, clientStatus not initialized.");
            }
            e.printStackTrace();
            return false;
        }

        if(clientPrefs == null) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "readPrefs: null, return false");
            }
            return false;
        }

        return true;
    }

    private Boolean getHostInfo() {
        // Try to get current client status from monitor
        try {
            hostinfo = BOINCActivity.monitor.getHostInfo(); // Get the hostinfo from client via rpc
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "PrefsActivity: Could not load data, clientStatus not initialized.");
            }
            e.printStackTrace();
            return false;
        }

        if(hostinfo == null) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "getHostInfo: null, return false");
            }
            return false;
        }
        return true;
    }

    private void populateLayout() throws RemoteException {

        if(!getPrefs() || BOINCActivity.monitor == null || !getHostInfo()) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "PrefsFragment.populateLayout returns, data is not present");
            }
            return;
        }

        data.clear();

        Boolean advanced = BOINCActivity.monitor.getShowAdvanced();
        Boolean stationaryDeviceMode = BOINCActivity.monitor.getStationaryDeviceMode();
        Boolean stationaryDeviceSuspected = BOINCActivity.monitor.isStationaryDeviceSuspected();

        // The order is important, the GUI will be displayed in the same order as the data is added.
        // General
        data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_general, true));
        data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_autostart_header, R.string.prefs_category_general, BOINCActivity.monitor.getAutostart()));
        data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_show_notification_notices_header, R.string.prefs_category_general, BOINCActivity.monitor.getShowNotificationForNotices()));
        data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_show_advanced_header, R.string.prefs_category_general, BOINCActivity.monitor.getShowAdvanced()));
        if(!stationaryDeviceMode) {
            data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_suspend_when_screen_on, R.string.prefs_category_general, BOINCActivity.monitor.getSuspendWhenScreenOn()));
        }
        data.add(new PrefsListItemWrapperText(getActivity(), R.string.prefs_general_device_name_header, R.string.prefs_category_general, BOINCActivity.monitor.getHostInfo().domain_name));

        // Network
        data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_network, true));
        data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_network_wifi_only_header, R.string.prefs_category_network, clientPrefs.network_wifi_only));
        if(advanced) {
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_network_daily_xfer_limit_mb_header, R.string.prefs_category_network, clientPrefs.daily_xfer_limit_mb, PrefsListItemWrapper.DialogButtonType.NUMBER));
        }

        // Power
        data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_power, true));
        if(stationaryDeviceSuspected) { // API indicates that there is no battery, offer opt-in preference for stationary device mode
            data.add(new PrefsListItemWrapperBool(getActivity(), R.string.prefs_stationary_device_mode_header, R.string.prefs_category_power, BOINCActivity.monitor.getStationaryDeviceMode()));
        }
        if(!stationaryDeviceMode) { // Client would compute regardless of battery preferences, so only show if that is not the case
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_power_source_header, R.string.prefs_category_power));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.battery_charge_min_pct_header, R.string.prefs_category_power, clientPrefs.battery_charge_min_pct, PrefsListItemWrapper.DialogButtonType.SLIDER));
            if(advanced) {
                data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.battery_temperature_max_header, R.string.prefs_category_power, clientPrefs.battery_max_temperature, PrefsListItemWrapper.DialogButtonType.NUMBER));
            }
        }

        // CPU
        if(advanced) {
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_cpu, true));
            if(hostinfo.p_ncpus > 1) {
                data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_cpu_number_cpus_header, R.string.prefs_category_cpu, pctCpuCoresToNumber(clientPrefs.max_ncpus_pct), PrefsListItemWrapper.DialogButtonType.SLIDER));
            }
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_cpu_time_max_header, R.string.prefs_category_cpu, clientPrefs.cpu_usage_limit, PrefsListItemWrapper.DialogButtonType.SLIDER));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_cpu_other_load_suspension_header, R.string.prefs_category_cpu, clientPrefs.suspend_cpu_usage, PrefsListItemWrapper.DialogButtonType.SLIDER));
        }

        // Storage
        if(advanced) {
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_storage, true));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_disk_max_pct_header, R.string.prefs_category_storage, clientPrefs.disk_max_used_pct, PrefsListItemWrapper.DialogButtonType.SLIDER));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_disk_min_free_gb_header, R.string.prefs_category_storage, clientPrefs.disk_min_free_gb, PrefsListItemWrapper.DialogButtonType.NUMBER));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_disk_access_interval_header, R.string.prefs_category_storage, clientPrefs.disk_interval, PrefsListItemWrapper.DialogButtonType.NUMBER));
        }

        // Memory
        if(advanced) {
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_memory, true));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_memory_max_idle_header, R.string.prefs_category_memory, clientPrefs.ram_max_used_idle_frac, PrefsListItemWrapper.DialogButtonType.SLIDER));
        }

        // Other
        if(advanced) {
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_other, true));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_other_store_at_least_x_days_of_work_header, R.string.prefs_category_other, clientPrefs.work_buf_min_days, PrefsListItemWrapper.DialogButtonType.NUMBER));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header, R.string.prefs_category_other, clientPrefs.work_buf_additional_days, PrefsListItemWrapper.DialogButtonType.NUMBER));
        }

        // Debug
        if(advanced) {
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_category_debug, true));
            data.add(new PrefsListItemWrapper(getActivity(), R.string.prefs_client_log_flags_header, R.string.prefs_category_debug));
            data.add(new PrefsListItemWrapperNumber(getActivity(), R.string.prefs_gui_log_level_header, R.string.prefs_category_debug, (double) BOINCActivity.monitor.getLogLevel(), PrefsListItemWrapper.DialogButtonType.SLIDER));
        }

        updateLayout();
        layoutSuccessful = true;
    }

    private void updateLayout() {
        listAdapter.notifyDataSetChanged();
    }

    // Updates list item of boolean preference
    // Requires updateLayout to be called afterwards
    private void updateBoolPreference(int ID, Boolean newValue) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "updateBoolPreference for ID: " + ID + " value: " + newValue);
        }
        for(PrefsListItemWrapper item : data) {
            if(item.ID == ID) {
                ((PrefsListItemWrapperBool) item).setStatus(newValue);
                break; // The preferences updated one by one.
            }
        }
    }

    // Updates list item of number preference
    // Requires updateLayout to be called afterwards
    private void updateNumberPreference(int ID, Double newValue) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "updateNumberPreference for ID: " + ID + " value: " + newValue);
        }
        for(PrefsListItemWrapper item : data) {
            if(item.ID == ID) {
                ((PrefsListItemWrapperNumber) item).status = newValue;
                break; // The preferences updated one by one.
            }
        }
    }

    // Updates list item of text preference
    private void updateTextPreference(int ID, String newValue) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "updateTextPreference for ID: " + ID + " value: " + newValue);
        }
        for(PrefsListItemWrapper item : data) {
            if(item.ID == ID) {
                ((PrefsListItemWrapperText) item).status = newValue;
                break; // The preferences updated one by one.
            }
        }
    }

    private void setupSliderDialog(PrefsListItemWrapper item, final Dialog dialog) {
        final PrefsListItemWrapperNumber prefsListItemWrapperNumber = (PrefsListItemWrapperNumber) item;
        dialog.setContentView(R.layout.prefs_layout_dialog_pct);
        TextView sliderProgress = dialog.findViewById(R.id.seekbar_status);
        SeekBar slider = dialog.findViewById(R.id.seekbar);

        if(prefsListItemWrapperNumber.ID == R.string.battery_charge_min_pct_header ||
           prefsListItemWrapperNumber.ID == R.string.prefs_disk_max_pct_header ||
           prefsListItemWrapperNumber.ID == R.string.prefs_cpu_time_max_header ||
           prefsListItemWrapperNumber.ID == R.string.prefs_cpu_other_load_suspension_header ||
           prefsListItemWrapperNumber.ID == R.string.prefs_memory_max_idle_header) {
            Double seekBarDefault = prefsListItemWrapperNumber.status / 10;
            slider.setProgress(seekBarDefault.intValue());
            final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
                    final String progressString = NumberFormat.getPercentInstance().format(progress / 10.0);
                    TextView sliderProgress = dialog.findViewById(R.id.seekbar_status);
                    sliderProgress.setText(progressString);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                }
            });
            onSeekBarChangeListener.onProgressChanged(slider, seekBarDefault.intValue(), false);
        }
        else if(prefsListItemWrapperNumber.ID == R.string.prefs_cpu_number_cpus_header) {
            if(!getHostInfo()) {
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "onItemClick missing hostInfo");
                }
                return;
            }
            slider.setMax(hostinfo.p_ncpus <= 1 ? 0 : hostinfo.p_ncpus - 1);
            final int statusValue;
            slider.setProgress((statusValue = prefsListItemWrapperNumber.status.intValue()) <= 0 ?
                               0 :
                               statusValue - 1 > slider.getMax() ?
                               slider.getMax() :
                               statusValue - 1);
            Log.d(Logging.TAG, String.format("statusValue == %,d", statusValue));
            final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
                    final String progressString = NumberFormat.getIntegerInstance().format(
                            progress <= 0 ? 1 : progress + 1); // do not allow 0 cpus
                    TextView sliderProgress = dialog.findViewById(R.id.seekbar_status);
                    sliderProgress.setText(progressString);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                }
            });
            onSeekBarChangeListener.onProgressChanged(slider, statusValue - 1, false);
        }
        else if(prefsListItemWrapperNumber.ID == R.string.prefs_gui_log_level_header) {
            slider.setMax(5);
            slider.setProgress(prefsListItemWrapperNumber.status.intValue());
            final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
                public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
                    String progressString = NumberFormat.getIntegerInstance().format(progress);
                    TextView sliderProgress = dialog.findViewById(R.id.seekbar_status);
                    sliderProgress.setText(progressString);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                }
            });
            onSeekBarChangeListener.onProgressChanged(slider, prefsListItemWrapperNumber.status.intValue(), false);
        }

        setupDialogButtons(item, dialog);
    }

    private void setupSelectionListDialog(final PrefsListItemWrapper item, final Dialog dialog) throws RemoteException {
        dialog.setContentView(R.layout.prefs_layout_dialog_selection);

        if(item.ID == R.string.prefs_client_log_flags_header) {
            final ArrayList<SelectionDialogOption> options = new ArrayList<>();
            String[] array = getResources().getStringArray(R.array.prefs_client_log_flags);
            for(String option : array) {
                options.add(new SelectionDialogOption(option));
            }
            ListView lv = dialog.findViewById(R.id.selection);
            new PrefsSelectionDialogListAdapter(getActivity(), lv, R.id.selection, options);

            // Setup confirm button action
            Button confirm = dialog.findViewById(R.id.confirm);
            confirm.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    ArrayList<String> selectedOptions = new ArrayList<>();
                    for(SelectionDialogOption option : options) {
                        if(option.selected) {
                            selectedOptions.add(option.name);
                        }
                    }
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG, selectedOptions.size() + " log flags selected");
                    }
                    new SetCcConfigAsync().execute(formatOptionsToCcConfig(selectedOptions));
                    dialog.dismiss();
                }
            });
        }
        else if(item.ID == R.string.prefs_power_source_header) {
            final ArrayList<SelectionDialogOption> options = new ArrayList<>();
            options.add(new SelectionDialogOption(R.string.prefs_power_source_ac, BOINCActivity.monitor.getPowerSourceAc()));
            options.add(new SelectionDialogOption(R.string.prefs_power_source_usb, BOINCActivity.monitor.getPowerSourceUsb()));
            options.add(new SelectionDialogOption(R.string.prefs_power_source_wireless, BOINCActivity.monitor.getPowerSourceWireless()));
            options.add(new SelectionDialogOption(R.string.prefs_power_source_battery, clientPrefs.run_on_batteries, true));
            ListView lv = dialog.findViewById(R.id.selection);
            new PrefsSelectionDialogListAdapter(getActivity(), lv, R.id.selection, options);

            // Setup confirm button action
            Button confirm = dialog.findViewById(R.id.confirm);
            confirm.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    try {
                        for(SelectionDialogOption option : options) {
                            switch(option.ID) {
                                case R.string.prefs_power_source_ac:
                                    BOINCActivity.monitor.setPowerSourceAc(option.selected);
                                    break;
                                case R.string.prefs_power_source_usb:
                                    BOINCActivity.monitor.setPowerSourceUsb(option.selected);
                                    break;
                                case R.string.prefs_power_source_wireless:
                                    BOINCActivity.monitor.setPowerSourceWireless(option.selected);
                                    break;
                                case R.string.prefs_power_source_battery:
                                    clientPrefs.run_on_batteries = option.selected;
                                    new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
                                    break;
                            }
                        }
                        dialog.dismiss();
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "PrefsFragment.setupSelectionListDialog.setOnClickListener: OnClick() error: ", e);
                        }
                    }
                }
            });
        }

        // Generic cancel button
        Button cancel = dialog.findViewById(R.id.cancel);
        cancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
            }
        });
    }

    private void setupDialogButtons(final PrefsListItemWrapper item, final Dialog dialog) {
        // Confirm
        Button confirm = dialog.findViewById(R.id.confirm);
        confirm.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // Sliders
                if(item.dialogButtonType == PrefsListItemWrapper.DialogButtonType.SLIDER) {
                    SeekBar slider = dialog.findViewById(R.id.seekbar);
                    int sliderProgress = slider.getProgress();
                    double value;

                    // Calculate value based on Slider Progress
                    if(item.ID == R.string.prefs_cpu_number_cpus_header) {
                        value = numberCpuCoresToPct(sliderProgress <= 0 ? 1 : sliderProgress + 1);
                        writeClientNumberPreference(item.ID, value);
                    }
                    else if(item.ID == R.string.prefs_gui_log_level_header) {
                        try {
                            // Monitor and UI in two different processes. set static variable in both
                            Logging.setLogLevel(sliderProgress);
                            BOINCActivity.monitor.setLogLevel(sliderProgress);
                        }
                        catch(RemoteException e) {
                            if(Logging.ERROR) {
                                Log.e(Logging.TAG, "PrefsFragment.setupSelectionListDialog.setOnClickListener: OnClick() error: ", e);
                            }
                        }
                        updateNumberPreference(item.ID, (double) sliderProgress);
                        updateLayout();
                    }
                    else {
                        value = sliderProgress * 10;
                        writeClientNumberPreference(item.ID, value);
                    }
                }
                // Numbers
                else if(item.dialogButtonType == PrefsListItemWrapper.DialogButtonType.NUMBER) {
                    EditText edit = dialog.findViewById(R.id.Input);
                    String input = edit.getText().toString();
                    Double valueTmp = parseInputValueToDouble(input);
                    if(valueTmp == null) {
                        return;
                    }
                    double value = valueTmp;
                    writeClientNumberPreference(item.ID, value);
                }
                // Texts
                else if(item.dialogButtonType == PrefsListItemWrapper.DialogButtonType.TEXT) {
                    EditText input = dialog.findViewById(R.id.Input);

                    if(item.ID == R.string.prefs_general_device_name_header) {
                        try {
                            if(!BOINCActivity.monitor.setDomainName(input.getText().toString())) {
                                if(Logging.DEBUG) {
                                    Log.d(Logging.TAG, "PrefsFragment.setupDialogButtons.onClick.setDomainName(): false");
                                }
                            }
                        }
                        catch(Exception e) {
                            if(Logging.ERROR) {
                                Log.e(Logging.TAG, "PrefsFragment.setupDialogButtons.onClick(): error: " + e);
                            }
                        }
                    }

                    updateTextPreference(item.ID, input.getText().toString());
                }
                dialog.dismiss();
            }
        });

        // Cancel
        Button cancel = dialog.findViewById(R.id.cancel);
        cancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
            }
        });
    }

    private void writeClientNumberPreference(int id, double value) {
        // Update preferences
        switch(id) {
            case R.string.prefs_disk_max_pct_header:
                clientPrefs.disk_max_used_pct = value;
                break;
            case R.string.prefs_disk_min_free_gb_header:
                clientPrefs.disk_min_free_gb = value;
                break;
            case R.string.prefs_disk_access_interval_header:
                clientPrefs.disk_interval = value;
                break;
            case R.string.prefs_network_daily_xfer_limit_mb_header:
                clientPrefs.daily_xfer_limit_mb = value;
                // also need to set the period!
                clientPrefs.daily_xfer_period_days = 1;
                break;
            case R.string.battery_charge_min_pct_header:
                clientPrefs.battery_charge_min_pct = value;
                break;
            case R.string.battery_temperature_max_header:
                clientPrefs.battery_max_temperature = value;
                break;
            case R.string.prefs_cpu_number_cpus_header:
                clientPrefs.max_ncpus_pct = value;
                value = pctCpuCoresToNumber(value); // Convert value back to number for layout update
                break;
            case R.string.prefs_cpu_time_max_header:
                clientPrefs.cpu_usage_limit = value;
                break;
            case R.string.prefs_cpu_other_load_suspension_header:
                clientPrefs.suspend_cpu_usage = value;
                break;
            case R.string.prefs_memory_max_idle_header:
                clientPrefs.ram_max_used_idle_frac = value;
                break;
            case R.string.prefs_other_store_at_least_x_days_of_work_header:
                clientPrefs.work_buf_min_days = value;
                break;
            case R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header:
                clientPrefs.work_buf_additional_days = value;
                break;
            default:
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "onClick (dialog submit button), couldnt match ID");
                }
                Toast toast = Toast.makeText(getActivity(), "ooops! something went wrong...", Toast.LENGTH_SHORT);
                toast.show();
                return;
        }
        // Update list item
        updateNumberPreference(id, value);
        // Preferences adapted, write preferences to client
        new WriteClientPrefsAsync().execute(clientPrefs);
    }

    private double numberCpuCoresToPct(double ncpus) {
        double pct = (ncpus / (double) hostinfo.p_ncpus) * 100;
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "numberCpuCoresToPct: " + ncpus + hostinfo.p_ncpus + pct);
        }
        return pct;
    }

    private double pctCpuCoresToNumber(double pct) {
        double ncpus = (double) hostinfo.p_ncpus * (pct / 100.0);
        if(ncpus < 1.0) {
            ncpus = 1.0;
        }
        return ncpus;
    }

    public Double parseInputValueToDouble(String input) {
        // Parse value
        Double value;
        try {
            input = input.replaceAll(",", "."); //Replace e.g. European decimal seperator "," by "."
            value = Double.parseDouble(input);
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "parseInputValueToDouble: " + value);
            }
            return value;
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, e);
            }
            Toast toast = Toast.makeText(getActivity(), "wrong format!", Toast.LENGTH_SHORT);
            toast.show();
            return null;
        }
    }

    private String formatOptionsToCcConfig(ArrayList<String> options) {
        StringBuilder builder = new StringBuilder();
        builder.append("<cc_config>\n <log_flags>\n");
        for(String option : options) {
            builder.append("  <").append(option).append("/>\n");
        }
        builder.append(" </log_flags>\n <options>\n </options>\n</cc_config>");
        return builder.toString();
    }

    public class BoolOnClick implements OnClickListener {

        private Integer ID;
        private CheckBox cb;

        public BoolOnClick(Integer ID, CheckBox cb) {
            this.ID = ID;
            this.cb = cb;
        }

        @Override
        public void onClick(View view) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "onCbClick");
            }
            Boolean previousState = cb.isChecked();
            cb.setChecked(!previousState);
            Boolean isSet = cb.isChecked();
            try {
                switch(ID) {
                    case R.string.prefs_autostart_header: //app pref
                        BOINCActivity.monitor.setAutostart(isSet);
                        updateBoolPreference(ID, isSet);
                        updateLayout();
                        break;
                    case R.string.prefs_show_notification_notices_header: //app pref
                        BOINCActivity.monitor.setShowNotificationForNotices(isSet);
                        updateBoolPreference(ID, isSet);
                        updateLayout();
                        break;
                    case R.string.prefs_show_advanced_header: //app pref
                        BOINCActivity.monitor.setShowAdvanced(isSet);
                        // reload complete layout to remove/add advanced elements
                        populateLayout();
                        break;
                    case R.string.prefs_suspend_when_screen_on: //app pref
                        BOINCActivity.monitor.setSuspendWhenScreenOn(isSet);
                        updateBoolPreference(ID, isSet);
                        updateLayout();
                        break;
                    case R.string.prefs_network_wifi_only_header: //client pref
                        clientPrefs.network_wifi_only = isSet;
                        updateBoolPreference(ID, isSet);
                        new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
                        break;
                    case R.string.prefs_stationary_device_mode_header: //app pref
                        BOINCActivity.monitor.setStationaryDeviceMode(isSet);
                        // reload complete layout to remove/add power preference elements
                        populateLayout();
                        break;
                }
            }
            catch(RemoteException e) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "PrefsFragment.BoolOnClick: onClick() error: ", e);
                }
            }
        }

    }

    public class ValueOnClick implements OnClickListener {

        private PrefsListItemWrapper item;

        public ValueOnClick(PrefsListItemWrapper wrapper) {
            this.item = wrapper;
        }

        @Override
        public void onClick(View view) {
            final Dialog dialog = new Dialog(getActivity());
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);

            // setup dialog layout
            switch(item.ID) {
                case R.string.prefs_general_device_name_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_text);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_network_daily_xfer_limit_mb_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_power_source_header:
                    try {
                        setupSelectionListDialog(item, dialog);
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "PrefsFragment.ValueOnClick.onClick() error: ", e);
                        }
                    }
                    break;
                case R.string.battery_charge_min_pct_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.battery_temperature_max_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_cpu_number_cpus_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.prefs_cpu_time_max_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.prefs_cpu_other_load_suspension_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.prefs_disk_max_pct_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.prefs_disk_min_free_gb_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_disk_access_interval_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_memory_max_idle_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                case R.string.prefs_other_store_at_least_x_days_of_work_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header:
                    dialog.setContentView(R.layout.prefs_layout_dialog_number);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    setupDialogButtons(item, dialog);
                    break;
                case R.string.prefs_client_log_flags_header:
                    try {
                        setupSelectionListDialog(item, dialog);
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "PrefsFragment.ValueOnClick.onClick() error: ", e);
                        }
                    }
                    break;
                case R.string.prefs_gui_log_level_header:
                    setupSliderDialog(item, dialog);
                    ((TextView) dialog.findViewById(R.id.pref)).setText(item.ID);
                    break;
                default:
                    if(Logging.ERROR) {
                        Log.d(Logging.TAG, "PrefsActivity onItemClick: could not map ID: " + item.ID);
                    }
                    return;
            }

            // show dialog
            dialog.show();
        }
    }

    private final class WriteClientPrefsAsync extends AsyncTask<GlobalPreferences, Void, Boolean> {
        @Override
        protected Boolean doInBackground(GlobalPreferences... params) {
            try {
                return BOINCActivity.monitor.setGlobalPreferences(params[0]);
            }
            catch(RemoteException e) {
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "WriteClientPrefsAsync returned: " + success);
            }
            updateLayout();
        }
    }

    private final class SetCcConfigAsync extends AsyncTask<String, Void, Boolean> {
        @Override
        protected Boolean doInBackground(String... params) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "SetCcConfigAsync with: " + params[0]);
            }
            try {
                return BOINCActivity.monitor.setCcConfig(params[0]);
            }
            catch(RemoteException e) {
                return false;
            }
        }
    }

    public class SelectionDialogOption {
        public String name;
        public Integer ID = null;
        public Boolean selected = false;
        public Boolean highlighted = false;

        public SelectionDialogOption(String name) {
            this.name = name;
        }

        public SelectionDialogOption(String name, Boolean selected) {
            this(name);
            this.selected = selected;
        }

        public SelectionDialogOption(String name, Boolean selected, Boolean highlighted) {
            this(name, selected);
            this.highlighted = highlighted;
        }

        public SelectionDialogOption(int ID, Boolean selected) {
            this(getResources().getString(ID), selected);
            this.ID = ID;
        }

        public SelectionDialogOption(int ID, Boolean selected, Boolean highlighted) {
            this(getResources().getString(ID), selected, highlighted);
            this.ID = ID;
        }
    }
}
