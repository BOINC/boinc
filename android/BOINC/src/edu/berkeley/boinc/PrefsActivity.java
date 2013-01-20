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
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
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
	
	private Boolean dataOutdated = false; //shows that current data is out of date. this happens when writing client status async just started and has not returned (clientstatechanged broadcast) yet.
	private Boolean initialSetup = true; //prevents reinitPrefsLayout() from adding elements to the list, every time it gets called. also prevent loadLayout from reading elements out of bounds (array not set up, if this is not false)
	
	/*
	 * Receiver is necessary, because writing of prefs has to be done asynchroneously. PrefsActivity will change to "loading" layout, until monitor read new results.
	 */
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG+"-localClientStatusRecNoisy","received");
			if(dataOutdated) { //cause activity to re-init layout
				Log.d(TAG, "data was outdated, go directly to reinitPrefsLayout");
				dataOutdated = false;
				reinitPrefsLayout();
			}
			loadSettings();
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
		
		//determine layout
		if(initialSetup) { //no data available (first call) 
			setContentView(R.layout.prefs_layout_loading);
		}
		
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
		Log.d(TAG, "readPrefs done");
		return true;
	}
	
	private void loadSettings() {
		if(!readPrefs() || appPrefs == null) {
			Log.d(TAG, "loadSettings returns, data is not present");
			return;
		}
		
		if(initialSetup) {
			//init layout instead
			reinitPrefsLayout();
		} else {
			Log.d(TAG, "loadSettings outdated: " + dataOutdated );
			if(dataOutdated) { //data is not present or not current, show loading instead!
				setContentView(R.layout.prefs_layout_loading);
			} else {
				//((PrefsListItemWrapperText)data.get(0)).status = appPrefs.getEmail();
				//((PrefsListItemWrapperText)data.get(1)).status = appPrefs.getPwd();
				((PrefsListItemWrapperBool)data.get(0)).setStatus(appPrefs.getAutostart());
				((PrefsListItemWrapperBool)data.get(1)).setStatus(clientPrefs.run_on_batteries);
				((PrefsListItemWrapperBool)data.get(2)).setStatus(clientPrefs.network_wifi_only); 
				((PrefsListItemWrapperDouble)data.get(3)).status = clientPrefs.disk_max_used_pct;
				((PrefsListItemWrapperDouble)data.get(4)).status = clientPrefs.disk_min_free_gb;
				((PrefsListItemWrapperDouble)data.get(5)).status = clientPrefs.daily_xfer_limit_mb;
				
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		}
	}
	
	private void reinitPrefsLayout() {
		
		setContentView(R.layout.prefs_layout);
		lv = (ListView) findViewById(R.id.listview);
        listAdapter = new PrefsListAdapter(PrefsActivity.this,R.id.listview,data);
        lv.setAdapter(listAdapter);

        if(initialSetup) { //prevent from re-population when reinit is called after dataOutdated
			//parse app prefs
			//data.add(0, new PrefsListItemWrapperText(this,R.string.prefs_project_email_header,appPrefs.getEmail()));
			//data.add(1, new PrefsListItemWrapperText(this,R.string.prefs_project_pwd_header,appPrefs.getPwd()));
			data.add(0, new PrefsListItemWrapperBool(this,R.string.prefs_autostart_header,appPrefs.getAutostart())); 
			//parse client prefs
			data.add(1, new PrefsListItemWrapperBool(this,R.string.prefs_run_on_battery_header,clientPrefs.run_on_batteries));
			data.add(2, new PrefsListItemWrapperBool(this,R.string.prefs_network_wifi_only_header,clientPrefs.network_wifi_only));
			data.add(3, new PrefsListItemWrapperDouble(this,R.string.prefs_disk_max_pct_header,clientPrefs.disk_max_used_pct));
			data.add(4, new PrefsListItemWrapperDouble(this,R.string.prefs_disk_min_free_gb_header,clientPrefs.disk_min_free_gb));
			data.add(5, new PrefsListItemWrapperDouble(this,R.string.prefs_daily_xfer_limit_mb_header,clientPrefs.daily_xfer_limit_mb));
			
        	initialSetup = false;
        }
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
		case R.string.prefs_run_on_battery_header: //client pref
			clientPrefs.run_on_batteries = isSet;
			monitor.setPrefs(clientPrefs);
			dataOutdated = true; //async write of client prefs started, data out dated until broadcast
			break;
		case R.string.prefs_network_wifi_only_header: //client pref
			clientPrefs.network_wifi_only = isSet;
			monitor.setPrefs(clientPrefs);
			dataOutdated = true; //async write of client prefs started, data out dated until broadcast
			break;
		}
		loadSettings();
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
		String title = "Enter new ";
		Button button = (Button) dialog.findViewById(R.id.buttonPrefSubmit);
		button.setOnClickListener(this);
		EditText edit = (EditText) dialog.findViewById(R.id.Input);
		//customize:
		switch (id) {/*
		case R.string.prefs_project_email_header:
			title += "eMail address";
			edit.setInputType(InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS);
			break;
		case R.string.prefs_project_pwd_header:
			title += "password";
			edit.setTransformationMethod(PasswordTransformationMethod.getInstance());
			button.setText("Login!");
			break;*/
		case R.string.prefs_disk_max_pct_header:
			title += "disk space limit (%)";
			edit.setInputType(InputType.TYPE_NUMBER_FLAG_DECIMAL);
			break;
		case R.string.prefs_disk_min_free_gb_header:
			title += "free disk space (GB)";
			edit.setInputType(InputType.TYPE_NUMBER_FLAG_DECIMAL);
			break;
		case R.string.prefs_daily_xfer_limit_mb_header:
			title += "transfer limit (MB)";
			edit.setInputType(InputType.TYPE_NUMBER_FLAG_DECIMAL);
			break;
		default:
			Log.d(TAG,"onCreateDialog, couldnt match ID");
			break;
			
		}
		dialog.setTitle(title + ":");
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
		String tmp = input.getText().toString();
		Log.d(TAG,"onClick with input " + tmp);
		try {
			switch (id) {
			case R.string.prefs_disk_max_pct_header:
				tmp=tmp.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
				clientPrefs.disk_max_used_pct = Double.parseDouble(tmp);
				monitor.setPrefs(clientPrefs);
				dataOutdated = true; //async write of client prefs started, data out dated until broadcast
				break;
			case R.string.prefs_disk_min_free_gb_header:
				tmp=tmp.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
				clientPrefs.disk_max_used_gb = Double.parseDouble(tmp);
				monitor.setPrefs(clientPrefs);
				dataOutdated = true; //async write of client prefs started, data out dated until broadcast
				break;
			case R.string.prefs_daily_xfer_limit_mb_header:
				tmp=tmp.replaceAll(",","."); //replace e.g. European decimal seperator "," by "."
				clientPrefs.daily_xfer_limit_mb = Double.parseDouble(tmp);
				monitor.setPrefs(clientPrefs);
				dataOutdated = true; //async write of client prefs started, data out dated until broadcast
				break;
			default:
				Log.d(TAG,"onClick (dialog submit button), couldnt match ID");
				break;
			
			}
			dialog.dismiss();
			loadSettings();
		} catch (Exception e) { //e.g. when parsing fails
			Log.e(TAG, "Exception in dialog onClick", e);
			Toast toast = Toast.makeText(getApplicationContext(), "wrong format!", Toast.LENGTH_SHORT);
			toast.show();
		}
	}
	


}
