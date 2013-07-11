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

import edu.berkeley.boinc.utils.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;

import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import edu.berkeley.boinc.AppPreferences;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.CcState;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.DeviceStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectAttachReply;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;

public class Monitor extends Service {
	
	private static ClientStatus clientStatus; //holds the status of the client as determined by the Monitor
	private static AppPreferences appPrefs; //hold the status of the app, controlled by AppPreferences
	
	public static Boolean monitorActive = false;
	
	// XML defined variables, populated in onCreate
	private String clientName; 
	private String clientCLI; 
	private String clientCABundle; 
	private String clientConfig; 
	private String authFileName; 
	private String allProjectsList; 
	private String globalOverridePreferences;
	private String clientPath; 
	private Integer clientStatusInterval;
	private Integer deviceStatusIntervalScreenOff;
	
	private Timer updateTimer = new Timer(true); // schedules frequent client status update
	private TimerTask statusUpdateTask = new StatusUpdateTimerTask();
	private boolean updateBroadcastEnabled = true;
	private DeviceStatus deviceStatus = null;
	private Integer screenOffStatusOmitCounter = 0;
	
	// screen on/off updated by screenOnOffBroadcastReceiver
	private boolean screenOn = false;
	
	//private Process clientProcess;
	private RpcClient rpc = new RpcClient();

	private final Integer maxDuration = 3000; //maximum polling duration


