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

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import edu.berkeley.boinc.client.IClientRemoteService;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Environment;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class BoincInteractionService extends Service {

	private final String TAG = "BoincInteractionService";

	public Integer boincStatus = BoincStatus.INITIALIZING; //TODO could be replaced by sticky broadcasts??!

	//IClientRemoteService attributes
	private IClientRemoteService mIClientRemoteService;
	private Boolean mClientRemoteServiceIsBound = false;
	private ServiceConnection mClientRemoteServiceConnection = new ServiceConnection() {
		private final String TAG = "mClientRemoteServiceConnection";
		@Override
		public void onServiceConnected (ComponentName className, IBinder service) {
			Log.d(TAG,"onServiceConnected: remote ClientService bound.");
			if(service == null) {
				Log.e(TAG,"onServiceConnected: service is null");
				publishAndDispatch(BoincStatus.CRS_BINDING_FAILED);
				return;
			}
			mIClientRemoteService = IClientRemoteService.Stub.asInterface(service);
			mClientRemoteServiceIsBound = true;
			publishAndDispatch(BoincStatus.CRS_BOUND);
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mIClientRemoteService = null;
			mClientRemoteServiceIsBound = false;
			Log.d(TAG,"service disconnected.");
		}
	};
	//management attributes for this local service
	private LocalBinder mBinder = new LocalBinder();
	public class LocalBinder extends Binder {
        public BoincInteractionService getService() {
            return BoincInteractionService.this;
        }
    }

	// service lifecycle methods
	@Override
    public void onCreate() {
		Log.d(TAG,"onCreate()");

    	//start of setup routine
		checkBoincClientAvailable();
    }

    @Override
	public void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    super.onDestroy();

	    unbindClientRemoteService();
	}

	//onBind gets triggered, when activities bind to this local client
	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	private void bindClientRemoteService () {
		publishAndDispatch(BoincStatus.BINDING_BOINC_CLIENT_REMOTE_SERVICE);
		Intent i = new Intent();
		i.setClassName(getResources().getString(R.string.client_android_package_name), getResources().getString(R.string.client_remote_service_name));
		bindService(i, mClientRemoteServiceConnection, BIND_AUTO_CREATE);
	}

	private void unbindClientRemoteService() {
		if(mClientRemoteServiceIsBound) {
			unbindService(mClientRemoteServiceConnection);
			mClientRemoteServiceIsBound = false;
			mIClientRemoteService = null;
		}
	}

    private void checkBoincClientAvailable() {
    	(new CheckBoincClientAvailable()).execute(); //asynchronous call.
    }

    public void downloadAndInstallClient() {
    	(new DownloadAndInstallClientApp()).execute(); //asynchronous call.
    }

    private void tryClientRemoteServiceInterface() {
		publishAndDispatch(BoincStatus.TRYING_CRS_INTERFACE);
    	//busy waiting until interface returns true
    	Boolean ready = false;
		Integer counter = 0;
		Integer sleepDuration = 500; //in milliseconds
		Integer maxLoops = getResources().getInteger(R.integer.busy_waiting_timeout_ms) / sleepDuration;
		while(!ready && (counter < maxLoops)) {
			try {
				Log.d(TAG, "tryClientRemoteServiceInterface - sleep...");
				Thread.sleep(sleepDuration);
			} catch (Exception e) {}
			try {
				ready = mIClientRemoteService.isReady();
			} catch (RemoteException re) {Log.e(TAG, "trying CRS interface", re);}
		}
		Log.d(TAG, "tryClientRemoteServiceInterface, ready: " + ready);
		if(ready) {
			publishAndDispatch(BoincStatus.CRS_INTERFACE_READY);
		} else {
			publishAndDispatch(BoincStatus.CRS_INTERFACE_TIMEOUT);
		}
    }

    private void checkProjectAttached() {
    	//TODO
    	publishAndDispatch(BoincStatus.PROJECT_ATTACHED);
    	/*
    	publishAndDispatch(BoincStatus.CHECKING_PROJECT_ATTACHED);
    	Boolean success = false;
		try {
			success = mIClientRemoteService.checkProjectAttached(getResources().getString(R.string.project_url));
		} catch (RemoteException re) {Log.e(TAG, "trying CRS interface", re);}
    	if(success) {
    		publishAndDispatch(BoincStatus.PROJECT_ATTACHED);
    	} else {
    		publishAndDispatch(BoincStatus.PROJECT_NOT_ATTACHED);
    	}*/
    }

    private void openBoincApp() {
    	Intent i = new Intent();
    	i.setClassName(getResources().getString(R.string.client_android_package_name),getResources().getString(R.string.client_main_activity_name));
    	i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    	i.putExtra("app_name", getResources().getString(R.string.app_name));
    	i.putExtra("project_name", getResources().getString(R.string.project_name));
    	startActivity(i);
    }

    private final class CheckBoincClientAvailable extends AsyncTask<Void,Void,Boolean> {

		private final String TAG = "CheckBoincClientAvailable";

		@Override
		protected void onPreExecute() {
			publishAndDispatch(BoincStatus.CHECKING_BOINC_AVAILABLILITY);
		}

		@Override
		protected Boolean doInBackground(Void... params) {
	    	Log.d(TAG+"-doInBackground", "checking...");
	    	Boolean found = false;
	    	try {
				Log.d(TAG,"checkBoincAvailable() checking for presence of package: " + getResources().getString(R.string.client_android_package_name));
				PackageInfo clientInfo = getPackageManager().getPackageInfo(getResources().getString(R.string.client_android_package_name), 0);
				if(clientInfo!=null) {
					return true;
				}
			} catch(PackageManager.NameNotFoundException e) {}
			//client not installed.
	    	return found;
		}

		@Override
		protected void onPostExecute(Boolean success) {
			if(success) {
				Log.d(TAG,"checkBoincAvailable() app found!");
				//continue with binding to Boinc's ClientRemoteService
				publishAndDispatch(BoincStatus.BOINC_AVAILABLE);
			} else {
				Log.d(TAG,"checkClientAvailable() app not found.");
				//stop execution and wait for user input
				publishAndDispatch(BoincStatus.BOINC_NOT_AVAILABLE);
			}

		}
	}

	private final class DownloadAndInstallClientApp extends AsyncTask<Void,Void,Boolean> {

    	//TODO download official Client app from PlayStore
    	//TODO remove write external storage permission if not needed anymore

		private final String TAG = "DownloadAndInstallClientApp";

		@Override
		protected void onPreExecute() {
			publishAndDispatch(BoincStatus.INSTALLING_BOINC);
		}

		@Override
		protected Boolean doInBackground(Void... params) {
	    	Log.d(TAG+"-doInBackground", "installing Client...");
	    	try{
		    	if(Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) { //checking whether SD card is present

		    		//downloading apk
		    		URL url = new URL(getResources().getString(R.string.client_download_url));
		            HttpURLConnection c = (HttpURLConnection) url.openConnection();
		            c.setRequestMethod("GET");
		            c.setDoOutput(true);
		            c.connect();
		    		File tmpDir = new File(Environment.getExternalStorageDirectory(),"/boinc_download/");
		    		tmpDir.mkdirs();
		    		File app = new File(tmpDir,"boinc_client.apk");
		    		FileOutputStream fos = new FileOutputStream(app);
		    		InputStream is = c.getInputStream();
		            byte[] buffer = new byte[1024];
		            int len1 = 0;
		            while ((len1 = is.read(buffer)) != -1) {
		                fos.write(buffer, 0, len1);
		            }
		            fos.close();
		            is.close();

		            //register receiver for successful install
		            IntentFilter mIntentFilter = new IntentFilter();
		            mIntentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
		            mIntentFilter.addDataScheme("package");
		            registerReceiver(clientInstalledReceiver, mIntentFilter);

		            //installation in new activity
		            Intent intent = new Intent(Intent.ACTION_VIEW);
		            intent.setDataAndType(Uri.fromFile(app), "application/vnd.android.package-archive");
		            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		            startActivity(intent);
		    	} else{
		    		Log.d(TAG, "getExternalStorageState() does not return MEDIA_MOUNTED");
		    		return false;
		    	}
	    	} catch(Exception e) {Log.e(TAG, "error in installClient", e); return false;}
			return true;
		}
	}

	BroadcastReceiver clientInstalledReceiver = new BroadcastReceiver() {

		private final String TAG = "clientInstalledReceiver";

		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG,"onReceive");
		    String action = intent.getAction();
		    if (Intent.ACTION_PACKAGE_ADDED.equals(action)) {
		        Uri data = intent.getData();
		        String packageName = data.getEncodedSchemeSpecificPart();
		        if(packageName.equals(getResources().getString(R.string.client_android_package_name))){
		        	Log.d(TAG, "installation successful: " + packageName);
		        	publishAndDispatch(BoincStatus.BOINC_INSTALLED);
		        }
            }
        }
	};

	private void publishAndDispatch(Integer status) {
		Log.d(TAG,"publishAndDispatch with status: " + status);

		//write in variable, to allow status pulling
		boincStatus = status;

		//send broadcast for activities to update layout
		Intent i = new Intent(getResources().getString(R.string.bis_broadcast_name));
		i.putExtra("status", status);
		sendBroadcast(i);

		//dispatch next task:
		switch(status) {
		case BoincStatus.CHECKING_BOINC_AVAILABLILITY:
			//do nothing, not a final status
			break;
		case BoincStatus.BOINC_AVAILABLE:
			//continue with binding to ClientRemoteService
			bindClientRemoteService();
			break;
		case BoincStatus.BOINC_NOT_AVAILABLE:
			//do nothing, wait for user input to continue
			break;
		case BoincStatus.INSTALLING_BOINC:
			//do nothing, not a final status
			break;
		case BoincStatus.BOINC_INSTALLED:
			//continue with binding to ClientRemoteService
			bindClientRemoteService();
			break;
		case BoincStatus.BINDING_BOINC_CLIENT_REMOTE_SERVICE:
			//do nothing, not a final status
			break;
		case BoincStatus.CRS_BOUND:
			tryClientRemoteServiceInterface();
			break;
		case BoincStatus.CRS_BINDING_FAILED:
			//do nothing, wait for user input to re-try
			break;
		case BoincStatus.TRYING_CRS_INTERFACE:
			//do nothing, not a final status
			break;
		case BoincStatus.CRS_INTERFACE_READY:
			checkProjectAttached();
			break;
		case BoincStatus.CRS_INTERFACE_TIMEOUT:
			//do nothing, wait for user input to re-try
			break;
		case BoincStatus.ATTACHING_PROJECT:
			//do nothing, not a final status
			break;
		case BoincStatus.CHECKING_PROJECT_ATTACHED:
			//do nothing, not a final status
			break;
		case BoincStatus.PROJECT_ATTACHED:
			openBoincApp();
			break;
		case BoincStatus.PROJECT_NOT_ATTACHED:
			// do nothing, wait for user to input credentials
			break;
		default:
			Log.d(TAG,"publishAndDispatch does not dispatch BoincStatus code: " + status);
		}
	}

}
