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

package com.example.projectapp;

import android.os.Bundle;
import android.os.IBinder;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class ProjectExampleMainActivity extends Activity implements OnClickListener{

	private final String TAG = "MainActivity";

	private BoincInteractionService bis;
	private Boolean mIsBound;

	private BroadcastReceiver bisUpdateReceiver;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
    	Log.d(TAG, "onCreate");

    	bisUpdateReceiver = new BroadcastReceiver() {
    		@Override
    	    public void onReceive(Context context, Intent intent) {
    			layout(intent.getIntExtra("status", 0));
    	    }
    	};
    	registerReceiver(bisUpdateReceiver, new IntentFilter(getResources().getString(R.string.bis_broadcast_name)));

		super.onCreate(savedInstanceState);

		layout(BoincStatus.INITIALIZING); //default

		bindBoincInteractionService();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

    @Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    super.onDestroy();

    	unregisterReceiver(bisUpdateReceiver);
	    unbindBoincInteractionService();
	}

	private ServiceConnection mBoincInteractionServiceConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        bis = ((BoincInteractionService.LocalBinder)service).getService();
	        mIsBound = true;
	        Log.d(TAG, "onServiceConnected: BoincInteractionService bound.");
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        // This should not happen
	    	bis = null;
	    	mIsBound = false;
	        Toast.makeText(getApplicationContext(), "service disconnected", Toast.LENGTH_SHORT).show();
	    }
	};

	private void bindBoincInteractionService() {
	    // Establish a connection with the service, onServiceConnected gets called when finished.
		Intent i = new Intent(this,BoincInteractionService.class);
		bindService(i, mBoincInteractionServiceConnection, Context.BIND_AUTO_CREATE);
	}

	private void unbindBoincInteractionService() {
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mBoincInteractionServiceConnection);
	        bis = null;
	        mIsBound = false;
	    }
	}

	private void layout(Integer boincStatus) {
		Log.d(TAG,"layout boincStatus: " + boincStatus);
		switch(boincStatus){
		case BoincStatus.INITIALIZING:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.CHECKING_BOINC_AVAILABLILITY:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.BOINC_AVAILABLE:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.BOINC_NOT_AVAILABLE:
			setContentView(R.layout.boinc_not_available);
			Button button = (Button)findViewById(R.id.install_boinc_button);
			button.setOnClickListener(this);
			break;
		case BoincStatus.INSTALLING_BOINC:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.BOINC_INSTALLED:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.BINDING_BOINC_CLIENT_REMOTE_SERVICE:
			setContentView(R.layout.loading);
			break;
		case BoincStatus.CRS_BOUND:
			setContentView(R.layout.activity_main);
			break;
		case BoincStatus.CRS_BINDING_FAILED:
			setContentView(R.layout.loading);
			break;
		default:
			Log.d(TAG, "could not layout status code: " + boincStatus);
		}
	}

	@Override
	public void onClick(View v) {
		Log.d(TAG, "onClick");
		if(v.getId()==R.id.install_boinc_button) {
			if(mIsBound) {
				bis.downloadAndInstallClient();
			}
		}

	}

}
