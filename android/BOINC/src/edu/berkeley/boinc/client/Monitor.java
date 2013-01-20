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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;
import edu.berkeley.boinc.LoginActivity;
import edu.berkeley.boinc.MainActivity;
import edu.berkeley.boinc.AppPreferences;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectAttachReply;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;

public class Monitor extends Service{
	
	private final String TAG = "BOINC Client Monitor Service";
	
	private static ClientStatus clientStatus; //holds the status of the client as determined by the Monitor
	private static AppPreferences appPrefs; //hold the status of the app, controlled by AppPreferences
	
	private String clientName; 
	private String authFileName; 
	private String clientPath; 
	
	private Boolean started = false;
	private Thread monitorThread = null;
	
	public static ClientStatus getClientStatus() { //singleton pattern
		if (clientStatus == null) {
			clientStatus = new ClientStatus();
		}
		return clientStatus;
	}
	
	public static AppPreferences getAppPrefs() { //singleton pattern
		if (appPrefs == null) {
			appPrefs = new AppPreferences();
		}
		return appPrefs;
	}

	
	public static Boolean monitorActive = false;
	public static Boolean clientSetupActive = false;
	private Process clientProcess;
	
	private RpcClient rpc = new RpcClient();

	private final Integer maxDuration = 3000; //maximum polling duration

	/*
	 * returns this class, allows clients to access this service's functions and attributes.
	 */
	public class LocalBinder extends Binder {
        public Monitor getService() {
            return Monitor.this;
        }
    }
    private final IBinder mBinder = new LocalBinder();

    /*
     * gets called every-time an activity binds to this service, but not the initial start (onCreate and onStartCommand are called there)
     */
    @Override
    public IBinder onBind(Intent intent) {
    	//Log.d(TAG,"onBind");
        return mBinder;
    }
	
    //onCreate is life-cycle method of service. regardless of bound or started service, this method gets called once upon first creation.
	@Override
    public void onCreate() {
		Log.d(TAG,"onCreate()");
		
		//populate attributes with XML resource values
		clientName = getString(R.string.client_name); 
		authFileName = getString(R.string.auth_file_name); 
		clientPath = getString(R.string.client_path); 
		
		// initialize singleton helper classes and provide application context
		getClientStatus().setCtx(this);
		getAppPrefs().readPrefs(this);
		
		if(!started) {
			started = true;
	        (new ClientMonitorAsync()).execute(new Integer[0]); //start monitor in new thread
	        Log.d(TAG, "asynchronous monitor started!");
		}
		else {
			Log.d(TAG, "asynchronous monitor NOT started!");
		}
    }
	
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {	
    	//this gets called after startService(intent) (either by BootReceiver or AndroidBOINCActivity, depending on the user's autostart configuration)
    	Log.d(TAG, "onStartCommand");
		/*
		 * START_NOT_STICKY is now used and replaced START_STICKY in previous implementations.
		 * Lifecycle events - e.g. killing apps by calling their "onDestroy" methods, or killing an app in the task manager - does not effect the non-Dalvik code like the native BOINC Client.
		 * Therefore, it is not necessary for the service to get reactivated. When the user navigates back to the app (BOINC Manager), the service gets re-started from scratch.
		 * Con: After user navigates back, it takes some time until current Client status is present.
		 * Pro: Saves RAM/CPU.
		 * 
		 * For detailed service documentation see
		 * http://android-developers.blogspot.com.au/2010/02/service-api-changes-starting-with.html
		 */
		return START_NOT_STICKY;
    }

    /*
     * this should not be reached
    */
    @Override
    public void onDestroy() {
    	Log.d(TAG,"onDestroy()");
    	
        // Cancel the persistent notification.
    	((NotificationManager) getSystemService(Service.NOTIFICATION_SERVICE)).cancel(getResources().getInteger(R.integer.autostart_notification_id));
        
		// quitClient();
        
        Toast.makeText(this, "service stopped", Toast.LENGTH_SHORT).show();
    }

    
    //sends broadcast about login (or register) result for login acitivty
	private void sendLoginResultBroadcast(Integer type, Integer result, String message) {
        Intent loginResults = new Intent();
        loginResults.setAction("edu.berkeley.boinc.loginresults");
        loginResults.putExtra("type", type);
        loginResults.putExtra("result", result);
        loginResults.putExtra("message", message);
        getApplicationContext().sendBroadcast(loginResults,null);
	}
	
