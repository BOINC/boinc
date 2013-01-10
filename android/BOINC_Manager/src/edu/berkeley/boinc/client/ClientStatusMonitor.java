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
package edu.berkeley.boinc.client;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import edu.berkeley.boinc.AppPreferences;
import edu.berkeley.boinc.IClientService;
import edu.berkeley.boinc.manager.R;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.CcState;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectAttachReply;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;

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
import android.util.Log;
import android.widget.Toast;

public class ClientStatusMonitor extends Service{
	
	private final String TAG = "ClientStatusMonitor";
	private Boolean monitorStarted = false;
	public static Boolean monitorActive = false;
	private RpcClient rpc = new RpcClient();
	//private NotificationManager mNM;
	
	//singletons:
	private static ClientStatus clientStatus; //holds the status of the client as determined by the Monitor
	public static ClientStatus getClientStatus() { //singleton pattern
		if (clientStatus == null) {
			clientStatus = new ClientStatus();
		}
		return clientStatus;
	}
	private static AppPreferences appPrefs; //hold the status of the app, controlled by AppPreferences
	public static AppPreferences getAppPrefs() { //singleton pattern
		if (appPrefs == null) {
			appPrefs = new AppPreferences();
		}
		return appPrefs;
	}
	
	//IClientService.aidl:
	private IClientService mIClientService;
	private Boolean mClientServiceIsBound = false;
	private ServiceConnection mClientServiceConnection = new ServiceConnection() {
		private final String TAG = "mClientServiceConnection";
		@Override
		public void onServiceConnected (ComponentName className, IBinder service) {
			Log.d(TAG,"onServiceConnected: remote ClientService bound.");
			if (service==null) { //checking binding success
				Log.d(TAG,"binding failed, service is null");
				//error while setting up client.
				Log.d(TAG, "onServiceConnected received null, error in remote service initialization!");
				mClientServiceIsBound = false;
				mIClientService = null;
				// set setupStatus to 2
				getClientStatus().setupStatus = 2;
				getClientStatus().fire();
			}
			else {
				mIClientService = IClientService.Stub.asInterface(service);
				mClientServiceIsBound = true;
				Log.d(TAG,"AIDL interface available. continue setup routine...");
				
				if(!monitorStarted){
			        (new ClientMonitorAsync()).execute(new Integer[0]); //start monitor in new thread
			        monitorStarted = true;
				} else {Log.d(TAG, "can't start monitor, already running. monitorStarted: " + monitorStarted);}
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mIClientService = null;
			Log.d(TAG,"service disconnected.");
		}
	};
	
	/*
	 * returns this class, allows clients to access this service's functions and attributes.
	 */
	public class LocalBinder extends Binder {
        public ClientStatusMonitor getService() {
            return ClientStatusMonitor.this;
        }
    }
	
	@Override
    public void onCreate() {
		Log.d(TAG,"onCreate()");
		
		//create singletons and set current application context:
		getAppPrefs().readPrefs(this);
		getClientStatus().setCtx(this);
		
		//start initialization of RemoteClientService
		initRemoteClientService();
    }

    /*
     * gets called every-time an activity binds to this service, but not the initial start (onCreate and onStartCommand are called there)
     */
    @Override
    public IBinder onBind(Intent intent) {
    	//Log.d(TAG,"onBind");
        return mBinder;
    }
    private final IBinder mBinder = new LocalBinder();
    
    
    public void initRemoteClientService() {
    	
    	// set client status
    	getClientStatus().setupStatus = 0;
		getClientStatus().fire();
    	
    	//checking whether client installed on device
		if(!checkClientAvailable()) {
			// client not available
			getClientStatus().setupStatus = 4;
			getClientStatus().fire();
			
			//TODO remove, in order to let user decide, whether to download or not (data usage)
			downloadAndInstallClient();
		} else {
			// client installed, start binding ...
			Intent i = new Intent();
			i.setClassName("edu.berkeley.boinc", "edu.berkeley.boinc.ClientService"); 
			bindService(i, mClientServiceConnection, BIND_AUTO_CREATE);
			
			Log.d(TAG,"initRemoteService finished, continue setup routing in onServiceConnected.");
			//... continue setup routine in onServiceConnected method of mClientServiceConnection...
		}
    }
    
    private Boolean checkClientAvailable() {
    	Boolean found = false; 
    	try {
			Log.d(TAG,"initRemoteService() checking for presence of package: " + getResources().getString(R.string.client_android_package_name));
			PackageInfo clientInfo = getPackageManager().getPackageInfo(getResources().getString(R.string.client_android_package_name), 0);
			if(clientInfo!=null) {
				Log.d(TAG,"initRemoteService() client found!");
				return true;
			}
		} catch(PackageManager.NameNotFoundException e) {}
		//client not installed.
		Log.d(TAG,"initRemoteService() Client not found.");
    	return found;
    }
    
    /*
     * should be triggerable by user, when in setupStatus = 4
     */
    public void downloadAndInstallClient() {
    	//TODO download official Client app from PlayStore
    	//TODO remove write external storage permission if not needed anymore
    	
    	// set client back to "setting up", in order to provide user feedback.
		getClientStatus().setupStatus = 0;
		getClientStatus().fire();
    	(new DownloadAndInstallClientApp()).execute(); //asynchronous call.
    	// initRemoteClientService gets called again by clientInstalledReceiver
    }
    
	
    public void restartMonitor() {
    	if(ClientStatusMonitor.monitorActive) { //monitor is already active, launch cancelled
    		//AndroidBOINCActivity.logMessage(getApplicationContext(), TAG, "monitor active - restart cancelled");
    	}
    	else {
        	Log.d(TAG,"restart monitor");
        	(new ClientMonitorAsync()).execute(new Integer[0]);
    	}
    }
    
    public void quitClient() {
    	(new ShutdownClientAsync()).execute();
    }
    
    public void attachProject(String email, String pwd) {
		Log.d(TAG,"attachProject");
		String[] param = new String[2];
		param[0] = email;
		param[1] = pwd;
		(new ProjectAttachAsync()).execute(param);
    }
    
    
	public void setRunMode(Integer mode) {
		//execute in different thread, in order to avoid network communication in main thread and therefore ANR errors
		(new WriteClientRunModeAsync()).execute(mode);
	}
	
	public void setPrefs(GlobalPreferences globalPrefs) {
		//execute in different thread, in order to avoid network communication in main thread and therefore ANR errors
		(new WriteClientPrefsAsync()).execute(globalPrefs);
	}
	
	private final class ClientMonitorAsync extends AsyncTask<Integer,String,Boolean> {

		private final String TAG = "ClientMonitorAsync";
		private final Boolean showRpcCommands = false;
		
		private Integer refreshFrequency = getResources().getInteger(R.integer.monitor_refresh_interval_ms); //frequency of which the monitor updates client status via RPC
		
		@Override
		protected Boolean doInBackground(Integer... params) {
			Log.d(TAG+"-doInBackground","monitor started in new thread.");
			
			while(true) {
				Log.d(TAG+"-doInBackground","monitor loop...");
				
				
				if(!rpc.connectionAlive()) { //check whether connection is still alive
					//if connection is not working, either client has not been set up yet, user not attached to project, or client crashed.
					//in all cases trigger startUp again.
					Log.d(TAG+"-doInBackground","connection is not alive. continue with setup routine...");
					if(!startUp()) { //synchronous execution in same thread -> blocks monitor until finished
						publishProgress("starting BOINC Client failed. Stop Monitor.");
						//cancel(true); 
						return false; //if startUp fails, stop monitor execution. restart has to be triggered by user.
					}
				}
				
				if(showRpcCommands) Log.d(TAG, "getCcStatus");
				CcStatus status = rpc.getCcStatus();
				if(showRpcCommands) Log.d(TAG, "getState"); 
				CcState state = rpc.getState();
				//TODO getState is quite verbose, optimize!
				if(showRpcCommands) Log.d(TAG, "getTransers");
				ArrayList<Transfer>  transfers = rpc.getFileTransfers();
				if(showRpcCommands) Log.d(TAG, "getGlobalPrefsWorkingStruct");
				GlobalPreferences clientPrefs = rpc.getGlobalPrefsWorkingStruct();
				//TODO only when debug tab:
				//TODO room for improvements, dont retrieve complete list every time, but only new messages.
				Integer count = rpc.getMessageCount();
				//Log.d(TAG, "message count: " + count);
				if(showRpcCommands) Log.d(TAG, "getMessages, count: " + count);
				ArrayList<Message> msgs = rpc.getMessages(count - 25); //get the most recent 25 messages
				
				if((state!=null)&&(status!=null)&&(transfers!=null)&&(clientPrefs!=null)) {
					ClientStatusMonitor.clientStatus.setClientStatus(status,state,transfers,clientPrefs,msgs);
				} else {
					//AndroidBOINCActivity.logMessage(getApplicationContext(), TAG, "client status connection problem");
				}
				
				//TODO pointless, since ClientStatus fires clientstatuschanged Broadcast?
				/*
		        Intent clientStatus = new Intent();
		        clientStatus.setAction("edu.berkeley.boinc.clientstatus");
		        getApplicationContext().sendBroadcast(clientStatus); */
				
	    		try {
	    			Thread.sleep(refreshFrequency); //sleep
	    		}catch(Exception e){}
			}
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			Log.d(TAG+"-onProgressUpdate",arg0[0]);
			//AndroidBOINCActivity.logMessage(getApplicationContext(), TAG, arg0[0]);
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			Log.d(TAG+" - onPostExecute","monitor exit"); 
			ClientStatusMonitor.monitorActive = false;
		}
		

		
		private Boolean startUp() {
			//status control
			Boolean success = false;
			Boolean clientConnect = false;
			
			// try connecting ...
			publishProgress("connecting to client... (0/2)");
			clientConnect = connectClient();
			
			if(!clientConnect) {
				//connect failed (socket connection or client authorization) publish setupStatus 2 -> permanent error!
				publishProgress("connection failed!");
				getClientStatus().setupStatus = 2;
				getClientStatus().fire();
				return false;
			}

			//client is connected.
			
			Boolean login = false;
			login = checkLogin();
			if(login) { // Client is attached to project, publish setupStatus 1 -> setup complete!
				getClientStatus().setupStatus = 1;
				getClientStatus().fire();
				//return true in order to start monitor
				success = true;
			} else { //client is not attached to project, publish setupStatus 3 -> wait for user input
				getClientStatus().setupStatus = 3;
				getClientStatus().fire();
			}
			
			//return success status, start monitor only if true.
			return success;
		}
		
		private Boolean connectClient() {
			Boolean success = false;
			Log.d(TAG+"-doInBackground connectClient()","connecting to client in loop....");
			
			//connecting to client in loop ...
			Integer counter = 0;
			Integer max = getResources().getInteger(R.integer.setup_connection_attempts); //max number of connection attempts attempts
			Integer duration = getResources().getInteger(R.integer.setup_connection_attempt_sleep_duration_ms); //sleep duration
			while(!(success = connect()) && (counter<max)) {
				counter++;
				Log.d(TAG+"-doInBackground connectClient()","1. connection attempt failed, retry in " + duration + "ms.");
				try {
					Thread.sleep(duration);
				}catch (Exception e) {}
			}
	        if(success) {
	        	publishProgress("socket connection established (1/2)");
	        }
	        else {
	        	Log.d(TAG+"-doInBackground connectClient()","connection failed.");
	        	return success;
	        }
	        
	        //authorize
	        success = authorize();
	        if(success) {
	        	publishProgress("socket authorized. (2/2)");
	        }
	        else {
	        	Log.d(TAG+"-doInBackground connectClient()","socket authorization failed.");
	        	return success;
	        }
	        return success;
		}
		
		/*
		 * checks whether client is attached to project
		 */
		private Boolean checkLogin() {
			publishProgress("verify project login:");
			Boolean success = false;
			success = verifyProjectAttach();
	        if(success) {
	        	publishProgress("credentials verified. logged in!");
	        }
	        else {
	        	publishProgress("not logged in!");
	        	return success;
	        }
			return success;
		}
	    
	    /*
	     * called by setupClientRoutine() and reconnectClient()
	     * connects to running BOINC client.
	     */
	    private Boolean connect() {
	    	return rpc.open("127.0.0.1", 31416);
	    }
	    
	    /*
	     * called by setupClientRoutine() and reconnectClient()
	     * authorizes this application as valid RPC Manager by reading auth token from file and making RPC call.
	     */
	    private Boolean authorize() {
	    	
	    	String authKey = "";
	    	List<String> projectURLs = new ArrayList<String>();
	    	projectURLs.add(getResources().getString(R.string.project_url));
	    	String applicationPackageName = getPackageName(); //method on Context, provides package name of current application
	    	Log.d(TAG, "applicationPackageName: " + applicationPackageName);
	    	try {
	    		authKey = mIClientService.getAuthToken(projectURLs, applicationPackageName);
	    		if (authKey==null) {
	    			Log.d(TAG+"-doInBackground authorize()","auth key returned by AIDL call is null, return.");
	    			return false;
	    		}
	    	}
	    	catch (Exception e) {Log.e(TAG, "error in receiveing auth token", e);}
	    	
			Log.d(TAG, "authKey: " + authKey);
			return rpc.authorize(authKey); 
	    }
	    
	    private Boolean verifyProjectAttach() {
	    	//TODO optimize so method checks for the actually specified project in configuration.xml
	    	Log.d(TAG, "verifyProjectAttach");
	    	Boolean success = false;
	    	ArrayList<Project> projects = rpc.getProjectStatus();
	    	Integer attachedProjectsAmount = projects.size();
	    	Log.d(TAG,"projects amount " + projects.size()); 
	    	if(attachedProjectsAmount > 0) { // there are attached projects
	    		success = true;
	    	}
	    	Log.d(TAG,"verifyProjectAttach about to return with " + success);
	    	return success;
	    }
	}
	
	private final class ProjectAttachAsync extends AsyncTask<String,String,Boolean> {

		private final String TAG = "ProjectAttachAsync";
		
		private final Integer maxDuration = 3000; //maximum polling duration
		
		private String email;
		private String pwd;
		
		@Override
		protected void onPreExecute() {
			Log.d(TAG+"-onPreExecute","publish setupStatus 0"); //client is in setup routine... again.
			getClientStatus().setupStatus = 0;
			getClientStatus().fire();
		}
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.email = params[0];
			this.pwd = params[1];
			Log.d(TAG+"-doInBackground","login started with: " + email + "-" + pwd);
			
			Integer retval = lookupCredentials();
			Boolean success = false;
			switch (retval) {
			case 0:
				Log.d(TAG, "verified successful");
				success = true;
				break;
			case -206:
				Log.d(TAG, "password incorrect!");
				publishProgress("Password Incorrect!");
				break;
			case -136:
				Log.d(TAG, "eMail incorrect!");
				publishProgress("eMail Incorrect!");
				break;
			case -113:
				Log.d(TAG, "No internet connection!");
				publishProgress("No internet connection!");
				break;
			default:
				Log.d(TAG, "unkown error occured!");
				publishProgress("Unknown Error!");
				break;
			}
			
			if(!success) {
				Log.d(TAG, "verification failed - exit");
				return false;
			}
			
			Boolean attach = attachProject(); //tries credentials stored in AppPreferences singleton, terminates after 3000 ms in order to prevent "ANR application not responding" dialog
			if(attach) {
				publishProgress("Successful.");
			}
			return attach;
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			Log.d(TAG+"-onProgressUpdate",arg0[0]);
			//AndroidBOINCActivity.logMessage(getApplicationContext(), TAG, arg0[0]);
			Toast toast = Toast.makeText(getApplicationContext(), arg0[0], Toast.LENGTH_SHORT);
			toast.show();
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) { //login successful
				Log.d(TAG,"login successful, restart monitor");
				restartMonitor();
			} else { //login failed
				Log.d(TAG,"login failed, publish");
				getClientStatus().setupStatus = 3;
				getClientStatus().fire();
			}
			
		}
		
