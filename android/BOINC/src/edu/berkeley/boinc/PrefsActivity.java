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
import edu.berkeley.boinc.client.ClientNotification;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class PrefsActivity extends FragmentActivity {
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private ListView lv;
	private PrefsListAdapter listAdapter;
	
	private ArrayList<PrefsListItemWrapper> data = new ArrayList<PrefsListItemWrapper>(); //Adapter for list data
	private GlobalPreferences clientPrefs = null; //preferences of the client, read on every onResume via RPC
	private AppPreferences appPrefs = null; //Android specific preferences, singleton of monitor
	private HostInfo hostinfo = null;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		doBindService();
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	if(Logging.DEBUG) Log.d(Logging.TAG,"PrefsActivity onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
			appPrefs = Monitor.getAppPrefs();
			if(Logging.DEBUG) Log.d(Logging.TAG, "appPrefs available");
			populateLayout();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
	        mIsBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
		}
	}

	private void doUnbindService() {
	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	private Boolean getPrefs() {
		clientPrefs = Monitor.getClientStatus().getPrefs(); //read prefs from client via rpc
		if(clientPrefs == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs: null, return false");
			return false;
		}
		//if(Logging.DEBUG) Log.d(Logging.TAG, "readPrefs done");
		return true;
	}
	
	private Boolean getHostInfo() {
		hostinfo = Monitor.getClientStatus().getHostInfo(); //Get the hostinfo from client via rpc
		if(hostinfo == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "getHostInfo: null, return false");
			return false;
		}
		return true;
	}
	
	private void populateLayout() {
		
		if(!getPrefs() || appPrefs == null || !getHostInfo()) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "populateLayout returns, data is not present");
			setLayoutLoading();
			return;
		}
		
		// setup layout
		setContentView(R.layout.prefs_layout);
		lv = (ListView) findViewById(R.id.listview);
        listAdapter = new PrefsListAdapter(PrefsActivity.this,R.id.listview,data);
        lv.setAdapter(listAdapter);
		
		data.clear();
		
		Boolean advanced = appPrefs.getShowAdvanced();

		// general
    	data.add(new PrefsListItemWrapper(this,R.string.prefs_category_general,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_autostart_header,R.string.prefs_category_general,appPrefs.getAutostart()));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_show_notification_header,R.string.prefs_category_general,appPrefs.getShowNotification())); 
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_show_advanced_header,R.string.prefs_category_general,appPrefs.getShowAdvanced()));
		// network
    	data.add(new PrefsListItemWrapper(this,R.string.prefs_category_network,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_network_wifi_only_header,R.string.prefs_category_network,clientPrefs.network_wifi_only));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_network_daily_xfer_limit_mb_header,R.string.prefs_category_network,clientPrefs.daily_xfer_limit_mb));
    	// power
		data.add(new PrefsListItemWrapper(this,R.string.prefs_category_power,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_run_on_battery_header,R.string.prefs_category_power,clientPrefs.run_on_batteries));
		data.add(new PrefsListItemWrapperValue(this,R.string.battery_charge_min_pct_header,R.string.prefs_category_power,clientPrefs.battery_charge_min_pct));
		// cpu
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_cpu,true));
		if(advanced && hostinfo.p_ncpus > 1) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_cpu_number_cpus_header,R.string.prefs_category_cpu,pctCpuCoresToNumber(clientPrefs.max_ncpus_pct)));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_cpu_time_max_header,R.string.prefs_category_cpu,clientPrefs.cpu_usage_limit));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_cpu_other_load_suspension_header,R.string.prefs_category_cpu,clientPrefs.suspend_cpu_usage));
		// storage
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_storage,true));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_disk_max_pct_header,R.string.prefs_category_storage,clientPrefs.disk_max_used_pct));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_disk_min_free_gb_header,R.string.prefs_category_storage,clientPrefs.disk_min_free_gb));
		// memory
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_memory,true));
		if(advanced) data.add(new PrefsListItemWrapperValue(this,R.string.prefs_memory_max_idle_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_idle_frac));
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
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateBoolPref for ID: " + ID + " value: " + newValue);
		for (PrefsListItemWrapper item: data) {
			if(item.ID == ID){
				((PrefsListItemWrapperValue) item).status = newValue;
				continue;
			}
		}
	}
	
	private void setLayoutLoading() {
		if(Logging.DEBUG) Log.d(Logging.TAG,"setLayoutLoading()");
        setContentView(R.layout.generic_layout_loading);
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.prefs_loading);
	}
	
	// onClick of listview items with PrefsListItemBool
	public void onCbClick (View view) {
		if(Logging.DEBUG) Log.d(Logging.TAG,"onCbClick");
		Integer ID = (Integer) view.getTag();
		CheckBox source = (CheckBox) view;
		Boolean isSet = source.isChecked();
		
		switch (ID) {
		case R.string.prefs_autostart_header: //app pref
			appPrefs.setAutostart(isSet);
			updateBoolPref(ID, isSet);
			updateLayout();
			break;
		case R.string.prefs_show_notification_header: //app pref
			appPrefs.setShowNotification(isSet);
			ClientNotification.getInstance(getApplicationContext()).update(); // update notification
			updateBoolPref(ID, isSet);
			updateLayout();
			break;
		case R.string.prefs_show_advanced_header: //app pref
			appPrefs.setShowAdvanced(isSet);
			// reload complete layout to remove/add advanced elements
			populateLayout();
			break;
		case R.string.prefs_run_on_battery_header: //client pref
			clientPrefs.run_on_batteries = isSet;
			updateBoolPref(ID, isSet);
			new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
			break;
		case R.string.prefs_network_wifi_only_header: //client pref
			clientPrefs.network_wifi_only = isSet;
			updateBoolPref(ID, isSet);
			new WriteClientPrefsAsync().execute(clientPrefs); //async task triggers layout update
			break;
		}
	}
	
	// onClick of listview items with PrefsListItemWrapperValue
	public void onItemClick (View view) {
			final PrefsListItemWrapperValue listItem = (PrefsListItemWrapperValue) view.getTag();
			if(Logging.DEBUG) Log.d(Logging.TAG,"PrefsActivity onItemClick Value " + listItem.ID);
			
			final Dialog dialog = new Dialog(this);
			dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
			if(listItem.isPct) {
				// show dialog with slider
				dialog.setContentView(R.layout.prefs_layout_dialog_pct);
				// setup slider
				TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
				sliderProgress.setText(listItem.status.intValue() + " " + getString(R.string.prefs_unit_pct));
				Double seekBarDefault = listItem.status / 10;
				SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
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
			} else if(listItem.isNumber) { 
				if(!getHostInfo()) {
					if(Logging.WARNING) Log.w(Logging.TAG, "onItemClick missing hostInfo");
					return;
				}
				
				// show dialog with slider
				dialog.setContentView(R.layout.prefs_layout_dialog_pct);
				TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
				sliderProgress.setText(""+listItem.status.intValue());
				SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
				slider.setMax(hostinfo.p_ncpus);
				slider.setProgress(listItem.status.intValue());
				slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
					public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
						if(progress == 0) progress = 1;
						String progressString = String.valueOf(progress);
						TextView sliderProgress = (TextView) dialog.findViewById(R.id.seekbar_status);
						sliderProgress.setText(progressString);
					}
					@Override
					public void onStartTrackingTouch(SeekBar seekBar) {}
					@Override
					public void onStopTrackingTouch(SeekBar seekBar) {}
				});
			} else {
				// show dialog with edit text
				dialog.setContentView(R.layout.prefs_layout_dialog);
			}
			// show preference name
			((TextView)dialog.findViewById(R.id.pref)).setText(listItem.ID);
			
			// setup buttons
			Button confirm = (Button) dialog.findViewById(R.id.confirm);
			confirm.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
		         	   double value;
		         	   if(listItem.isPct) {
		         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
		         		   value = slider.getProgress()*10;
		         	   } else if(listItem.isNumber) {
		         		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
		         		   int sbProgress = slider.getProgress();
		         		   if(sbProgress == 0) sbProgress = 1;
		         		   value = numberCpuCoresToPct(sbProgress);
		         	   } else {
		         		   EditText edit = (EditText) dialog.findViewById(R.id.Input);
		         		   String input = edit.getText().toString();
		         		   Double valueTmp = parseInputValueToDouble(input);
		         		   if(valueTmp == null) return;
		         		   value = valueTmp;
		         	   }
		         	   writeValuePreference(listItem.ID, value);
		         	   dialog.dismiss();
				}
			});
			Button cancel = (Button) dialog.findViewById(R.id.cancel);
			cancel.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					dialog.dismiss();
				}
			});
			
			dialog.show();
	}

	@Override
	protected void onDestroy() {
	    if(Logging.VERBOSE) Log.v(Logging.TAG,"PrefsActivity onDestroy()");
	    super.onDestroy();
	    doUnbindService();
	}
	
	private void writeValuePreference(int id, double value) {
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
			Toast toast = Toast.makeText(getApplicationContext(), "ooops! something went wrong...", Toast.LENGTH_SHORT);
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
			Toast toast = Toast.makeText(getApplicationContext(), "wrong format!", Toast.LENGTH_SHORT);
			toast.show();
			return null;
		}
	}
	
	private final class WriteClientPrefsAsync extends AsyncTask<GlobalPreferences,Void,Boolean> {
		@Override
		protected Boolean doInBackground(GlobalPreferences... params) {
			if(mIsBound) return monitor.setGlobalPreferences(params[0]);
			else return false;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			updateLayout();
		}
	}
}
