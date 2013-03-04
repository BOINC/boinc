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
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class StatusActivity extends Activity {
	
	private final String TAG = "StatusActivity";
	
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
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    loadLayout();
	    }

	    public void onServiceDisconnected(ComponentName className) {
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
			switch(status.setupStatus){
			case 0: //launching
				setContentView(R.layout.status_layout_launching);
				break;
			case 1: 
				switch(status.computingStatus) {
				case 0: //suspended by user
					setContentView(R.layout.status_layout_computing_disabled);
					findViewById(R.id.enableImage).setOnClickListener(mEnableClickListener);
					findViewById(R.id.enableText).setOnClickListener(mEnableClickListener);
					break;
				case 1: //suspended because of preference
					setContentView(R.layout.status_layout_suspended);
					findViewById(R.id.disableImage).setOnClickListener(mDisableClickListener);
					findViewById(R.id.disableText).setOnClickListener(mDisableClickListener);
					TextView t=(TextView)findViewById(R.id.suspend_reason);
					switch(status.computingSuspendReason) {
					case 1:
						t.setText(R.string.suspend_batteries);
						break;
					case 2:
						t.setText(R.string.suspend_useractive);
						break;
					case 4:
						t.setText(R.string.suspend_userreq);
						break;
					case 8:
						t.setText(R.string.suspend_tod);
						break;
					case 16:
						t.setText(R.string.suspend_bm);
						break;
					case 32:
						t.setText(R.string.suspend_disksize);
						break;
					case 64:
						t.setText(R.string.suspend_cputhrottle);
						break;
					case 128:
						t.setText(R.string.suspend_noinput);
						break;
					case 256:
						t.setText(R.string.suspend_delay);
						break;
					case 512:
						t.setText(R.string.suspend_exclusiveapp);
						break;
					case 1024:
						t.setText(R.string.suspend_cpu);
						break;
					case 2048:
						t.setText(R.string.suspend_network_quota);
						break;
					case 4096:
						t.setText(R.string.suspend_os);
						break;
					case 8192:
						//pointless??! wifi causes network suspension, does not influence computing
						t.setText(R.string.suspend_wifi);
						break;
					default:
						t.setText(R.string.suspend_unknown);
						break;
					}
					break;
				case 2: //idle
					setContentView(R.layout.status_layout_suspended);
					TextView t2=(TextView)findViewById(R.id.suspend_reason);
					findViewById(R.id.disableImage).setOnClickListener(mDisableClickListener);
					findViewById(R.id.disableText).setOnClickListener(mDisableClickListener);
					Integer networkState = 0;
					try{
						networkState = status.networkSuspendReason;
					} catch (Exception e) {}
					if(networkState==8192){ //network suspended due to wifi state
						t2.setText(R.string.suspend_wifi);
					}else {
						t2.setText(R.string.suspend_idle);
					}
					break;
				case 3: // computing
					setContentView(R.layout.status_layout_computing);
					findViewById(R.id.disableImage).setOnClickListener(mDisableClickListener);
					findViewById(R.id.disableText).setOnClickListener(mDisableClickListener);
					break;
				}
				break;
			case 2:
				setContentView(R.layout.status_layout_error);
				Log.d(TAG,"layout: status_layout_error");
				break;
			case 3:
				setContentView(R.layout.status_layout_noproject);
				Log.d(TAG,"layout: status_layout_noproject");
				break;
			}
		}
	}
	
	//gets called when user clicks on retry of error_layout
	//has to be public in order to get triggered by layout component
	public void reinitClient(View view) {
		if(!mIsBound) return;
		Log.d(TAG,"reinitClient");
		monitor.restartMonitor(); //start over with setup of client
	}
	
	public void loginButtonClicked (View view) {
		Log.d(TAG,"loginButtonClicked");
		
		//read input data
		EditText emailET = (EditText) findViewById(R.id.emailIn);
		EditText pwdET = (EditText) findViewById(R.id.pwdIn);
		String email = emailET.getText().toString();
		String pwd = pwdET.getText().toString();
		Log.d(TAG,"Input data: " + email + " - " + pwd);
		
		//preventing 0 input
		if(email.length()==0) {
			Toast toast = Toast.makeText(this, "Please enter eMail!", Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		if(pwd.length()==0) {
			Toast toast = Toast.makeText(this, "Please enter password!", Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		
		//trigger async attach
		//TODO adapt layout to multi-project
		String name = getString(R.string.project_name);
		String url = getString(R.string.project_url);
		monitor.attachProjectAsync(url, name, email, pwd);
	}
	
	private OnClickListener mEnableClickListener = new OnClickListener() {
	    public void onClick(View v) {
	    	Log.d(TAG,"mEnableClickListener - onClick");
			if(!mIsBound) return;
			Log.d(TAG,"enableComputation");
			monitor.setRunMode(CommonDefs.RUN_MODE_AUTO);
	    }
	};
	
	private OnClickListener mDisableClickListener = new OnClickListener() {
	    public void onClick(View v) {
	    	Log.d(TAG,"mDisableClickListener - onClick");
			if(!mIsBound) return;
			Log.d(TAG,"disableComputation");
			monitor.setRunMode(CommonDefs.RUN_MODE_NEVER);
	    }
	};

}
