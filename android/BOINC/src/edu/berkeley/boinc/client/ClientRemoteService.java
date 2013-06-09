package edu.berkeley.boinc.client;

import edu.berkeley.boinc.utils.*;

import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.RpcClient;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class ClientRemoteService extends Service {

	private Monitor monitor;
	private Boolean mIsMonitorBound = false;

	@Override
	public void onCreate() {
		if(Logging.DEBUG) Log.d(Logging.TAG, "ClientRemoteService onCreate()");
		doBindService();
	}
	
	@Override
    public void onDestroy() {
    	if(Logging.DEBUG) Log.d(Logging.TAG, "ClientRemoteService onDestroy()");
        doUnbindService();
    }
	
	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder; //return binder for remote interface, specified in IClientRemoteService.aidl
	}
	
	private final IClientRemoteService.Stub mBinder = new IClientRemoteService.Stub() {

		@Override
		public boolean isReady() throws RemoteException {
			return mIsMonitorBound;
		}

		@Override
		public int getVersionCode() throws RemoteException {
			Integer version = 0;
			try {
				version = getPackageManager().getPackageInfo(getPackageName(), 0).versionCode;
		    } catch (NameNotFoundException e) {
		        if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not retrieve own version code!", e);
		    }
			return version;
		}

		@Override
		public boolean attachProject(String packageName, String url, String id, String pwd) throws RemoteException {
			// TODO store packageName in AppPreferences
			if(mIsMonitorBound) {
				return monitor.attachProject(url, id, pwd);
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not attach project, service not bound!"); return false;}
		}

		@Override
		public boolean checkProjectAttached(String url) throws RemoteException {
			if(mIsMonitorBound) {
				return monitor.checkProjectAttached(url);
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not attach project, service not bound!"); return false;}
		}

		@Override
		public AccountOut verifyCredentials(String url, String id, String pwd, boolean usesName) throws RemoteException {
			if(mIsMonitorBound) {
				return monitor.lookupCredentials(url, id, pwd, usesName);
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not verify credentials, service not bound!"); return null;}
		}

		@Override
		public boolean detachProject(String packageName, String url) throws RemoteException {
			// TODO remove packageName in AppPreferences
			if(mIsMonitorBound) {
				return monitor.projectOperation(RpcClient.PROJECT_DETACH,url);
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not detach project, service not bound!"); return false;}
		}

		@Override
		public AccountOut createAccount(String url, String email, String userName, String pwd, String teamName) throws RemoteException {
			if(mIsMonitorBound) {
				return monitor.createAccount(url, email, userName, pwd, teamName);
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not create account, service not bound!"); return null;}
		}

		@Override
		public String getRpcAuthToken() throws RemoteException {
			if(mIsMonitorBound) {
				return monitor.readAuthToken();
			} else {if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientRemoteService could not read auth token, service not bound!"); return null;}
		}
    };
	
	/*
	 * ClientRemoteService needs to bind to Monitor service, since it is using the web RPC methods.
	 * the following methods are required...
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "ClientRemoteService onServiceConnected - local Monitor service bound.");
	        monitor = ((Monitor.LocalBinder)service).getService();
	        mIsMonitorBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "ClientRemoteService onServiceDisconnected - local Monitor service bound.");
	        monitor = null;
	        mIsMonitorBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsMonitorBound) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "ClientRemoteService binding local Monitor service...");
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, Context.BIND_AUTO_CREATE);
		}
	}

	private void doUnbindService() {
	    if(mIsMonitorBound) {
	    	getApplicationContext().unbindService(mConnection);
	    	mIsMonitorBound = false;
	    }
	}

}
