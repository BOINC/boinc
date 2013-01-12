package edu.berkeley.boinc.client;

import edu.berkeley.boinc.rpc.AccountOut;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class ClientRemoteService extends Service {

	private final String TAG = "ClientRemoteService";

	private Monitor monitor;
	private Boolean mIsBound = false;

	@Override
	public void onCreate() {
		Log.d(TAG,"onCreate()");
		doBindService();
	}
	
	@Override
    public void onDestroy() {
    	Log.d(TAG,"onDestroy()");
        doUnbindService();
    }
	
	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder; //return binder for remote interface, specified in IClientRemoteService.aidl
	}
	
	private final IClientRemoteService.Stub mBinder = new IClientRemoteService.Stub() {

		@Override
		public boolean attachProject(String packageName, String url, String id, String pwd) throws RemoteException {
			// TODO store packageName in AppPreferences
			if(mIsBound) {
				return monitor.attachProject(url, id, pwd);
			} else {Log.e(TAG, "could not attach project, service not bound!"); return false;}
		}

		@Override
		public AccountOut verifyCredentials(String url, String id, String pwd) throws RemoteException {
			if(mIsBound) {
				return monitor.lookupCredentials(url, id, pwd);
			} else {Log.e(TAG, "could not verify credentials, service not bound!"); return null;}
		}

		@Override
		public boolean detachProject(String packageName, String url) throws RemoteException {
			// TODO remove packageName in AppPreferences
			if(mIsBound) {
				return monitor.detachProject(url);
			} else {Log.e(TAG, "could not detach project, service not bound!"); return false;}
		}

		@Override
		public AccountOut createAccount(String url, String email, String userName, String pwd, String teamName) throws RemoteException {
			if(mIsBound) {
				return monitor.createAccount(url, email, userName, pwd, teamName);
			} else {Log.e(TAG, "could not create account, service not bound!"); return null;}
		}

		@Override
		public String getRpcAuthToken() throws RemoteException {
			if(mIsBound) {
				return monitor.readAuthToken();
			} else {Log.e(TAG, "could not read auth token, service not bound!"); return null;}
		}
    };
	
	/*
	 * ClientRemoteService needs to bind to Monitor service, since it is using the web RPC methods.
	 * the following methods are required...
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG,"onServiceConnected - local Monitor service bound.");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
	        mIsBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0);
		}
	}

	private void doUnbindService() {
	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}

}