    public void restartMonitor() {
    	if(Monitor.monitorActive) { //monitor is already active, launch cancelled
    		MainActivity.logMessage(getApplicationContext(), TAG, "monitor active - restart cancelled");
    	}
    	else {
        	Log.d(TAG,"restart monitor");
        	(new ClientMonitorAsync()).execute(new Integer[0]);
    	}
    }
    
    public void forceRefresh() {
    	Log.d(TAG,"forceRefresh()");
    	if(monitorThread!= null) {
    		monitorThread.interrupt();
    	}
    }
    
    public void quitClient() {
    	(new ShutdownClientAsync()).execute();
    }
    
    public void attachProjectAsync(String url, String name, String email, String pwd) {
		Log.d(TAG,"attachProjectAsync");
		String[] param = new String[4];
		param[0] = url;
		param[1] = name;
		param[2] = email;
		param[3] = pwd;
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
	
	public String readAuthToken() {
		File authFile = new File(clientPath+authFileName);
    	StringBuffer fileData = new StringBuffer(100);
    	char[] buf = new char[1024];
    	int read = 0;
    	try{
    		BufferedReader br = new BufferedReader(new FileReader(authFile));
    		while((read=br.read(buf)) != -1){
    	    	String readData = String.valueOf(buf, 0, read);
    	    	fileData.append(readData);
    	    	buf = new char[1024];
    	    }
    		br.close();
    	}
    	catch (FileNotFoundException fnfe) {
    		Log.e(TAG, "auth file not found",fnfe);
    	}
    	catch (IOException ioe) {
    		Log.e(TAG, "ioexception",ioe);
    	}

		String authKey = fileData.toString();
		Log.d(TAG, "authKey: " + authKey);
		return authKey;
	}
	
	public Boolean attachProject(String url, String name, String authenticator) {
    	Boolean success = false;
    	success = rpc.projectAttach(url, authenticator, name); //asynchronous call to attach project
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
    			}
    		}
    	}
    	return success;
    }
	
	public Boolean checkProjectAttached(String url) {
		//TODO
		return false;
	}
	
	public AccountOut lookupCredentials(String url, String id, String pwd) {
    	Integer retval = -1;
    	AccountOut auth = null;
    	AccountIn credentials = new AccountIn();
    	credentials.email_addr = id;
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
    			auth = rpc.lookupAccountPoll();
    			if(auth==null) {
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				retval = auth.error_num;
    				if(auth.error_num == 0) { 
        				Log.d(TAG, "credentials verification result, retrieved authenticator: " + auth.authenticator);
    				}
    			}
    		}
    	}
    	Log.d(TAG, "lookupCredentials returns " + retval);
    	return auth;
    }
	
	public Boolean detachProject(String url){
		return rpc.projectOp(RpcClient.PROJECT_DETACH, url);
	}
	
	public void detachProjectAsync(String url){
		Log.d(TAG,"detachProjectAsync");
		String[] param = new String[1];
		param[0] = url;
		(new ProjectDetachAsync()).execute(param);
	}
    
    public void createAccountAsync(String url, String email, String userName, String pwd, String teamName) {
		Log.d(TAG,"createAccountAsync");
		String[] param = new String[5];
		param[0] = url;
		param[1] = email;
		param[2] = userName;
		param[3] = pwd;
		param[4] = teamName;
		(new CreateAccountAsync()).execute(param);
    }
	
	public AccountOut createAccount(String url, String email, String userName, String pwd, String teamName) {
		AccountIn information = new AccountIn();
		information.url = url;
		information.email_addr = email;
		information.user_name = userName;
		information.passwd = pwd;
		information.team_name = teamName;
		
		AccountOut auth = null;
		
    	Boolean success = rpc.createAccount(information); //asynchronous call to attach project
    	if(success) { //only continue if attach command did not fail
    		// verify success of projectAttach with poll function
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
    			auth = rpc.createAccountPoll();
    			if(auth==null) {
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(auth.error_num == 0) { 
        				Log.d(TAG, "account creation result, retrieved authenticator: " + auth.authenticator);
    				}
    			}
    		}
    	}
    	return auth;
	}
	
	private final class ClientMonitorAsync extends AsyncTask<Integer,String,Boolean> {

		private final String TAG = "ClientMonitorAsync";
		private final Boolean showRpcCommands = false;
		
		 //frequency of which the monitor updates client status via RPC, to often can cause reduced performance!
		private Integer refreshFrequency = getResources().getInteger(R.integer.monitor_refresh_rate_ms);
		
		@Override
		protected Boolean doInBackground(Integer... params) {
			monitorThread = Thread.currentThread(); //save current thread, to interrupt sleep from outside...
			while(true) {
				Log.d(TAG+"-doInBackground","monitor loop...");
				
				if(!rpc.connectionAlive()) { //check whether connection is still alive
					//if connection is not working, either client has not been set up yet or client crashed.
					(new ClientSetupAsync()).execute();
				} else {
					if(showRpcCommands) Log.d(TAG, "getCcStatus");
					CcStatus status = rpc.getCcStatus();
					/*
					if(showRpcCommands) Log.d(TAG, "getState"); 
					CcState state = rpc.getState();
					*/
					if(showRpcCommands) Log.d(TAG, "getResults");
					ArrayList<Result>  results = rpc.getResults();
					if(showRpcCommands) Log.d(TAG, "getProjects");
					ArrayList<Project>  projects = rpc.getProjectStatus();
					if(showRpcCommands) Log.d(TAG, "getTransers");
					ArrayList<Transfer>  transfers = rpc.getFileTransfers();
					if(showRpcCommands) Log.d(TAG, "getGlobalPrefsWorkingStruct");
					GlobalPreferences clientPrefs = rpc.getGlobalPrefsWorkingStruct();
					ArrayList<Message> msgs = new ArrayList<Message>();
					if(getResources().getBoolean(R.bool.tab_debug)) { //retrieve messages only when debug tab is enabled
						Integer count = rpc.getMessageCount();
						if(showRpcCommands) Log.d(TAG, "getMessages, count: " + count);
						msgs = rpc.getMessages(count - 25); //get the most recent 25 messages
					}
					
					if((status!=null)&&(results!=null)&&(projects!=null)&&(transfers!=null)&&(clientPrefs!=null)) {
						Monitor.clientStatus.setClientStatus(status,results,projects,transfers,clientPrefs,msgs);
					} else {
						MainActivity.logMessage(getApplicationContext(), TAG, "client status connection problem");
					}
					
			        Intent clientStatus = new Intent();
			        clientStatus.setAction("edu.berkeley.boinc.clientstatus");
			        getApplicationContext().sendBroadcast(clientStatus);
				}
				
	    		try {
	    			Thread.sleep(refreshFrequency); //sleep
	    		}catch(InterruptedException e){
	    			Log.d(TAG, "sleep interrupted...");
	    		}
			}
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			Log.d(TAG+"-onProgressUpdate",arg0[0]);
			MainActivity.logMessage(getApplicationContext(), TAG, arg0[0]);
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			Log.d(TAG+" - onPostExecute","monitor exit"); 
			Monitor.monitorActive = false;
		}
	}
	
	private final class ClientSetupAsync extends AsyncTask<Void,String,Boolean> {
		private final String TAG = "ClientSetupAsync";
		
		private Integer retryRate = getResources().getInteger(R.integer.monitor_setup_connection_retry_rate_ms);
		private Integer retryAttempts = getResources().getInteger(R.integer.monitor_setup_connection_retry_attempts);
		
		@Override
		protected void onPreExecute() {
			if(Monitor.clientSetupActive) { // setup is already running, cancel execution...
				Log.d(TAG,"setup is already active, quit.");
				cancel(false);
			} else {
				Monitor.clientSetupActive = true;
				getClientStatus().setupStatus = ClientStatus.SETUP_STATUS_LAUNCHING;
				getClientStatus().fire();
			}
		}
		
		@Override
		protected Boolean doInBackground(Void... params) {
			return startUp();
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			Log.d(TAG+" - onPostExecute","setup exit"); 
			Monitor.clientSetupActive = false;
			if(success) {
				getClientStatus().setupStatus = ClientStatus.SETUP_STATUS_AVAILABLE;
				// do not fire new client status here, wait for ClientMonitorAsync to retrieve initial status
				forceRefresh();
			} else {
				getClientStatus().setupStatus = ClientStatus.SETUP_STATUS_ERROR;
				getClientStatus().fire();
			}
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			Log.d(TAG+"-onProgressUpdate",arg0[0]);
			MainActivity.logMessage(getApplicationContext(), TAG, arg0[0]);
		}
		
		private Boolean startUp() {
			Boolean connect =  connectClient(); //try to connect, if client got started in previous life-cycle
			
			if(!connect) { //if connect did not work out, start new client instance and run connect attempts in loop
				Integer counter = 0;
				Boolean setup = setupClient();
				if(!setup) {
					return false; //setup failed
				}
				
				//try to connect to executed Client in loop
				while(!(connect=connectClient()) && (counter<retryAttempts)) { //re-trys setting up the client several times, before giving up.
					MainActivity.logMessage(getApplicationContext(), TAG, "--- restart setup ---");
					counter++;
					try {
						Thread.sleep(retryRate);
					}catch (Exception e) {}
				}
				
				if(!connect) { //connect still not succeeded.
					return false;
				}
			}

			//client is connected.
			return true;
		}
		
		private Boolean connectClient() {
			Boolean success = false;
			
			publishProgress("connect client.");
			
	        success = connect();
	        if(success) {
	        	publishProgress("socket connection established (1/2)");
	        }
	        else {
	        	publishProgress("socket connection failed!");
	        	return success;
	        }
	        
	        //authorize
	        success = authorize();
	        if(success) {
	        	publishProgress("socket authorized. (2/2)");
	        }
	        else {
	        	publishProgress("socket authorization failed!");
	        	return success;
	        }
	        return success;
		}
		
		// copies client binaries from apk to install directory and exetuces them.
		private Boolean setupClient() {
			Boolean success = false;
	
			publishProgress("Client setup.");
			
	        success = installClient(true);
	        if(success) {
	        	publishProgress("installed. (1/2)");
	        }
	        else {
	        	publishProgress("installation failed!");
	        	return success;
	        }
	        
	        //run client
	        success = runClient();
	        if(success) {
	        	publishProgress("started. (2/2)");
	        }
	        else {
	        	publishProgress("start failed!");
	        	return success;
	        }
	        return success;
		}


		// copies the binaries of BOINC client from assets directory into storage space of this application
	    private Boolean installClient(Boolean overwrite){
	    	Boolean success = false;
	    	try {
	    		
	    		//end execution if no overwrite
	    		File boincClient = new File(clientPath+clientName);
	    		if (boincClient.exists() && !overwrite) {
	    			Log.d(TAG,"client exists, end setup of client");
	    			return true;
	    		}
	    		
	    		//delete old client
	    		if(boincClient.exists() && overwrite) {
	    			Log.d(TAG,"delete old client");
	    			boincClient.delete();
	    		}
	    		
	    		//check path and create it
	    		File clientDir = new File(clientPath);
	    		if(!clientDir.exists()) {
	    			clientDir.mkdir();
	    			clientDir.setWritable(true); 
	    		}
	    		
	    		//copy client from assets to clientPath
	    		InputStream assets = getApplicationContext().getAssets().open(clientName); 
	    		OutputStream data = new FileOutputStream(boincClient); 
	    		byte[] b = new byte [1024];
	    		int read; 
	    		while((read = assets.read(b)) != -1){ 
	    			data.write(b,0,read);
	    		}
	    		assets.close(); 
	    		data.flush(); 
	    		data.close();
	    		Log.d(TAG, "copy successful"); 
	    		boincClient.setExecutable(true);
	    		success = boincClient.canExecute();
	    		Log.d(TAG, "native client file in app space is executable: " + success);  
	    	}
	    	catch (IOException ioe) {  
	    		Log.d(TAG, "Exception: " + ioe.getMessage());
	    		Log.e(TAG, "IOException", ioe);
	    	}
	    	
	    	return success;
	    }
	    

	    // executes the BOINC client using the Java Runtime exec method.
	    private Boolean runClient() {
	    	Boolean success = false;
	    	try { 
	        	//starts a new process which executes the BOINC client 
	        	clientProcess = Runtime.getRuntime().exec(clientPath + clientName, null, new File(clientPath));
	        	success = true;
	    	}
	    	catch (IOException ioe) {
	    		Log.d(TAG, "starting BOINC client failed with Exception: " + ioe.getMessage());
	    		Log.e(TAG, "IOException", ioe);
	    	}
	    	return success;
	    }
	    

	    // connects to running BOINC client.
	    private Boolean connect() {
	    	return rpc.open("127.0.0.1", 31416);
	    }
	    

	    // authorizes this application as valid RPC Manager by reading auth token from file and making RPC call.
	    private Boolean authorize() {
	    	String authKey = readAuthToken();
			
			//trigger client rpc
			return rpc.authorize(authKey); 
	    }
		
		
	}
	
	private final class ProjectAttachAsync extends AsyncTask<String,String,Boolean> {

		private final String TAG = "ProjectAttachAsync";
		
		private String url;
		private String projectName;
		private String email;
		private String pwd;
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.url = params[0];
			this.projectName = params[1];
			this.email = params[2];
			this.pwd = params[3];
			Log.d(TAG+"-doInBackground","attachProjectAsync started with: " + url + "-" + projectName + "-" + email + "-" + pwd);
			
			AccountOut auth = lookupCredentials(url,email,pwd);
			
			if(auth == null) {
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_LOGIN,-1,"null");
				Log.d(TAG, "verification failed - exit");
				return false;
			}
			
			if(auth.error_num != 0) { // an error occured
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_LOGIN,auth.error_num,auth.error_msg);
				Log.d(TAG, "verification failed - exit");
				return false;
			}
			Boolean attach = attachProject(url, email, auth.authenticator); 
			if(attach) {
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_LOGIN,0,"Successful!");
			}
			return attach;
		}
	}
	
	private final class CreateAccountAsync extends AsyncTask<String,String,Boolean> {

		private final String TAG = "CreateAccountAsync";
		
		private String url;
		private String email;
		private String userName;
		private String pwd;
		private String teamName;
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.url = params[0];
			this.email = params[1];
			this.userName = params[2];
			this.pwd = params[3];
			this.teamName = params[4];
			Log.d(TAG+"-doInBackground","creating account with: " + url + "-" + email + "-" + userName + "-" + pwd + "-" + teamName);
			
			
			AccountOut auth = createAccount(url, email, userName, pwd, teamName);
			
			if(auth == null) {
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_REGISTRATION,-1,"null");
				Log.d(TAG, "verification failed - exit");
				return false;
			}
			
			if(auth.error_num != 0) { // an error occured
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_REGISTRATION,auth.error_num,auth.error_msg);
				Log.d(TAG, "verification failed - exit");
				return false;
			}
			Boolean attach = attachProject(url, email, auth.authenticator); 
			if(attach) {
				sendLoginResultBroadcast(LoginActivity.BROADCAST_TYPE_REGISTRATION,0,"Successful!");
			}
			return attach;
		}
	}
	
	private final class ProjectDetachAsync extends AsyncTask<String,String,Boolean> {

		private final String TAG = "ProjectDetachAsync";
		
		private String url;
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.url = params[0];
			Log.d(TAG+"-doInBackground","ProjectDetachAsync url: " + url);
			
			Boolean detach = rpc.projectOp(RpcClient.PROJECT_DETACH, url);
			if(detach) {
				Log.d(TAG,"successful.");
			}
			return detach;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			forceRefresh();
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
		
		@Override
		protected void onPostExecute(Boolean success) {
			forceRefresh();
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
		
		@Override
		protected void onPostExecute(Boolean success) {
			forceRefresh();
		}
	}
	
	private final class ShutdownClientAsync extends AsyncTask<Void,Void,Boolean> {

		private final String TAG = "ShutdownClientAsync";
		@Override
		protected Boolean doInBackground(Void... params) {
			Log.d(TAG, "doInBackground");
	    	Boolean success = rpc.quit();
	    	MainActivity.logMessage(getApplicationContext(), TAG, "graceful shutdown returned " + success);
			if(!success) {
				clientProcess.destroy();
				MainActivity.logMessage(getApplicationContext(), TAG, "process killed ");
			}
			return success;
		}
	}
}