		private Integer lookupCredentials() {
	    	Integer retval = -1;
	    	AccountIn credentials = new AccountIn();
	    	credentials.email_addr = email;
	    	credentials.passwd = pwd;
	    	credentials.url = getString(R.string.project_url);
	    	Boolean success = rpc.lookupAccount(credentials); //asynch
	    	if(success) { //only continue if lookupAccount command did not fail
	    		//get authentication token from lookupAccountPoll
	    		Integer counter = 0;
	    		Integer sleepDuration = 500; //in mili seconds
	    		Integer maxLoops = maxDuration / sleepDuration;
	    		Boolean loop = true;
	    		while(loop && (counter < maxLoops)) {
	    			loop = false;
	    			try {
	    				Thread.sleep(sleepDuration);
	    			} catch (Exception e) {}
	    			counter ++;
	    			AccountOut auth = rpc.lookupAccountPoll();
	    			if(auth==null) {
	    				return -1;
	    			}
	    			if (auth.error_num == -204) {
	    				loop = true; //no result yet, keep looping
	    			}
	    			else {
	    				//final result ready
	    				if(auth.error_num == 0) { //write usable results to AppPreferences
	        				AppPreferences appPrefs = ClientStatusMonitor.getAppPrefs(); //get singleton appPrefs to save authToken
	        				appPrefs.setEmail(email);
	        				appPrefs.setPwd(pwd);
	        				appPrefs.setMd5(auth.authenticator);
	        				Log.d(TAG, "credentials verified");
	    				}
	    				retval = auth.error_num;
	    			}
	    		}
	    	}
	    	Log.d(TAG, "lookupCredentials returns " + retval);
	    	return retval;
	    }
	    
