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

import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.definitions.CommonDefs;
import android.app.Activity;
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
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class StatusActivity extends Activity {
	
	private final String TAG = "BOINC StatusActivity";
	
	private Monitor monitor;
	
	private Boolean mIsBound = false;

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			String action = intent.getAction();
			Log.d(TAG+"-localClientStatusRecNoisy","received action " + action);
			loadLayout(); //initial layout set up
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	

	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG, "onServiceConnected");

	    	monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    loadLayout();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	Log.d(TAG, "onServiceDisconnected");

	    	monitor = null;
	        mIsBound = false;
	    }
	};

	void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
		}
	}

	void doUnbindService() {
	    if (mIsBound) {
	        getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	public void onCreate(Bundle savedInstanceState) {
		//bind to monitor in order to call its functions and access ClientStatus singleton
		doBindService();
		super.onCreate(savedInstanceState);
	}
	
	public void onResume() {
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		Log.d(TAG+"-onResume","register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		loadLayout();
		super.onResume();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}
	
	private void loadLayout() {
		//load layout, if service is available and ClientStatus can be accessed.
		//if this is not the case, "onServiceConnected" will call "loadLayout" as soon as the service is bound
		if(mIsBound) {
			ClientStatus status = Monitor.getClientStatus();
			setContentView(R.layout.status_layout);
			// get views
			RelativeLayout statusWrapper = (RelativeLayout) findViewById(R.id.status_wrapper);
			TextView statusHeader = (TextView) findViewById(R.id.status_header);
			ImageView statusImage = (ImageView) findViewById(R.id.status_image);
			TextView statusDescriptor = (TextView) findViewById(R.id.status_long);
			ImageView changeRunmodeImage = (ImageView) findViewById(R.id.status_change_runmode_image);
			TextView changeRunmodeDescriptor = (TextView) findViewById(R.id.status_change_runmode_long);
			
			if(status.setupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) { 
				statusWrapper.setVisibility(View.VISIBLE);
				switch(status.computingStatus) {
				case ClientStatus.COMPUTING_STATUS_NEVER:
					statusHeader.setText(R.string.status_computing_disabled);
					statusImage.setImageResource(R.drawable.stopw48);
					statusImage.setContentDescription(getString(R.string.status_computing_disabled_long));
					statusDescriptor.setText("");
					changeRunmodeImage.setImageResource(R.drawable.playw24);
					changeRunmodeImage.setContentDescription(getString(R.string.enable_computation));
					changeRunmodeImage.setTag(true);
					changeRunmodeDescriptor.setText(R.string.enable_computation);
					changeRunmodeDescriptor.setTag(true);
					break;
				case ClientStatus.COMPUTING_STATUS_SUSPENDED:
					statusHeader.setText(R.string.status_paused);
					statusImage.setImageResource(R.drawable.pausew48);
					statusImage.setContentDescription(getString(R.string.status_paused));
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
					switch(status.computingSuspendReason) {
					case CommonDefs.SUSPEND_REASON_BATTERIES:
						statusDescriptor.setText(R.string.suspend_batteries);
						break;
					case CommonDefs.SUSPEND_REASON_USER_ACTIVE:
						statusDescriptor.setText(R.string.suspend_useractive);
						break;
					case CommonDefs.SUSPEND_REASON_USER_REQ:
						statusDescriptor.setText(R.string.suspend_userreq);
						break;
					case CommonDefs.SUSPEND_REASON_TIME_OF_DAY:
						statusDescriptor.setText(R.string.suspend_tod);
						break;
					case CommonDefs.SUSPEND_REASON_BENCHMARKS:
						statusDescriptor.setText(R.string.suspend_bm);
						break;
					case CommonDefs.SUSPEND_REASON_DISK_SIZE:
						statusDescriptor.setText(R.string.suspend_disksize);
						break;
					case CommonDefs.SUSPEND_REASON_CPU_THROTTLE:
						statusDescriptor.setText(R.string.suspend_cputhrottle);
						break;
					case CommonDefs.SUSPEND_REASON_NO_RECENT_INPUT:
						statusDescriptor.setText(R.string.suspend_noinput);
						break;
					case CommonDefs.SUSPEND_REASON_INITIAL_DELAY:
						statusDescriptor.setText(R.string.suspend_delay);
						break;
					case CommonDefs.SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
						statusDescriptor.setText(R.string.suspend_exclusiveapp);
						break;
					case CommonDefs.SUSPEND_REASON_CPU_USAGE:
						statusDescriptor.setText(R.string.suspend_cpu);
						break;
					case CommonDefs.SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED:
						statusDescriptor.setText(R.string.suspend_network_quota);
						break;
					case CommonDefs.SUSPEND_REASON_OS:
						statusDescriptor.setText(R.string.suspend_os);
						break;
					case CommonDefs.SUSPEND_REASON_WIFI_STATE:
						statusDescriptor.setText(R.string.suspend_wifi);
						break;
					case CommonDefs.SUSPEND_REASON_BATTERY_CHARGING:
						statusDescriptor.setText(R.string.suspend_battery_charging);
						break;
					case CommonDefs.SUSPEND_REASON_BATTERY_OVERHEATED:
						statusDescriptor.setText(R.string.suspend_battery_overheating);
						break;
					default:
						statusDescriptor.setText(R.string.suspend_unknown);
						break;
					}
					break;
				case ClientStatus.COMPUTING_STATUS_IDLE: 
					statusHeader.setText(R.string.status_idle);
					statusImage.setImageResource(R.drawable.pausew48);
					statusImage.setContentDescription(getString(R.string.status_idle));
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
					Integer networkState = 0;
					try{
						networkState = status.networkSuspendReason;
					} catch (Exception e) {}
					if(networkState==8192){ //network suspended due to wifi state
						statusDescriptor.setText(R.string.suspend_wifi);
					}else {
						statusDescriptor.setText(R.string.status_idle_long);
					}
					break;
				case ClientStatus.COMPUTING_STATUS_COMPUTING:
					statusHeader.setText(R.string.status_running);
					statusImage.setImageResource(R.drawable.playw48);
					statusImage.setContentDescription(getString(R.string.status_running));
					statusDescriptor.setText(R.string.status_running_long);
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
					break;
				}
			} else { // BOINC client is not available, disable layout
				statusWrapper.setVisibility(View.GONE);
			}
		}
	}
	
	public void onClick (View v) {
		Log.d(TAG,"onClick");
		if(!mIsBound) {Log.w(TAG,"not bound");return;}
		try {
			Boolean enable = (Boolean) v.getTag();
			if(enable) {
				monitor.setRunMode(CommonDefs.RUN_MODE_AUTO);
			} else {
				monitor.setRunMode(CommonDefs.RUN_MODE_NEVER);
			}
		} catch (Exception e) {Log.e(TAG, "could not map status tag", e);}
	}
}
