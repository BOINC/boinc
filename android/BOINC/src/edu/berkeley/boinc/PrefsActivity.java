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

import java.util.ArrayList;
import edu.berkeley.boinc.adapter.PrefsListAdapter;
import edu.berkeley.boinc.adapter.PrefsListItemWrapper;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperBool;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperDouble;
import edu.berkeley.boinc.client.ClientNotification;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.GlobalPreferences;
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
	
	private final String TAG = "BOINC PrefsActivity";
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private ListView lv;
	private PrefsListAdapter listAdapter;
	
	private ArrayList<PrefsListItemWrapper> data = new ArrayList<PrefsListItemWrapper>(); //Adapter for list data
	private GlobalPreferences clientPrefs = null; //preferences of the client, read on every onResume via RPC
	private AppPreferences appPrefs = null; //Android specific preferences, singleton of monitor
	
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
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
			appPrefs = Monitor.getAppPrefs();
			Log.d(TAG, "appPrefs available");
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
			Log.d(TAG, "readPrefs: null, return false");
			return false;
		}
		//Log.d(TAG, "readPrefs done");
		return true;
	}
	
	private void populateLayout() {
		
		if(!getPrefs() || appPrefs == null) {
			Log.d(TAG, "populateLayout returns, data is not present");
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
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_network_daily_xfer_limit_mb_header,R.string.prefs_category_network,clientPrefs.daily_xfer_limit_mb));
    	// power
		data.add(new PrefsListItemWrapper(this,R.string.prefs_category_power,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_run_on_battery_header,R.string.prefs_category_power,clientPrefs.run_on_batteries));
		data.add(new PrefsListItemWrapperDouble(this,R.string.battery_charge_min_pct_header,R.string.prefs_category_power,clientPrefs.battery_charge_min_pct));
		// cpu
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_cpu,true));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_number_cpus_header,R.string.prefs_category_cpu,clientPrefs.max_ncpus_pct));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_time_max_header,R.string.prefs_category_cpu,clientPrefs.cpu_usage_limit));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_other_load_suspension_header,R.string.prefs_category_cpu,clientPrefs.suspend_cpu_usage));
		// storage
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_storage,true));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_disk_max_pct_header,R.string.prefs_category_storage,clientPrefs.disk_max_used_pct));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_disk_min_free_gb_header,R.string.prefs_category_storage,clientPrefs.disk_min_free_gb));
		// memory
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_memory,true));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_memory_max_idle_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_idle_frac));
	}
	
	private void setLayoutLoading() {
		Log.d(TAG,"setLayoutLoading()");
        setContentView(R.layout.generic_layout_loading);
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.prefs_loading);
	}
	
	// onClick of listview items with PrefsListItemBool
	public void onCbClick (View view) {
		Log.d(TAG,"onCbClick");
		Integer ID = (Integer) view.getTag();
		CheckBox source = (CheckBox) view;
		Boolean isSet = source.isChecked();
		
		switch (ID) {
		case R.string.prefs_autostart_header: //app pref
			appPrefs.setAutostart(isSet);
			populateLayout(); // updates status text
			break;
		case R.string.prefs_show_notification_header: //app pref
			appPrefs.setShowNotification(isSet);
			ClientNotification.getInstance(getApplicationContext()).update(); // update notification
			populateLayout(); // updates status text
			break;
		case R.string.prefs_show_advanced_header: //app pref
			appPrefs.setShowAdvanced(isSet);
			 // call reload of list directly, whithout detour via setDataOutdated and waiting for event.
			populateLayout();
			break;
		case R.string.prefs_run_on_battery_header: //client pref
			clientPrefs.run_on_batteries = isSet;
			new WriteClientPrefsAsync().execute(clientPrefs);
			break;
		case R.string.prefs_network_wifi_only_header: //client pref
			clientPrefs.network_wifi_only = isSet;
			new WriteClientPrefsAsync().execute(clientPrefs);
			break;
		}
	}
	
	// onClick of listview items with PrefsListItemWrapperDouble
	public void onItemClick (View view) {
		final PrefsListItemWrapperDouble listItem = (PrefsListItemWrapperDouble) view.getTag();
		Log.d(TAG,"onItemClick " + listItem.ID);
		
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
	         	   } else {
	         		   EditText edit = (EditText) dialog.findViewById(R.id.Input);
	         		   String input = edit.getText().toString();
	         		   Double valueTmp = parseInputValueToDouble(input);
	         		   if(valueTmp == null) return;
	         		   value = valueTmp;
	         	   }
	         	   writeDoublePreference(listItem.ID, value);
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
	    Log.d(TAG,"onDestroy()");
	    super.onDestroy();
	    doUnbindService();
	}
	
	private void writeDoublePreference(int id, double value) {
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
			Log.d(TAG,"onClick (dialog submit button), couldnt match ID");
			Toast toast = Toast.makeText(getApplicationContext(), "ooops! something went wrong...", Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		// preferences adapted, write preferences to client
		new WriteClientPrefsAsync().execute(clientPrefs);
	}

	public Double parseInputValueToDouble(String input) {
		// parse value
		Double value = 0.0;
		try {
			input=input.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
			value = Double.parseDouble(input);
			Log.d(TAG,"parseInputValueToDouble: " + value);
			return value;
		} catch (Exception e) {
			Log.w(TAG, e);
			Toast toast = Toast.makeText(getApplicationContext(), "wrong format!", Toast.LENGTH_SHORT);
			toast.show();
			return null;
		}
	}
	
	private final class WriteClientPrefsAsync extends AsyncTask<GlobalPreferences,Void,Boolean> {

		@Override
		protected void onPreExecute() {
			//setLayoutLoading();
			super.onPreExecute();
		}

		@Override
		protected Boolean doInBackground(GlobalPreferences... params) {
			if(mIsBound) return monitor.setGlobalPreferences(params[0]);
			else return false;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			populateLayout();
		}
	}

}
