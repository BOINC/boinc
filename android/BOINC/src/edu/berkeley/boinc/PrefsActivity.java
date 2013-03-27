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
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class PrefsActivity extends Activity implements OnClickListener {
	
	private final String TAG = "BOINC PrefsActivity";
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private ListView lv;
	private PrefsListAdapter listAdapter;
	
	private ArrayList<PrefsListItemWrapper> data = new ArrayList<PrefsListItemWrapper>(); //Adapter for list data
	private GlobalPreferences clientPrefs = null; //preferences of the client, read on every onResume via RPC
	private AppPreferences appPrefs = null; //Android specific preferences, singleton of monitor
	
	private Dialog dialog; //Dialog for input on non-Bool preferences
	
	private Boolean dataOutdated = true;
	
	/*
	 * Receiver is necessary, because writing of prefs has to be done asynchroneously. PrefsActivity will change to "loading" layout, until monitor read new results.
	 */
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			//Log.d(TAG+"-localClientStatusRecNoisy","received");
			if(dataOutdated) loadSettings(); //otherwise view gets refreshed on every broadcast.
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		doBindService();
	}
	
	public void onResume() {
		super.onResume();
		//gets called every time Activity comes to front, therefore also after onCreate
		
		//register receiver of client status
		registerReceiver(mClientStatusChangeRec,ifcsc);
		
		loadSettings();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 * currently in Prefs- and DebugActivity 
	 * 
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
			appPrefs = Monitor.getAppPrefs();
			Log.d(TAG, "appPrefs available");
			loadSettings();
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
	
	private Boolean readPrefs() {
		clientPrefs = Monitor.getClientStatus().getPrefs(); //read prefs from client via rpc
		if(clientPrefs == null) {
			Log.d(TAG, "readPrefs: null, return false");
			return false;
		}
		//Log.d(TAG, "readPrefs done");
		return true;
	}
	
	private void loadSettings() {
		
		if(!readPrefs() || appPrefs == null) {
			Log.d(TAG, "loadSettings returns, data is not present");
			setDataOutdated();
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
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_memory_max_busy_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_busy_frac));
		if(advanced) data.add(new PrefsListItemWrapperDouble(this,R.string.prefs_memory_max_idle_header,R.string.prefs_category_memory,clientPrefs.ram_max_used_idle_frac));

		dataOutdated = false;
	}
	
	private void setDataOutdated() {
        setContentView(R.layout.generic_layout_loading);
        TextView loadingHeader = (TextView)findViewById(R.id.loading_header);
        loadingHeader.setText(R.string.prefs_loading);
		dataOutdated = true;
	}
	
	/*
	 * Gets triggered by change of checkboxes. (Boolean prefs)
	 */
	public void onCbClick (View view) {
		Log.d(TAG,"onCbClick");
		Integer ID = (Integer) view.getTag();
		CheckBox source = (CheckBox) view;
		Boolean isSet = source.isChecked();
		
		switch (ID) {
		case R.string.prefs_autostart_header: //app pref
			appPrefs.setAutostart(isSet);
			break;
		case R.string.prefs_show_advanced_header: //app pref
			appPrefs.setShowAdvanced(isSet);
			loadSettings(); // force reload of list view;
			break;
		case R.string.prefs_run_on_battery_header: //client pref
			clientPrefs.run_on_batteries = isSet;
			monitor.setPrefs(clientPrefs);
			break;
		case R.string.prefs_network_wifi_only_header: //client pref
			clientPrefs.network_wifi_only = isSet;
			monitor.setPrefs(clientPrefs);
			break;
		}
	}
	
	public void onItemClick (View view) {
		Integer ID = (Integer) view.getTag();
		Log.d(TAG,"onItemClick " + ID);
		showDialog(ID);
	}
	
	/*
	 * Gets called when showDialog is triggered
	 */
	@Override
	protected Dialog onCreateDialog(int id) {
		dialog = new Dialog(this); //instance new dialog
		dialog.setContentView(R.layout.prefs_layout_dialog);
		Button button = (Button) dialog.findViewById(R.id.buttonPrefSubmit);
		button.setOnClickListener(this);
		//EditText edit = (EditText) dialog.findViewById(R.id.Input);
		TextView description = (TextView) dialog.findViewById(R.id.description);
		description.setText(id);
		dialog.setTitle(R.string.prefs_dialog_title);
		button.setId(id); //set input id, for evaluation in onClick
		return dialog;
	}

	@Override
	protected void onDestroy() {
	    Log.d(TAG,"onDestroy()");
	    super.onDestroy();
	    doUnbindService();
	}

	/*
	 * Gets called when Dialog's confirm button is clicked
	 */
	@Override
	public void onClick(View v) {
		Log.d(TAG,"dialogDismiss");
		Button button = (Button) v;
		Integer id = button.getId();
		EditText input = (EditText) dialog.findViewById(R.id.Input);
		
		// parse value
		Double value = 0.0;
		try {
			String tmp = input.getText().toString();
			tmp=tmp.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
			value = Double.parseDouble(tmp);
			Log.d(TAG,"onClick with input value " + value);
		} catch (Exception e) {
			Log.w(TAG, e);
			Toast toast = Toast.makeText(getApplicationContext(), "wrong format!", Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		
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
		case R.string.prefs_memory_max_busy_header:
			clientPrefs.ram_max_used_busy_frac = value;
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
		monitor.setPrefs(clientPrefs);
		setDataOutdated();
		dialog.dismiss();
	}
	


}
