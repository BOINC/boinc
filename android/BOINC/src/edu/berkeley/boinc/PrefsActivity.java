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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
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
	
	private Dialog dialog; //Dialog for input on non-Bool preferences
	private PrefsListItemWrapperDouble dialogItem; // saves content of preference Dialog is showing
	
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

    	data.add(new PrefsListItemWrapper(this,R.string.prefs_category_general,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_autostart_header,R.string.prefs_category_general,appPrefs.getAutostart()));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_show_notification_header,R.string.prefs_category_general,appPrefs.getShowNotification())); 
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_show_advanced_header,R.string.prefs_category_general,appPrefs.getShowAdvanced()));
    	data.add(new PrefsListItemWrapper(this,R.string.prefs_category_network,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_network_wifi_only_header,R.string.prefs_category_network,clientPrefs.network_wifi_only));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_network_daily_xfer_limit_mb_header,R.string.prefs_category_network,clientPrefs.daily_xfer_limit_mb));
    	data.add(new PrefsListItemWrapper(this,R.string.prefs_category_power,true));
		data.add(new PrefsListItemWrapperBool(this,R.string.prefs_run_on_battery_header,R.string.prefs_category_power,clientPrefs.run_on_batteries));
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_cpu,true));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_number_cpus_header,R.string.prefs_category_cpu,clientPrefs.max_ncpus_pct));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_time_max_header,R.string.prefs_category_cpu,clientPrefs.cpu_usage_limit));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_cpu_other_load_suspension_header,R.string.prefs_category_cpu,clientPrefs.suspend_cpu_usage));
		if(advanced) data.add(new PrefsListItemWrapper(this,R.string.prefs_category_storage,true));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_disk_max_pct_header,R.string.prefs_category_storage,clientPrefs.disk_max_used_pct));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_disk_min_free_gb_header,R.string.prefs_category_storage,clientPrefs.disk_min_free_gb));
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
			populateLayout(); // updates status text
			ClientNotification.getInstance().enable(getApplicationContext(), isSet);
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
		PrefsListItemWrapperDouble listItem = (PrefsListItemWrapperDouble) view.getTag();
		Log.d(TAG,"onItemClick " + listItem.ID);

		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		LayoutInflater inflater = getLayoutInflater();
		final View dialogContent;
		if(listItem.isPct) {
			dialogContent = inflater.inflate(R.layout.prefs_layout_dialog_pct, null);
			TextView sliderProgress = (TextView) dialogContent.findViewById(R.id.seekbar_status);
			sliderProgress.setText(listItem.status.intValue() + " %");
			SeekBar slider = (SeekBar) dialogContent.findViewById(R.id.seekbar);
			slider.setProgress(listItem.status.intValue());
			slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
		        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
		        	String progressString = progress + " %";
		        	TextView sliderProgress = (TextView) dialogContent.findViewById(R.id.seekbar_status);
		            sliderProgress.setText(progressString);
		        }
				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {}
				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {}
		    });
		} else {
			dialogContent = inflater.inflate(R.layout.prefs_layout_dialog, null);
		}
        builder.setMessage(listItem.ID)
        	   .setView(dialogContent)
               .setNegativeButton(R.string.prefs_cancel_button, new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialogI, int id) {
                       dialog.cancel();
                   }
               })
               .setPositiveButton(R.string.prefs_submit_button, new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialogI, int id) {
                	   double value;
                	   if(dialogItem.isPct) {
                		   SeekBar slider = (SeekBar) dialog.findViewById(R.id.seekbar);
                		   value = slider.getProgress();
                	   } else {
                		   EditText edit = (EditText) dialog.findViewById(R.id.Input);
                		   String input = edit.getText().toString();
                		   Double valueTmp = parseInputValueToDouble(input);
                		   if(valueTmp == null) return;
                		   value = valueTmp;
                	   }
                	   writeDoublePreference(dialogItem.ID, value);
                   }
               });
        dialog = builder.create();
        dialog.show();
        dialogItem = listItem; // set dialog content
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
		
		// preferences adapted, dismiss dialog and write preferences to client
		dialog.dismiss();
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
			setLayoutLoading();
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
