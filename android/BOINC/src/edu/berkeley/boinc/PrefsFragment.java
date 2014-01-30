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
import edu.berkeley.boinc.adapter.PrefsListAdapter;
import edu.berkeley.boinc.adapter.PrefsListItemWrapper;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperBool;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperValue;
import edu.berkeley.boinc.adapter.PrefsSelectionDialogListAdapter;
import edu.berkeley.boinc.client.ClientNotification;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.DeviceStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import android.app.Dialog;
import android.os.AsyncTask;
import android.os.Bundle;
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

public class PrefsFragment extends Fragment {
	
	private ListView lv;
	private PrefsListAdapter listAdapter;
	
	private ArrayList<PrefsListItemWrapper> data = new ArrayList<PrefsListItemWrapper>(); //Adapter for list data
	private GlobalPreferences clientPrefs = null; //preferences of the client, read on every onResume via RPC
	private AppPreferences appPrefs = null; //Android specific preferences, singleton of monitor
	private HostInfo hostinfo = null;
	
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
		appPrefs = Monitor.getAppPrefs();
		super.onCreate(savedInstanceState);
	}

	// fragment lifecycle: 3.
	@Override
	public void onResume() {
		super.onResume();
		populateLayout();
	}
	
	private Boolean getPrefs() {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"PrefsActivity: Could not load data, clientStatus not initialized.");
			return false;
		}
		clientPrefs = status.getPrefs(); //read prefs from client via rpc
		if(clientPrefs == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs: null, return false");
			return false;
		}
		//if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs done");
		return true;
	}
	
	private Boolean getHostInfo() {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"PrefsActivity: Could not load data, clientStatus not initialized.");
			return false;
		}
		hostinfo = status.getHostInfo(); //Get the hostinfo from client via rpc
		if(hostinfo == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "getHostInfo: null, return false");
			return false;
		}
		return true;
	}
	
	private void populateLayout() {
		
		if(!getPrefs() || appPrefs == null || !getHostInfo()) {
			if(Logging.ERROR) Log.e(Logging.TAG, "PrefsFragment.populateLayout returns, data is not present");
			return;
		}
		
		data.clear();
		
		Boolean advanced = appPrefs.getShowAdvanced();
		Boolean stationaryDevice = false;
		try{
			DeviceStatus deviceStatus  = Monitor.getDeviceStatus();
			stationaryDevice = deviceStatus.isStationaryDevice();
		} catch (Exception e){
			if(Logging.ERROR) Log.e(Logging.TAG, "PrefsFragment.populateLayout failed to retrieve device status. treat device as non-stationary");
		}

		// general
    	data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_general,true));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_autostart_header,R.string.prefs_category_general,appPrefs.getAutostart()));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_show_notification_header,R.string.prefs_category_general,appPrefs.getShowNotification())); 
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_show_advanced_header,R.string.prefs_category_general,appPrefs.getShowAdvanced()));
		// network
    	data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_network,true));
		data.add(new PrefsListItemWrapperBool(getActivity(),R.string.prefs_network_wifi_only_header,R.string.prefs_category_network,clientPrefs.network_wifi_only));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_network_daily_xfer_limit_mb_header,R.string.prefs_category_network,clientPrefs.daily_xfer_limit_mb));
    	// power
		if(!stationaryDevice) {
			data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_power,true));
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
		// memory
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_memory,true));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_memory_max_idle_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_idle_frac));
		// debug
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_category_debug,true));
		if(advanced) data.add(new PrefsListItemWrapper(getActivity(),R.string.prefs_client_log_flags_header,R.string.prefs_category_debug));
		if(advanced) data.add(new PrefsListItemWrapperValue(getActivity(),R.string.prefs_gui_log_level_header,R.string.prefs_category_debug,(double)appPrefs.getLogLevel()));
		
		updateLayout();
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
			sliderProgress.setText(valueWrapper.status.intValue() + " " + getString(R.string.prefs_unit_pct));
			Double seekBarDefault = valueWrapper.status / 10;
			slider.setProgress(seekBarDefault.intValue());
			slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
		        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
		        	String progressString = (progress * 10) + " " + getString(R.string.prefs_unit_pct);
		        	TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
		            sliderProgress.setText(progressString);
		        }
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
		    });
		} else if(valueWrapper.ID == R.string.prefs_cpu_number_cpus_header) {
			if(!getHostInfo()) {
				if(Logging.WARNING) Log.w(Logging.TAG, "onItemClick missing hostInfo");
				return;
			}
			sliderProgress.setText(""+valueWrapper.status.intValue());
			slider.setMax(hostinfo.p_ncpus);
			slider.setProgress(valueWrapper.status.intValue());
			slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
					if(progress == 0) progress = 1; // do not allow 0 cpus
					String progressString = String.valueOf(progress);
					TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
					sliderProgress.setText(progressString);
				}
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
			});
		} else if(valueWrapper.ID == R.string.prefs_gui_log_level_header) {
			sliderProgress.setText(""+valueWrapper.status.intValue());
			slider.setMax(5);
			slider.setProgress(valueWrapper.status.intValue());
			slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
					String progressString = String.valueOf(progress);
					TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
					sliderProgress.setText(progressString);
				}
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
			});
		}
		
		setupDialogButtons(item, dialog);
	}
	
	private void setupSelectionListDialog(final PrefsListItemWrapper item, final Dialog dialog) {
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
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_ac), appPrefs.getPowerSourceAc()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_usb), appPrefs.getPowerSourceUsb()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_wireless), appPrefs.getPowerSourceWireless()));
			options.add(new SelectionDialogOption(getResources().getString(R.string.prefs_power_source_battery), clientPrefs.run_on_batteries, true));
			ListView lv = (ListView) dialog.findViewById(R.id.selection);
			new PrefsSelectionDialogListAdapter(getActivity(), lv, R.id.selection, options);

			// setup confirm button action
			Button confirm = (Button) dialog.findViewById(R.id.confirm);
			confirm.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					for(SelectionDialogOption option: options) {
						if(option.name == getResources().getString(R.string.prefs_power_source_ac)) appPrefs.setPowerSourceAc(option.selected);
						if(option.name == getResources().getString(R.string.prefs_power_source_usb)) appPrefs.setPowerSourceUsb(option.selected);
						if(option.name == getResources().getString(R.string.prefs_power_source_wireless)) appPrefs.setPowerSourceWireless(option.selected);
						if(option.name == getResources().getString(R.string.prefs_power_source_battery)) {
							clientPrefs.run_on_batteries = option.selected;
							new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
						}
					}
					dialog.dismiss();
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
         		   if(sbProgress == 0) sbProgress = 1;
         		   double value = numberCpuCoresToPct(sbProgress);
         		   writeClientValuePreference(item.ID, value);
         	   } else if(item.ID == R.string.prefs_gui_log_level_header) {
         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
         		   int sbProgress = slider.getProgress();
         		   appPrefs.setLogLevel(sbProgress);
         		   updateValuePref(item.ID, (double) sbProgress);
         		   updateLayout();
         	   } else if(item.ID == R.string.prefs_network_daily_xfer_limit_mb_header || item.ID == R.string.battery_temperature_max_header || item.ID == R.string.prefs_disk_min_free_gb_header) {
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
		case R.string.prefs_network_daily_xfer_limit_mb_header:
			clientPrefs.daily_xfer_limit_mb = value;
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
			Boolean isSet = cb.isChecked();
			
			switch (ID) {
			case R.string.prefs_autostart_header: //app pref
				appPrefs.setAutostart(isSet);
				updateBoolPref(ID, isSet);
				updateLayout();
				break;
			case R.string.prefs_show_notification_header: //app pref
				appPrefs.setShowNotification(isSet);
				if(isSet) ClientNotification.getInstance(getActivity()).update();
				else ClientNotification.getInstance(getActivity()).cancel();
				updateBoolPref(ID, isSet);
				updateLayout();
				break;
			case R.string.prefs_show_advanced_header: //app pref
				appPrefs.setShowAdvanced(isSet);
				// reload complete layout to remove/add advanced elements
				populateLayout();
				break;
			case R.string.prefs_network_wifi_only_header: //client pref
				clientPrefs.network_wifi_only = isSet;
				updateBoolPref(ID, isSet);
				new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
				break;
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
			case R.string.prefs_network_daily_xfer_limit_mb_header:
				dialog.setContentView(R.layout.prefs_layout_dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				setupDialogButtons(item, dialog);
				break;
			case R.string.prefs_power_source_header:
				setupSelectionListDialog(item, dialog);
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
			case R.string.prefs_memory_max_idle_header:
				setupSliderDialog(item, dialog);
				((TextView)dialog.findViewById(R.id.pref)).setText(item.ID);
				break;
			case R.string.prefs_client_log_flags_header:
				setupSelectionListDialog(item, dialog);
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
			return ((BOINCActivity) getActivity()).getMonitorService().clientInterface.setGlobalPreferences(params[0]);
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
			return ((BOINCActivity) getActivity()).getMonitorService().clientInterface.setCcConfig(params[0]);
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