	// installs client and required files, executes client and reads initial preferences
	// used by ClientMonitorAsync if no connection is available
	// includes network communication => don't call from UI thread!
	private Boolean clientSetup() {
		if(Logging.DEBUG) Log.d(Logging.TAG,"Monitor.clientSetup()");
		
		// initialize full wakelock.
		// gets used if client has to be started from scratch
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		WakeLock setupWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, Logging.TAG);
		// do not acquire here, otherwise screen turns on, every time rpc connection
		// gets reconnected.
		
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"Monitor.clientSetup: Could not load data, clientStatus not initialized.");
			return false;
		}
		
		status.setSetupStatus(ClientStatus.SETUP_STATUS_LAUNCHING,true);
		String clientProcessName = clientPath + clientName;

		String md5AssetClient = ComputeMD5Asset(clientName);
		//if(Logging.DEBUG) Log.d(Logging.TAG, "Hash of client (Asset): '" + md5AssetClient + "'");

		String md5InstalledClient = ComputeMD5File(clientProcessName);
		//if(Logging.DEBUG) Log.d(Logging.TAG, "Hash of client (File): '" + md5InstalledClient + "'");

		// If client hashes do not match, we need to install the one that is a part
		// of the package. Shutdown the currently running client if needed.
		//
		if (!md5InstalledClient.equals(md5AssetClient)) {
		//if (md5InstalledClient.compareToIgnoreCase(md5AssetClient) != 0) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"Hashes of installed client does not match binary in assets - re-install.");
			
			// try graceful shutdown using RPC (faster)
	    	Boolean success = false;
			if(connectClient()) {
				rpc.quit();
		    	Integer attempts = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_attempts);
		    	Integer sleepPeriod = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_rate_ms);
		    	for(int x = 0; x < attempts; x++) {
		    		try {
		    			Thread.sleep(sleepPeriod);
		    		} catch (Exception e) {}
		    		if(getPidForProcessName(clientProcessName) == null) { //client is now closed
		        		if(Logging.DEBUG) Log.d(Logging.TAG,"quitClient: gracefull RPC shutdown successful after " + x + " seconds");
		    			success = true;
		    			x = attempts;
		    		}
		    	}
			}
			
			// quit with OS signals
			if(!success) quitProcessOsLevel(clientProcessName);

			// at this point client is definitely not running. install new binary...
			if(!installClient()) {
	        	if(Logging.WARNING) Log.w(Logging.TAG, "BOINC client installation failed!");
	        	return false;
	        }
		}
		
		// Start the BOINC client if we need to.
		//
		Integer clientPid = getPidForProcessName(clientProcessName);
		if(clientPid == null) {
        	if(Logging.DEBUG) Log.d(Logging.TAG, "Starting the BOINC client");
    		// wake up device and acquire full WakeLock here to allow BOINC client to detect
    		// all available CPU cores if not acquired and device in power saving mode, client
    		// might detect fewer CPU cores than available.
    		// Lock needs to be release, before return!
    		setupWakeLock.acquire();
			if (!runClient()) {
	        	if(Logging.DEBUG) Log.d(Logging.TAG, "BOINC client failed to start");
	        	setupWakeLock.release();
				return false;
			}
		}

		// Try to connect to executed Client in loop
		//
		Integer retryRate = getResources().getInteger(R.integer.monitor_setup_connection_retry_rate_ms);
		Integer retryAttempts = getResources().getInteger(R.integer.monitor_setup_connection_retry_attempts);
		Boolean connected = false;
		Integer counter = 0;
		while(!connected && (counter < retryAttempts)) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "Attempting BOINC client connection...");
			connected = connectClient();
			counter++;

			try {
				Thread.sleep(retryRate);
			} catch (Exception e) {}
		}
		
		if(connected) { // connection established
			// make client read override settings from file
			rpc.readGlobalPrefsOverride();
			// read preferences for GUI to be able to display data
			GlobalPreferences clientPrefs = rpc.getGlobalPrefsWorkingStruct();
			status.setPrefs(clientPrefs);
			// read supported projects
			readAndroidProjectsList();
			// set Android model as hostinfo
			// should output something like "Samsung Galaxy SII - SDK:15 ABI:armeabi-v7a"
			String model = Build.MANUFACTURER + " " + Build.MODEL + " - SDK:" + Build.VERSION.SDK_INT + " ABI: " + Build.CPU_ABI;
			if(Logging.DEBUG) Log.d(Logging.TAG,"reporting hostinfo model name: " + model);
			rpc.setHostInfo(model);
		}
		
		if(connected) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "setup completed successfully"); 
			status.setSetupStatus(ClientStatus.SETUP_STATUS_AVAILABLE,false);
		} else {
			if(Logging.DEBUG) Log.d(Logging.TAG, "onPostExecute - setup experienced an error"); 
			status.setSetupStatus(ClientStatus.SETUP_STATUS_ERROR,true);
		}
		
		try{
			setupWakeLock.release(); // throws exception if it has not been acquired before
		} catch(Exception e){}
		return connected;
	}
	
    // Executes the BOINC client using the Java Runtime exec method.
	//
    private Boolean runClient() {
    	Boolean success = false;
    	try { 
    		String[] cmd = new String[2];
    		
    		cmd[0] = clientPath + clientName;
    		cmd[1] = "--daemon";
    		
        	Runtime.getRuntime().exec(cmd, null, new File(clientPath));
        	success = true;
    	} catch (IOException e) {
    		if(Logging.DEBUG) Log.d(Logging.TAG, "Starting BOINC client failed with exception: " + e.getMessage());
    		if(Logging.ERROR) Log.e(Logging.TAG, "IOException", e);
    	}
    	return success;
    }

	private Boolean connectClient() {
		Boolean success = false;
		
        success = connect();
        if(!success) {
        	if(Logging.DEBUG) Log.d(Logging.TAG, "connection failed!");
        	return success;
        }
        
        //authorize
        success = authorize();
        if(!success) {
        	if(Logging.DEBUG) Log.d(Logging.TAG, "authorization failed!");
        }
        return success;
	}
	
	// Copies the binaries of BOINC client from assets directory into 
	// storage space of this application
	//
    private Boolean installClient(){

		installFile(clientName, true, true);
		installFile(clientCLI, true, true);
		installFile(clientCABundle, true, false);
		installFile(clientConfig, true, false);
		installFile(allProjectsList, true, false);
		installFile(globalOverridePreferences, false, false);
    	
    	return true; 
    }
    
	private Boolean installFile(String file, Boolean override, Boolean executable) {
    	Boolean success = false;
    	byte[] b = new byte [1024];
		int count; 
		
		try {
			if(Logging.DEBUG) Log.d(Logging.TAG, "installing: " + file);
			
    		File target = new File(clientPath + file);
    		
    		// Check path and create it
    		File installDir = new File(clientPath);
    		if(!installDir.exists()) {
    			installDir.mkdir();
    			installDir.setWritable(true); 
    		}
    		
    		if(target.exists()) {
    			if(override) target.delete();
    			else {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"skipped file, exists and ovverride is false");
    				return true;
    			}
    		}
    		
    		// Copy file from the asset manager to clientPath
    		InputStream asset = getApplicationContext().getAssets().open(file); 
    		OutputStream targetData = new FileOutputStream(target); 
    		while((count = asset.read(b)) != -1){ 
    			targetData.write(b, 0, count);
    		}
    		asset.close(); 
    		targetData.flush(); 
    		targetData.close();

    		success = true; //copy succeeded without exception
    		
    		// Set executable, if requested
    		Boolean isExecutable = false;
    		if(executable) {
    			target.setExecutable(executable);
    			isExecutable = target.canExecute();
    			success = isExecutable; // return false, if not executable
    		}

    		if(Logging.DEBUG) Log.d(Logging.TAG, "install of " + file + " successfull. executable: " + executable + "/" + isExecutable);
    		
    	} catch (IOException e) {  
    		if(Logging.ERROR) Log.e(Logging.TAG, "IOException: " + e.getMessage());
    		if(Logging.DEBUG) Log.d(Logging.TAG, "install of " + file + " failed.");
    	}
		
		return success;
	}

    // Connects to running BOINC client.
    //
    private Boolean connect() {
    	return rpc.open("127.0.0.1", 31416);
    }
    
    // Authorizes this application as valid RPC Manager by reading auth token from file 
    // and making RPC call.
    //
    private Boolean authorize() {
    	String authKey = readAuthToken();
		
		//trigger client rpc
		return rpc.authorize(authKey); 
    }
	
    // updates ClientStatus data structure with values received from client via rpc calls.
    private void updateStatus(){
		// check whether RPC client connection is alive
		if(!rpc.connectionAlive()) clientSetup(); // start setup routine
		
    	if(!screenOn && screenOffStatusOmitCounter < deviceStatusIntervalScreenOff) screenOffStatusOmitCounter++; // omit status reporting according to configuration
    	else {
    		// screen is on, or omit counter reached limit
    		reportDeviceStatus();
    		readClientStatus(); // readClientStatus is also required when screen is off, otherwise no wakeLock acquisition.
    	}
    }
    
    // reads client status via rpc calls
    // if screen off, only computing status to adjust wakelocks
    private void readClientStatus() {
    	try{
    		// read ccStatus and adjust wakelocks independently of screen status
    		CcStatus status = rpc.getCcStatus();
    		Boolean computationEnabled = (status.task_suspend_reason == BOINCDefs.SUSPEND_NOT_SUSPENDED);
    		if(Logging.VERBOSE) Log.d(Logging.TAG,"readClientStatus(): computation enabled: " + computationEnabled);
			Monitor.getClientStatus().setWifiLock(computationEnabled);
			Monitor.getClientStatus().setWakeLock(computationEnabled);
    		
			// complete status read, depending on screen status
    		// screen off: only read computing status to adjust wakelock, do not send broadcast
    		// screen on: read complete status, set ClientStatus, send broadcast
	    	if(screenOn) {
	    		// complete status read, with broadcast
				if(Logging.VERBOSE) Log.d(Logging.TAG, "readClientStatus(): screen on, get complete status");
				CcState state = rpc.getState();
				ArrayList<Transfer>  transfers = rpc.getFileTransfers();
				
				if( (status != null) && (state != null) && (state.results != null) && (state.projects != null) && (transfers != null) && (state.host_info != null)) {
					Monitor.getClientStatus().setClientStatus(status, state.results, state.projects, transfers, state.host_info);
					// Update status bar notification
					ClientNotification.getInstance(getApplicationContext()).update();
				} else {
					if(Logging.ERROR) Log.e(Logging.TAG, "readClientStatus(): connection problem");
				}
				
				// check whether monitor is still intended to update, if not, skip broadcast and exit...
				if(updateBroadcastEnabled) {
			        Intent clientStatus = new Intent();
			        clientStatus.setAction("edu.berkeley.boinc.clientstatus");
			        getApplicationContext().sendBroadcast(clientStatus);
				}
	    	} 
		}catch(Exception e) {
			if(Logging.ERROR) Log.e(Logging.TAG, "Monitor.readClientStatus excpetion: " + e.getMessage(),e);
		}
    }
    
    // reports current device status to the client via rpc
    // client uses data to enforce preferences, e.g. suspend on battery
    private void reportDeviceStatus() {
		if(Logging.VERBOSE) Log.d(Logging.TAG, "reportDeviceStatus()");
    	try{
	    	// set devices status
			if(deviceStatus != null) { // make sure deviceStatus is initialized
				deviceStatus.update(); // poll device status
				Boolean reportStatusSuccess = rpc.reportDeviceStatus(deviceStatus); // transmit device status via rpc
				if(reportStatusSuccess) screenOffStatusOmitCounter = 0;
				else if(Logging.DEBUG) Log.d(Logging.TAG,"reporting device status returned false.");
			} else if(Logging.WARNING) Log.w(Logging.TAG,"reporting device status failed, wrapper not initialized.");
		}catch(Exception e) {
			if(Logging.ERROR) Log.e(Logging.TAG, "Monitor.reportDeviceStatus excpetion: " + e.getMessage());
		}
    }
    
    // Compute MD5 of the requested asset
    //
    private String ComputeMD5Asset(String file) {
    	byte[] b = new byte [1024];
		int count; 
		
		try {
			MessageDigest md5 = MessageDigest.getInstance("MD5");

			InputStream asset = getApplicationContext().getAssets().open(file); 
    		while((count = asset.read(b)) != -1){ 
    			md5.update(b, 0, count);
    		}
    		asset.close();
    		
			byte[] md5hash = md5.digest();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < md5hash.length; ++i) {
				sb.append(String.format("%02x", md5hash[i]));
			}
    		
    		return sb.toString();
    	} catch (IOException e) {  
    		if(Logging.ERROR) Log.e(Logging.TAG, "IOException: " + e.getMessage());
    	} catch (NoSuchAlgorithmException e) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "NoSuchAlgorithmException: " + e.getMessage());
		}
		
		return "";
    }

    // Compute MD5 of the requested file
    //
    private String ComputeMD5File(String file) {
    	byte[] b = new byte [1024];
		int count; 
		
		try {
			MessageDigest md5 = MessageDigest.getInstance("MD5");

    		File target = new File(file);
    		InputStream asset = new FileInputStream(target); 
    		while((count = asset.read(b)) != -1){ 
    			md5.update(b, 0, count);
    		}
    		asset.close();

			byte[] md5hash = md5.digest();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < md5hash.length; ++i) {
				sb.append(String.format("%02x", md5hash[i]));
			}
    		
    		return sb.toString();
    	} catch (IOException e) {  
    		if(Logging.ERROR) Log.e(Logging.TAG, "IOException: " + e.getMessage());
    	} catch (NoSuchAlgorithmException e) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "NoSuchAlgorithmException: " + e.getMessage());
		}
		
		return "";
    }
    
	// Get PID for process name using native 'ps' console command
    //
    private Integer getPidForProcessName(String processName) {
    	int count;
    	char[] buf = new char[1024];
    	StringBuffer sb = new StringBuffer();
    	
    	//run ps and read output
    	try {
	    	Process p = Runtime.getRuntime().exec("ps");
	    	p.waitFor();
	    	InputStreamReader isr = new InputStreamReader(p.getInputStream());
	    	while((count = isr.read(buf)) != -1)
	    	{
	    	    sb.append(buf, 0, count);
	    	}
    	} catch (Exception e) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "Exception: " + e.getMessage());
    	}
    	
    	//parse output into hashmap
    	HashMap<String,Integer> pMap = new HashMap<String, Integer>();
    	String [] processLinesAr = sb.toString().split("\n");
    	for(String line : processLinesAr)
    	{
    		Integer pid;
    		String packageName;
    	    String [] comps = line.split("[\\s]+");
    	    if(comps.length != 9) {continue;}     
    	    pid = Integer.parseInt(comps[1]);
    	    packageName = comps[8];
    	    pMap.put(packageName, pid);
    	    //if(Logging.DEBUG) Log.d(Logging.TAG,"added: " + packageName + pid); 
    	}
    	
    	// Find required pid
    	return pMap.get(processName);
    }
    
    // Exit a process with OS signals SIGQUIT and SIGKILL
    //
    private void quitProcessOsLevel(String processName) {
    	Integer clientPid = getPidForProcessName(processName);
    	
    	// client PID could not be read, client already ended / not yet started?
    	if (clientPid == null) {
    		if(Logging.DEBUG) Log.d(Logging.TAG, "quitProcessOsLevel could not find PID, already ended or not yet started?");
    		return;
    	}
    	
    	if(Logging.DEBUG) Log.d(Logging.TAG, "quitProcessOsLevel for " + processName + ", pid: " + clientPid);
    	
    	// Do not just kill the client on the first attempt.  That leaves dangling 
		// science applications running which causes repeated spawning of applications.
		// Neither the UI or client are happy and each are trying to recover from the
		// situation.  Instead send SIGQUIT and give the client time to clean up.
		//
    	android.os.Process.sendSignal(clientPid, android.os.Process.SIGNAL_QUIT);
    	
    	// Wait for the client to shutdown gracefully
    	Integer attempts = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_os_check_attempts);
    	Integer sleepPeriod = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_os_check_rate_ms);
    	for(int x = 0; x < attempts; x++) {
			try {
				Thread.sleep(sleepPeriod);
			} catch (Exception e) {}
    		if(getPidForProcessName(processName) == null) { //client is now closed
        		if(Logging.DEBUG) Log.d(Logging.TAG,"quitClient: gracefull SIGQUIT shutdown successful after " + x + " seconds");
    			x = attempts;
    		}
    	}
    	
    	clientPid = getPidForProcessName(processName);
    	if(clientPid != null) {
    		// Process is still alive, send SIGKILL
    		if(Logging.WARNING) Log.w(Logging.TAG, "SIGQUIT failed. SIGKILL pid: " + clientPid);
    		android.os.Process.killProcess(clientPid);
    	}
    	
    	clientPid = getPidForProcessName(processName);
    	if(clientPid != null) {
    		if(Logging.WARNING) Log.w(Logging.TAG, "SIGKILL failed. still living pid: " + clientPid);
    	}
    }
	
    // reads all_project_list.xml from Client and filters
 	// projects not supporting Android. List does not change
    // during run-time. Called once during setup.
    // Stored in ClientStatus.
	public void readAndroidProjectsList() {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"Monitor.readAndroidProjectList: Could not load data, clientStatus not initialized.");
			return;
		}
		
		ArrayList<ProjectInfo> allProjects = rpc.getAllProjectsList();
		ArrayList<ProjectInfo> androidProjects = new ArrayList<ProjectInfo>();
		
		if(allProjects == null) return;
		
		//filter projects that do not support Android
		for (ProjectInfo project: allProjects) {
			if(project.platforms.contains(getString(R.string.boinc_platform_name))) {
				if(Logging.DEBUG) Log.d(Logging.TAG, project.name + " supports " + getString(R.string.boinc_platform_name));
				androidProjects.add(project);
			} 
		}
		
		// set list in ClientStatus
		status.setSupportedProjects(androidProjects);
	}
	
	public static ClientStatus getClientStatus() throws Exception{ //singleton pattern
		if (clientStatus == null) {
			// client status needs application context, but context might not be available
			// in static code. functions have to deal with Exception!
			if(Logging.WARNING) Log.w(Logging.TAG,"getClientStatus: clientStatus not yet initialized");
			throw new Exception("clientStatus not initialized");
		}
		return clientStatus;
	}
	
	public static AppPreferences getAppPrefs() { //singleton pattern
		if (appPrefs == null) {
			appPrefs = new AppPreferences();
		}
		return appPrefs;
	}

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
    	if(Logging.DEBUG) Log.d(Logging.TAG,"Monitor onBind");
        return mBinder;
    }
	
    /*
     * onCreate is life-cycle method of service. regardless of bound or started service, this method gets called once upon first creation.
     */
	@Override
    public void onCreate() {
		if(Logging.DEBUG) Log.d(Logging.TAG,"Monitor onCreate()");
		
		// populate attributes with XML resource values
		clientPath = getString(R.string.client_path); 
		clientName = getString(R.string.client_name); 
		clientCLI = getString(R.string.client_cli); 
		clientCABundle = getString(R.string.client_cabundle); 
		clientConfig = getString(R.string.client_config); 
		authFileName = getString(R.string.auth_file_name); 
		allProjectsList = getString(R.string.all_projects_list); 
		globalOverridePreferences = getString(R.string.global_prefs_override);
		clientStatusInterval = getResources().getInteger(R.integer.status_update_interval_ms);
		deviceStatusIntervalScreenOff = getResources().getInteger(R.integer.device_status_update_screen_off_every_X_loop);
		
		// initialize singleton helper classes and provide application context
		clientStatus = new ClientStatus(this);
		getAppPrefs().readPrefs(this);
		
		// set current screen on/off status
		PowerManager pm = (PowerManager)
		getSystemService(Context.POWER_SERVICE);
		screenOn = pm.isScreenOn();
		
		// initialize DeviceStatus wrapper
		deviceStatus = new DeviceStatus(getApplicationContext());
		
		// register screen on/off receiver
        IntentFilter onFilter = new IntentFilter (Intent.ACTION_SCREEN_ON); 
        IntentFilter offFilter = new IntentFilter (Intent.ACTION_SCREEN_OFF); 
        registerReceiver(screenOnOffReceiver, onFilter);
        registerReceiver(screenOnOffReceiver, offFilter);
		
        // register and start update task
        // using .scheduleAtFixedRate() can cause a series of bunched-up runs
        // when previous executions are delayed (e.g. during clientSetup() )
        updateTimer.schedule(statusUpdateTask, 0, clientStatusInterval);
	}
	
    @Override
    public void onDestroy() {
    	if(Logging.DEBUG) Log.d(Logging.TAG,"Monitor onDestroy()");
    	
    	// remove screen on/off receiver
    	unregisterReceiver(screenOnOffReceiver);
    	
        // Cancel the persistent notification.
    	((NotificationManager)getSystemService(Service.NOTIFICATION_SERVICE)).cancel(getResources().getInteger(R.integer.autostart_notification_id));
        
    	updateBroadcastEnabled = false; // prevent broadcast from currently running update task
		updateTimer.cancel(); // cancel task
		
		 // release locks, if held.
		clientStatus.setWakeLock(false);
		clientStatus.setWifiLock(false);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {	
    	//this gets called after startService(intent) (either by BootReceiver or AndroidBOINCActivity, depending on the user's autostart configuration)
    	if(Logging.DEBUG) Log.d(Logging.TAG, "Monitor onStartCommand()");
		/*
		 * START_STICKY causes service to stay in memory until stopSelf() is called, even if all
		 * Activities get destroyed by the system. Important for GUI keep-alive
		 * For detailed service documentation see
		 * http://android-developers.blogspot.com.au/2010/02/service-api-changes-starting-with.html
		 */
		return START_STICKY;
    }
    
    // schedule manual status update, without any delay
    // will fire clientstatuschange Broadcast updon completion
    public void forceRefresh() {
    	if(Logging.DEBUG) Log.d(Logging.TAG,"forceRefresh()");
        updateTimer.schedule(new StatusUpdateTimerTask(), 0);
    }
    
    // exits both, UI and BOINC client. 
    // BLOCKING! call from AsyncTask!
    public void quitClient() {
		// try to get current client status from monitor
		ClientStatus status = null;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"Monitor.quitClient: Could not load data, clientStatus not initialized.");
			// do not return here, try to shut down without publishing status
		}
    	String processName = clientPath + clientName;
    	
    	updateBroadcastEnabled = false; // prevent broadcast from currently running update task
		updateTimer.cancel(); // cancel task
    	// no scheduled RPCs anymore
    	
    	// set client status to SETUP_STATUS_CLOSING to adapt layout accordingly
		if(status!=null)status.setSetupStatus(ClientStatus.SETUP_STATUS_CLOSING,true);
    	
    	// try graceful shutdown via RPC
    	rpc.quit();
    	
    	// there might be still other AsyncTasks executing RPCs
    	// close sockets in a synchronized way
    	rpc.close();
    	// there are now no more RPCs...
    	
    	// graceful RPC shutdown waiting period...
    	Boolean success = false;
    	Integer attempts = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_attempts);
    	Integer sleepPeriod = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_rate_ms);
    	for(int x = 0; x < attempts; x++) {
    		try {
    			Thread.sleep(sleepPeriod);
    		} catch (Exception e) {}
    		if(getPidForProcessName(processName) == null) { //client is now closed
        		if(Logging.DEBUG) Log.d(Logging.TAG,"quitClient: gracefull RPC shutdown successful after " + x + " seconds");
    			success = true;
    			x = attempts;
    		}
    	}
    	
    	if(!success) {
    		// graceful RPC shutdown was not successful, try OS signals
        	quitProcessOsLevel(processName);
    	}
    	
    	// cancel notification
		ClientNotification.getInstance(getApplicationContext()).cancel();
    	
    	// set client status to SETUP_STATUS_CLOSED to adapt layout accordingly
		if(status!=null)status.setSetupStatus(ClientStatus.SETUP_STATUS_CLOSED,true);
		
		//stop service, triggers onDestroy
		stopSelf();
    }
       
	public Boolean setRunMode(Integer mode) {
		return rpc.setRunMode(mode, 0);
	}
	
	public Boolean setNetworkMode(Integer mode) {
		return rpc.setNetworkMode(mode, 0);
	}
	
	// writes the given GlobalPreferences via RPC to the client
	// after writing, the active preferences are read back and
	// written to ClientStatus.
	public Boolean setGlobalPreferences(GlobalPreferences prefs) {

		// try to get current client status from monitor
		ClientStatus status = null;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"Monitor.setGlobalPreferences: Could not load data, clientStatus not initialized.");
			return false;
		}

		Boolean retval1 = rpc.setGlobalPrefsOverrideStruct(prefs); //set new override settings
		Boolean retval2 = rpc.readGlobalPrefsOverride(); //trigger reload of override settings
		if(!retval1 || !retval2) {
			return false;
		}
		GlobalPreferences workingPrefs = rpc.getGlobalPrefsWorkingStruct();
		if(workingPrefs != null){
			status.setPrefs(workingPrefs);
			return true;
		}
		return false;
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
    		if(Logging.ERROR) Log.e(Logging.TAG, "auth file not found",fnfe);
    	}
    	catch (IOException ioe) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "ioexception",ioe);
    	}

		String authKey = fileData.toString();
		if(Logging.DEBUG) Log.d(Logging.TAG, "authentication key acquired. length: " + authKey.length());
		return authKey;
	}
	
	public ProjectConfig getProjectConfig(String url) {
		ProjectConfig config = null;
		
    	Boolean success = rpc.getProjectConfig(url); //asynchronous call
    	if(success) { //only continue if attach command did not fail
    		// verify success of getProjectConfig with poll function
    		Integer counter = 0;
    		Integer sleepDuration = 500; //in milliseconds
    		Integer maxLoops = maxDuration / sleepDuration;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(sleepDuration);
    			} catch (Exception e) {}
    			counter ++;
    			config = rpc.getProjectConfigPoll();
    			if(config==null) {
    				return null;
    			}
    			if (config.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			} else {
    				//final result ready
    				if(config.error_num == 0) { 
        				if(Logging.DEBUG) Log.d(Logging.TAG, "ProjectConfig retrieved: " + config.name);
    				} else {
    					if(Logging.DEBUG) Log.d(Logging.TAG, "final result with error_num: " + config.error_num);
    				}
    			}
    		}
    	}
		return config;
	}
	
	public Boolean attachProject(String url, String projectName, String authenticator) {
    	Boolean success = false;
    	success = rpc.projectAttach(url, authenticator, projectName); //asynchronous call to attach project
    	if(success) { //only continue if attach command did not fail
    		// verify success of projectAttach with poll function
    		success = false;
    		Integer counter = 0;
    		Integer sleepDuration = 500; //in milliseconds
    		Integer maxLoops = maxDuration / sleepDuration;
    		while(!success && (counter < maxLoops)) {
    			try {
    				Thread.sleep(sleepDuration);
    			} catch (Exception e) {}
    			counter ++;
    			ProjectAttachReply reply = rpc.projectAttachPoll();
    			if(reply != null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.projectAttachPoll reply error_num: " + reply.error_num);
    				if(reply.error_num == 0) success = true;
    			}
    		}
    	} else if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.projectAttach failed.");
    	return success;
    }
	
	public Boolean checkProjectAttached(String url) {
		Boolean match = false;
		try{
			ArrayList<Project> attachedProjects = rpc.getProjectStatus();
			for (Project project: attachedProjects) {
				if(Logging.DEBUG) Log.d(Logging.TAG, project.master_url + " vs " + url);
				if(project.master_url.equals(url)) {
					match = true;
					continue;
				}
			}
		} catch(Exception e){}
		return match;
	}
	
	public AccountOut lookupCredentials(String url, String id, String pwd, Boolean usesName) {
    	AccountOut auth = null;
    	AccountIn credentials = new AccountIn();
    	if(usesName) credentials.user_name = id;
    	else credentials.email_addr = id;
    	credentials.passwd = pwd;
    	credentials.url = url;
    	Boolean success = rpc.lookupAccount(credentials); //asynch
    	if(success) { //only continue if lookupAccount command did not fail
    		//get authentication token from lookupAccountPoll
    		Integer counter = 0;
    		Integer sleepDuration = 500; //in milliseconds
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
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.lookupAccountPoll.");
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(auth.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "credentials verification result, retrieved authenticator.");
    				else Log.d(Logging.TAG, "credentials verification result, error: " + auth.error_num);
    			}
    		}
    	} else if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.lookupAccount failed.");
    	return auth;
    }
	
	// sets cc_config.xml for client and trigger is to re-read.
	public void setCcConfig(String ccConfig) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "Monitor.setCcConfig: current cc_config: " + rpc.getCcConfig());
		if(Logging.DEBUG) Log.d(Logging.TAG, "Monitor.setCcConfig: setting new cc_config: " + ccConfig);
		rpc.setCcConfig(ccConfig);
		rpc.readCcConfig();
	}
    
	public Boolean abortTransfer(String url, String name){
		return rpc.transferOp(RpcClient.TRANSFER_ABORT, url, name);
	}
	
	public void abortTransferAsync(String url, String name){
		if(Logging.DEBUG) Log.d(Logging.TAG, "abortTransferAsync");
		String[] param = new String[2];
		param[0] = url;
		param[1] = name;
		(new TransferAbortAsync()).execute(param);
	}
    
	public Boolean projectOperation(int operation, String url){
		return rpc.projectOp(operation, url);
	}
    
	public Boolean retryTransfer(String url, String name){
		return rpc.transferOp(RpcClient.TRANSFER_RETRY, url, name);
	}
	
	public void retryTransferAsync(String url, String name){
		if(Logging.DEBUG) Log.d(Logging.TAG, "retryTransferAsync");
		String[] param = new String[2];
		param[0] = url;
		param[1] = name;
		(new TransferRetryAsync()).execute(param);
	}

	// executes specified operation on result
	// e.g. RpcClient.RESULT_SUSPEND, RpcClient.RESULT_RESUME, RpcClient.RESULT_ABORT
	public Boolean resultOperation(String url, String name, int operation) {
		return rpc.resultOp(operation, url, name);
	}
	
	public Boolean transferOperation(String url, String name, int operation) {
		return rpc.transferOp(operation, url, name);
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
    		Integer sleepDuration = 500; //in milliseconds
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
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.createAccountPoll.");
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(auth.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "account creation result, retrieved authenticator.");
    				else if(Logging.DEBUG) Log.d(Logging.TAG, "account creation result, error: " + auth.error_num);
    			}
    		}
    	} else {if(Logging.DEBUG) Log.d(Logging.TAG,"rpc.createAccount returned false.");}
    	return auth;
	}
	
	// returns given number of client messages, older than provided seqNo
	// if seqNo <= 0 initial data retrieval
	public ArrayList<Message> getEventLogMessages(int seqNo, int number) {
		// determine oldest message seqNo for data retrieval
		int lowerBound = 0;
		if(seqNo > 0) lowerBound = seqNo - number - 2;
		else lowerBound = rpc.getMessageCount() - number - 1; // can result in >number results, if client writes message btwn. here and rpc.getMessages!
		
		// less than desired number of messsages available, adapt lower bound
		if(lowerBound < 0) lowerBound = 0;
		ArrayList<Message> msgs= rpc.getMessages(lowerBound); // returns ever messages with seqNo > lowerBound
		
		if(seqNo > 0) {
			// remove messages that are >= seqNo
			Iterator<Message> it = msgs.iterator();
			while(it.hasNext()) {
				Message tmp = it.next();
				if (tmp.seqno >= seqNo) it.remove();
			}
		}
		
		if(!msgs.isEmpty()) 
			if(Logging.DEBUG) Log.d(Logging.TAG,"getEventLogMessages: returning array with " + msgs.size() + " entries. for lowerBound: " + lowerBound + " at 0: " + msgs.get(0).seqno + " at " + (msgs.size()-1) + ": " + msgs.get(msgs.size()-1).seqno);
		else 
			if(Logging.DEBUG) Log.d(Logging.TAG,"getEventLogMessages: returning empty array for lowerBound: " + lowerBound);
		return msgs;
	}
	
	// returns client messages that are more recent than given seqNo
	public ArrayList<Message> getEventLogMessages(int seqNo) {
		//if(Logging.DEBUG) Log.d(Logging.TAG, "getEventLogMessage more recent than seqNo: " + seqNo);
		return rpc.getMessages(seqNo);
	}
	
	// updates the client status via rpc
	// reports current device status to the client via rpc
	//
	// get executed in seperate thread
	private final class StatusUpdateTimerTask extends TimerTask {
		@Override
		public void run() {
			updateStatus();
		}
	}

	private final class TransferAbortAsync extends AsyncTask<String,String,Boolean> {
		
		private String url;
		private String name;
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.url = params[0];
			this.name = params[0];
			publishProgress("doInBackground() - TransferAbortAsync url: " + url + " Name: " + name);
			
			Boolean abort = rpc.transferOp(RpcClient.TRANSFER_ABORT, url, name);
			if(abort) {
				if(Logging.DEBUG) Log.d(Logging.TAG, "successful.");
			}
			return abort;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			forceRefresh();
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "onProgressUpdate - " + arg0[0]);
		}
	}

	private final class TransferRetryAsync extends AsyncTask<String,String,Boolean> {
		
		private String url;
		private String name;
		
		@Override
		protected Boolean doInBackground(String... params) {
			this.url = params[0];
			this.name = params[1];
			publishProgress("doInBackground() - TransferRetryAsync url: " + url + " Name: " + name);
			
			Boolean retry = rpc.transferOp(RpcClient.TRANSFER_RETRY, url, name);
			if(retry) {
				publishProgress("successful.");
			}
			return retry;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			forceRefresh();
		}

		@Override
		protected void onProgressUpdate(String... arg0) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "onProgressUpdate - " + arg0[0]);
		}
	}
	
	// broadcast receiver to detect changes to screen on or off
	// used to adapt ClientMonitorAsync bahavior
	// e.g. avoid polling GUI status rpcs while screen is off to
	// save battery
	BroadcastReceiver screenOnOffReceiver = new BroadcastReceiver() { 
		@Override 
        public void onReceive(Context context, Intent intent) { 
			String action = intent.getAction();
			if(action.equals(Intent.ACTION_SCREEN_OFF)) {
				screenOn = false;
				if(Logging.DEBUG) Log.d(Logging.TAG, "screenOnOffReceiver: screen turned off");
			}
			if(action.equals(Intent.ACTION_SCREEN_ON)) {
				screenOn = true;
				if(Logging.DEBUG) Log.d(Logging.TAG, "screenOnOffReceiver: screen turned on, force data refresh...");
				forceRefresh();
			}
        } 
 }; 

}
