/*******************************************************************************
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
 ******************************************************************************/
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
import edu.berkeley.boinc.adapter.PrefsListItemWrapperValue;
import edu.berkeley.boinc.adapter.PrefsSelectionDialogListAdapter;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import edu.berkeley.boinc.utils.Logging;
import java.text.NumberFormat;
import java.util.ArrayList;

public class PrefsFragment extends Fragment {
	
	private ListView lv;
	private PrefsListAdapter listAdapter;
	
	private ArrayList<PrefsListItemWrapper> data = new ArrayList<PrefsListItemWrapper>(); //Adapter for list data
	private GlobalPreferences clientPrefs = null; //preferences of the client, read on every onResume via RPC
	//private AppPreferences appPrefs = null; //Android specific preferences, singleton of monitor
	private HostInfo hostinfo = null;
	
	private boolean layoutSuccessful = false;
	
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			if(Logging.VERBOSE) Log.d(Logging.TAG, "PrefsFragment ClientStatusChange - onReceive()"); 
			try {
				if(!layoutSuccessful) populateLayout();
			} catch (RemoteException e) {}
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	// fragment lifecycle: 2.
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    	if(Logging.VERBOSE) Log.d(Logging.TAG,"ProjectsFragment onCreateView");
        // Inflate the layout for this fragment
    	View layout = inflater.inflate(R.layout.prefs_layout, container, false);
		lv = (ListView) layout.findViewById(R.id.listview);
        listAdapter = new PrefsListAdapter(getActivity(),this,R.id.listview,data);
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
		} catch (RemoteException e) {}
		getActivity().registerReceiver(mClientStatusChangeRec,ifcsc);
		super.onResume();
	}
	
	@Override
	public void onPause() {
		getActivity().unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	private Boolean getPrefs() {
		// try to get current client status from monitor
		//ClientStatus status;
		try{
			//status  = Monitor.getClientStatus();
			clientPrefs = BOINCActivity.monitor.getPrefs();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"PrefsActivity: Could not load data, clientStatus not initialized.");
			e.printStackTrace();
			return false;
		}
		//clientPrefs = status.getPrefs(); //read prefs from client via rpc
		if(clientPrefs == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs: null, return false");
			return false;
		}
		//if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs done");
		return true;
	}
	
	private Boolean getHostInfo() {
		// try to get current client status from monitor
		//ClientStatus status;
		
		try{
			//status  = Monitor.getClientStatus();
			hostinfo = BOINCActivity.monitor.getHostInfo();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"PrefsActivity: Could not load data, clientStatus not initialized.");
			e.printStackTrace();
			return false;
		}
		//hostinfo = status.getHostInfo(); //Get the hostinfo from client via rpc
		if(hostinfo == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "getHostInfo: null, return false");
			return false;
		}
		return true;
	}
	
	private void populateLayout() throws RemoteException{
		
		if(!getPrefs() || BOINCActivity.monitor == null || !getHostInfo()) {
			if(Logging.ERROR) Log.e(Logging.TAG, "PrefsFragment.populateLayout returns, data is not present");
			return;
		}
		
		data.clear();
		
		Boolean advanced = BOINCActivity.monitor.getShowAdvanced();
		Boolean stationaryDeviceMode = BOINCActivity.monitor.getStationaryDeviceMode();
		Boolean stationaryDeviceSuspected = BOINCActivity.monitor.isStationaryDeviceSuspected();

		// general
    	data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_general,true));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_autostart_header,R.string.prefs_category_general,BOINCActivity.monitor.getAutostart()));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_show_notification_notices_header,R.string.prefs_category_general,BOINCActivity.monitor.getShowNotificationForNotices()));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_show_notification_suspended_header,R.string.prefs_category_general,BOINCActivity.monitor.getShowNotificationDuringSuspend()));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_show_advanced_header,R.string.prefs_category_general,BOINCActivity.monitor.getShowAdvanced()));
		if(!stationaryDeviceMode) data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_suspend_when_screen_on,R.string.prefs_category_general,BOINCActivity.monitor.getSuspendWhenScreenOn()));
		// network
    	data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_network,true));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_network_wifi_only_header,R.string.prefs_category_network,clientPrefs.network_wifi_only));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_network_daily_xfer_limit_mb_header,R.string.prefs_category_network,clientPrefs.daily_xfer_limit_mb));
    	// power
		data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_power,true));
		if(stationaryDeviceSuspected) { // API indicates that there is no battery, offer opt-in preference for stationary device mode
			data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_stationary_device_mode_header,R.string.prefs_category_power,BOINCActivity.monitor.getStationaryDeviceMode()));
		}
		if(!stationaryDeviceMode) { // client would compute regardless of battery preferences, so only show if that is not the case
			data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_power_source_header,R.string.prefs_category_power));
			data.add(new PrefsListItemWrapperValue(getActivity(),R.string.battery_charge_min_pct_header,R.string.prefs_category_power,clientPrefs.battery_charge_min_pct));
			if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.battery_temperature_max_header,R.string.prefs_category_power,clientPrefs.battery_max_temperature));
		}
		// cpu
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_cpu,true));
		if(advanced && hostinfo.p_ncpus > 1) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_cpu_number_cpus_header,R.string.prefs_category_cpu,pctCpuCoresToNumber(clientPrefs.max_ncpus_pct)));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_cpu_time_max_header,R.string.prefs_category_cpu,clientPrefs.cpu_usage_limit));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_cpu_other_load_suspension_header,R.string.prefs_category_cpu,clientPrefs.suspend_cpu_usage));
		// storage
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_storage,true));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_disk_max_pct_header,R.string.prefs_category_storage,clientPrefs.disk_max_used_pct));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_disk_min_free_gb_header,R.string.prefs_category_storage,clientPrefs.disk_min_free_gb));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_disk_access_interval_header,R.string.prefs_category_storage,clientPrefs.disk_interval));
		// memory
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_memory,true));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_memory_max_idle_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_idle_frac));
		// debug
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_debug,true));
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_client_log_flags_header,R.string.prefs_category_debug));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_gui_log_level_header,R.string.prefs_category_debug,(double)BOINCActivity.monitor.getLogLevel()));
		
		updateLayout();
		layoutSuccessful = true;
	}
	
	private void updateLayout(){
		listAdapter.notifyDataSetChanged();
	}

	// updates list item of boolean preference
	// requires updateLayout to be called afterwards
	private void updateBoolPref(int ID, Boolean newValue) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateBoolPref for ID: " + ID + " value: " + newValue);
		for (PrefsListItemWrapper item: data) {
			if(item.ID == ID){
				((PrefsListItemWrapperBool) item).setStatus(newValue);
				continue;
			}
		}
	}
	
	// updates list item of value preference
	// requires updateLayout to be called afterwards
	private void updateValuePref(int ID, Double newValue) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateValuePref for ID: " + ID + " value: " + newValue);
		for (PrefsListItemWrapper item: data) {
			if(item.ID == ID){
				((PrefsListItemWrapperValue) item).status = newValue;
				continue;
			}
		}
	}
	
	private void setupSliderDialog(PrefsListItemWrapper item, final Dialog dialog) {
		final PrefsListItemWrapperValue valueWrapper = (PrefsListItemWrapperValue) item;
		dialog.setContentView(R.layout.prefs_layout_dialog_pct);
		TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
		SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
		
		if(valueWrapper.ID == R.string.battery_charge_min_pct_header || 
				valueWrapper.ID == R.string.prefs_disk_max_pct_header || 
				valueWrapper.ID == R.string.prefs_cpu_time_max_header ||
				valueWrapper.ID == R.string.prefs_cpu_other_load_suspension_header || 
				valueWrapper.ID == R.string.prefs_memory_max_idle_header ) {
			Double seekBarDefault = valueWrapper.status / 10;
			slider.setProgress(seekBarDefault.intValue());
			final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
			slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
				public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
					final String progressString = NumberFormat.getPercentInstance().format(progress / 10.0);
					TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
					sliderProgress.setText(progressString);
		        }
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
			});
			onSeekBarChangeListener.onProgressChanged(slider, seekBarDefault.intValue(), false);
		} else if(valueWrapper.ID == R.string.prefs_cpu_number_cpus_header) {
			if(!getHostInfo()) {
				if(Logging.WARNING) Log.w(Logging.TAG, "onItemClick missing hostInfo");
				return;
			}
			slider.setMax(hostinfo.p_ncpus <= 1 ? 0 : hostinfo.p_ncpus - 1);
			final int statusValue;
			slider.setProgress((statusValue = valueWrapper.status.intValue()) <= 0 ?
				0 :
				statusValue - 1 > slider.getMax() ?
					slider.getMax() :
					statusValue - 1);
			Log.d(Logging.TAG, String.format("statusValue == %,d", statusValue));
			final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
			slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
				public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
					final String progressString = NumberFormat.getIntegerInstance().format(progress <= 0 ? 1 : progress + 1); // do not allow 0 cpus
					TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
					sliderProgress.setText(progressString);
				}
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
			});
			onSeekBarChangeListener.onProgressChanged(slider, statusValue - 1, false);
		} else if(valueWrapper.ID == R.string.prefs_gui_log_level_header) {
			slider.setMax(5);
			slider.setProgress(valueWrapper.status.intValue());
			final SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
			slider.setOnSeekBarChangeListener(onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
				public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
					String progressString = NumberFormat.getIntegerInstance().format(progress);
					TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
					sliderProgress.setText(progressString);
				}
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
			});
			onSeekBarChangeListener.onProgressChanged(slider, valueWrapper.status.intValue(), false);
		}
		
		setupDialogButtons(item, dialog);
	}
	
	private void setupSelectionListDialog(final PrefsListItemWrapper item, final Dialog dialog) throws RemoteException{
		dialog.setContentView(R.layout.prefs_layout_dialog_selection);
		
		if(item.ID == R.string.prefs_client_log_flags_header) {
			final ArrayList<SelectionDialogOption> options = new ArrayList<SelectionDialogOption>();
			String[] array = getResources().getStringArray(R.array.prefs_client_log_flags);
			for(String option: array) options.add(new SelectionDialogOption(option));
			ListView lv = (ListView) dialog.findViewById(R.id.selection);
			new PrefsSelectionDialogListAdapter(getActivity(), lv, R.id.selection, options);

			// setup confirm button action
			Button confirm = (Button) dialog.findViewById(R.id.confirm);
			confirm.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					ArrayList<String> selectedOptions = new ArrayList<String>();
					for(SelectionDialogOption option: options) if(option.selected) selectedOptions.add(option.name);
					if(Logging.DEBUG) Log.d(Logging.TAG, selectedOptions.size() + " log flags selected");
					new SetCcConfigAsync().execute(formatOptionsToCcConfig(selectedOptions)); 
					dialog.dismiss();
				}
			});
		}else if(item.ID == R.string.prefs_power_source_header) {
			final ArrayList<SelectionDialogOption> options = new ArrayList<SelectionDialogOption>();
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_ac), BOINCActivity.monitor.getPowerSourceAc()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_usb), BOINCActivity.monitor.getPowerSourceUsb()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_wireless), BOINCActivity.monitor.getPowerSourceWireless()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_battery), clientPrefs.run_on_batteries, true));
			ListView lv = (ListView) dialog.findViewById(R.id.selection);
			new PrefsSelectionDialogListAdapter(getActivity(), lv, R.id.selection, options);

			// setup confirm button action
			Button confirm = (Button) dialog.findViewById(R.id.confirm);
			confirm.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					try{
						for(SelectionDialogOption option: options) {
							if(option.name == getResources().getString(R.string.prefs_power_source_ac))
								BOINCActivity.monitor.setPowerSourceAc(option.selected);
							if(option.name == getResources().getString(R.string.prefs_power_source_usb))
								BOINCActivity.monitor.setPowerSourceUsb(option.selected);
							if(option.name == getResources().getString(R.string.prefs_power_source_wireless))
								BOINCActivity.monitor.setPowerSourceWireless(option.selected);
							if(option.name == getResources().getString(R.string.prefs_power_source_battery)) {
								clientPrefs.run_on_batteries = option.selected;
								new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
							}
						}
						dialog.dismiss();
					} catch(RemoteException e) {}
				}
			});
		}
		
		// generic cancel button
		Button cancel = (Button) dialog.findViewById(R.id.cancel);
		cancel.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				dialog.dismiss();
			}
		});
	}
		
	private void setupDialogButtons(final PrefsListItemWrapper item, final Dialog dialog) {
		// confirm
		Button confirm = (Button) dialog.findViewById(R.id.confirm);
		confirm.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
         	   if(item.ID == R.string.battery_charge_min_pct_header || 
         			item.ID == R.string.prefs_disk_max_pct_header || 
         			item.ID == R.string.prefs_cpu_time_max_header ||
         			item.ID == R.string.prefs_cpu_other_load_suspension_header || 
         			item.ID == R.string.prefs_memory_max_idle_header ) {
         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
         		   double value = slider.getProgress()*10;
         		   writeClientValuePreference(item.ID, value);
         	   } else if(item.ID == R.string.prefs_cpu_number_cpus_header) {
         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
         		   int sbProgress = slider.getProgress();
         		   double value = numberCpuCoresToPct(sbProgress <= 0 ? 1 : sbProgress + 1);
         		   writeClientValuePreference(item.ID, value);
         	   } else if(item.ID == R.string.prefs_gui_log_level_header) {
         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
         		   int sbProgress = slider.getProgress();
         		   try {
         			   // monitor and UI in two different processes. set static variable in both
          			  Logging.setLogLevel(sbProgress);
         			  BOINCActivity.monitor.setLogLevel(sbProgress);
         		   } catch (RemoteException e) {}
         		   updateValuePref(item.ID, (double) sbProgress);
         		   updateLayout();
         	   } else if(item.ID == R.string.prefs_network_daily_xfer_limit_mb_header ||
         			   item.ID == R.string.battery_temperature_max_header ||
         			   item.ID == R.string.prefs_disk_min_free_gb_header ||
         			   item.ID == R.string.prefs_disk_access_interval_header) {
         		   EditText edit = (EditText) dialog.findViewById(R.id.Input);
         		   String input = edit.getText().toString();
         		   Double valueTmp = parseInputValueToDouble(input);
         		   if(valueTmp == null) return;
         		   double value = valueTmp;
         		   writeClientValuePreference(item.ID, value);
         	   }
         	   dialog.dismiss();
			}
		});
		// cancel
		Button cancel = (Button) dialog.findViewById(R.id.cancel);
		cancel.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				dialog.dismiss();
			}
		});
	}
	
	private void writeClientValuePreference(int id, double value) {
		// update preferences
		switch (id) {
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
			//convert value back to number for layout update
			value = pctCpuCoresToNumber(value);
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
		default:
			if(Logging.DEBUG) Log.d(Logging.TAG,"onClick (dialog submit button), couldnt match ID");
			Toast toast = Toast.makeText(getActivity(), "ooops! something went wrong...", Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		// update list item
		updateValuePref(id, value);
		// preferences adapted, write preferences to client
		new WriteClientPrefsAsync().execute(clientPrefs);
	}
	
	private double numberCpuCoresToPct(double ncpus) {
		double pct = (ncpus / (double)hostinfo.p_ncpus) * 100;
		if(Logging.DEBUG) Log.d(Logging.TAG,"numberCpuCoresToPct: " + ncpus + hostinfo.p_ncpus + pct);
		return pct;
	}
	
	private double pctCpuCoresToNumber(double pct) {
		double ncpus = (double)hostinfo.p_ncpus * (pct / 100.0);
		if(ncpus < 1.0) ncpus = 1.0;
		return ncpus;
	}

	public Double parseInputValueToDouble(String input) {
		// parse value
		Double value = 0.0;
		try {
			input=input.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
			value = Double.parseDouble(input);
			if(Logging.DEBUG) Log.d(Logging.TAG,"parseInputValueToDouble: " + value);
			return value;
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG, e);
			Toast toast = Toast.makeText(getActivity(), "wrong format!", Toast.LENGTH_SHORT);
			toast.show();
			return null;
		}
	}
	
	private String formatOptionsToCcConfig(ArrayList<String> options) {
		StringBuilder builder = new StringBuilder();
		builder.append("<cc_config>\n <log_flags>\n");
	    for(String option: options) builder.append("  <" + option + "/>\n");
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
			if(Logging.DEBUG) Log.d(Logging.TAG,"onCbClick");
			Boolean previousState = cb.isChecked();
			cb.setChecked(!previousState);
			Boolean isSet = cb.isChecked();
			try{
				switch (ID) {
				case R.string.prefs_autostart_header: //app pref
					BOINCActivity.monitor.setAutostart(isSet);
					updateBoolPref(ID, isSet);
					updateLayout();
					break;
				case R.string.prefs_show_notification_notices_header: //app pref
					BOINCActivity.monitor.setShowNotificationForNotices(isSet);
					updateBoolPref(ID, isSet);
					updateLayout();
					break;
				case R.string.prefs_show_notification_suspended_header: //app pref
					BOINCActivity.monitor.setShowNotificationDuringSuspend(isSet);
					updateBoolPref(ID, isSet);
					updateLayout();
					break;
				case R.string.prefs_show_advanced_header: //app pref
					BOINCActivity.monitor.setShowAdvanced(isSet);
					// reload complete layout to remove/add advanced elements
					populateLayout();
					break;
				case R.string.prefs_suspend_when_screen_on: //app pref
					BOINCActivity.monitor.setSuspendWhenScreenOn(isSet);
					updateBoolPref(ID, isSet);
					updateLayout();
					break;
				case R.string.prefs_network_wifi_only_header: //client pref
					clientPrefs.network_wifi_only = isSet;
					updateBoolPref(ID, isSet);
					new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
					break;
				case R.string.prefs_stationary_device_mode_header: //app pref
					BOINCActivity.monitor.setStationaryDeviceMode(isSet);
					// reload complete layout to remove/add power preference elements
					populateLayout();
					break;
				}
			} catch(RemoteException e) {}
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
			case R.string.prefs_network_daily_xfer_limit_mb_header:
				dialog.setContentView(R.layout.prefs_layout_dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				setupDialogButtons(item, dialog);
				break;
			case R.string.prefs_power_source_header:
				try {
					setupSelectionListDialog(item, dialog);
				} catch (RemoteException e) {}
				break;
			case R.string.battery_charge_min_pct_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.battery_temperature_max_header:
				dialog.setContentView(R.layout.prefs_layout_dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				setupDialogButtons(item, dialog);
				break;
			case R.string.prefs_cpu_number_cpus_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_cpu_time_max_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_cpu_other_load_suspension_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_disk_max_pct_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_disk_min_free_gb_header:
				dialog.setContentView(R.layout.prefs_layout_dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				setupDialogButtons(item, dialog);
				break;
			case R.string.prefs_disk_access_interval_header:
				dialog.setContentView(R.layout.prefs_layout_dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				setupDialogButtons(item, dialog);
				break;
			case R.string.prefs_memory_max_idle_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_client_log_flags_header:
				try {
					setupSelectionListDialog(item, dialog);
				} catch (RemoteException e) {}
				break;
			case R.string.prefs_gui_log_level_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			default:
				if(Logging.ERROR) Log.d(Logging.TAG,"PrefsActivity onItemClick: could not map ID: " + item.ID);
				return;
			}
			
			// show dialog
			dialog.show();
		}
	}
	
	private final class WriteClientPrefsAsync extends AsyncTask<GlobalPreferences,Void,Boolean> {
		@Override
		protected Boolean doInBackground(GlobalPreferences... params) {
			try {
				return BOINCActivity.monitor.setGlobalPreferences(params[0]);
			} catch (RemoteException e) {
				return false;
			}
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"WriteClientPrefsAsync returned: " + success);
			updateLayout();
		}
	}
	
	private final class SetCcConfigAsync extends AsyncTask<String,Void,Boolean> {
		@Override
		protected Boolean doInBackground(String... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"SetCcConfigAsync with: " + params[0]);
			try {
				return BOINCActivity.monitor.setCcConfig(params[0]);
			} catch (RemoteException e) {
				return false;
			}
		}
	}
	
	public class SelectionDialogOption {
		public String name;
		public Boolean selected = false;
		public Boolean highlighted = false;
		
		public SelectionDialogOption(String name) {
			this.name = name;
		}
		
		public SelectionDialogOption(String name, Boolean selected) {
			this.name = name;
			this.selected = selected;
		}
		
		public SelectionDialogOption(String name, Boolean selected, Boolean highlighted) {
			this.name = name;
			this.selected = selected;
			this.highlighted = highlighted;
		}
	}
}