	    private Boolean attachProject() {
	    	Boolean success = false;

	    	//get singleton appPrefs to read authToken
			AppPreferences appPrefs = ClientStatusMonitor.getAppPrefs(); 
			
			//make asynchronous call to attach project
	    	success = rpc.projectAttach(getString(R.string.project_url), appPrefs.getMd5(), getString(R.string.project_name));
	    	if(success) { //only continue if attach command did not fail
	    		// verify success of projectAttach with poll function
	    		success = false;
	    		Integer counter = 0;
	    		Integer sleepDuration = 500; //in mili seconds
	    		Integer maxLoops = maxDuration / sleepDuration;
	    		while(!success && (counter < maxLoops)) {
	    			try {
	    				Thread.sleep(sleepDuration);
	    			} catch (Exception e) {}
	    			counter ++;
	    			ProjectAttachReply reply = rpc.projectAttachPoll();
	    			Integer result = reply.error_num;
	    			if(result == 0) {
	    				success = true;
	    	    		//AndroidBOINCActivity.logMessage(getApplicationContext(), TAG, "project attached!");
	    				// Client is attached to project, publish setupStatus 1 -> setup complete!
	    				getClientStatus().setupStatus = 1;
	    				getClientStatus().fire();
	    			}
	    		}
	    	}
	    	return success;
	    }
	}

	private final class WriteClientPrefsAsync extends AsyncTask<GlobalPreferences,Void,Boolean> {

		private final String TAG = "WriteClientPrefsAsync";
		@Override
		protected Boolean doInBackground(GlobalPreferences... params) {
			Log.d(TAG, "doInBackground");
			Boolean retval1 = rpc.setGlobalPrefsOverrideStruct(params[0]); //set new override settings
			Boolean retval2 = rpc.readGlobalPrefsOverride(); //trigger reload of override settings
			if(retval1 && retval2) {
				Log.d(TAG, "successful.");
				return true;
			}
			return false;
		}
	}
	
	private final class WriteClientRunModeAsync extends AsyncTask<Integer,Void,Boolean> {

		private final String TAG = "WriteClientRunModeAsync";
		@Override
		protected Boolean doInBackground(Integer... params) {
			Log.d(TAG, "doInBackground");
			Boolean success = rpc.setRunMode(params[0],0);
			Log.d(TAG,"run mode set to " + params[0] + " returned " + success);
			return success;
		}
	}
	
	private final class ShutdownClientAsync extends AsyncTask<Void,Void,Boolean> {

		private final String TAG = "ShutdownClientAsync";
		@Override
		protected Boolean doInBackground(Void... params) {
			Log.d(TAG, "doInBackground");
	    	Boolean success = rpc.quit();
	    	Log.d(TAG,"graceful shutdown returned: " + success);
			return success;
		}
	}
	
	private final class DownloadAndInstallClientApp extends AsyncTask<Void,Void,Boolean> {

		private final String TAG = "DownloadAndInstallClientApp";
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
		        	Log.d(TAG, "installation successful: " + packageName + " ; restarting initRemoteClientService()");
		        	initRemoteClientService(); //restarting initialization of ClientService
		        }
            }
        }          
	};
	
}
