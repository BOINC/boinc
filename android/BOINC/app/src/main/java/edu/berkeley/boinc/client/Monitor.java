/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2019 University of California
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
 */
package edu.berkeley.boinc.client;

import edu.berkeley.boinc.rpc.AccountManager;
import edu.berkeley.boinc.utils.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.RemoteException;
import android.util.Log;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.mutex.BoincMutex;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AcctMgrRPCReply;
import edu.berkeley.boinc.rpc.CcState;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import edu.berkeley.boinc.rpc.ImageWrapper;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.rpc.AcctMgrInfo;

/**
 * Main Service of BOINC on Android
 * - manages life-cycle of the BOINC Client.
 * - frequently polls the latest status of the client (e.g. running tasks, attached projects etc)
 * - reports device status (e.g. battery level, connected to charger etc) to the client
 * - holds singleton of client status data model and applications persistent preferences
 */
public class Monitor extends Service {

    private static BoincMutex mutex = new BoincMutex(); // holds the BOINC mutex, only compute if acquired
    private static ClientStatus clientStatus; //holds the status of the client as determined by the Monitor
    private static AppPreferences appPrefs; //hold the status of the app, controlled by AppPreferences
    private static DeviceStatus deviceStatus; // holds the status of the device, i.e. status information that can only be obtained trough Java APIs

    public ClientInterfaceImplementation clientInterface = new ClientInterfaceImplementation(); //provides functions for interaction with client via rpc

    // XML defined variables, populated in onCreate
    private String fileNameClient;
    private String fileNameCLI;
    private String fileNameCABundle;
    private String fileNameClientConfig;
    private String fileNameGuiAuthentication;
    private String fileNameAllProjectsList;
    private String fileNameNoMedia;
    private String boincWorkingDir;
    private Integer clientStatusInterval;
    private Integer deviceStatusIntervalScreenOff;
    private String clientSocketAddress;

    private Timer updateTimer = new Timer(true); // schedules frequent client status update
    private TimerTask statusUpdateTask = new StatusUpdateTimerTask();
    private boolean updateBroadcastEnabled = false;
    private Integer screenOffStatusOmitCounter = 0;

    // screen on/off updated by screenOnOffBroadcastReceiver
    private boolean screenOn = false;

    private boolean forceReinstall = false; // for debugging purposes //TODO

    @Override
    public IBinder onBind(Intent intent) {
        if (Logging.DEBUG) Log.d(Logging.TAG, "Monitor onBind");
        return mBinder;
    }

    @Override
    public void onCreate() {

        Log.d(Logging.TAG, "Monitor onCreate()");

        // populate attributes with XML resource values
        boincWorkingDir = getString(R.string.client_path);
        fileNameClient = getString(R.string.client_name);
        fileNameCLI = getString(R.string.client_cli);
        fileNameCABundle = getString(R.string.client_cabundle);
        fileNameClientConfig = getString(R.string.client_config);
        fileNameGuiAuthentication = getString(R.string.auth_file_name);
        fileNameAllProjectsList = getString(R.string.all_projects_list);
        fileNameNoMedia = getString(R.string.nomedia);
        clientStatusInterval = getResources().getInteger(R.integer.status_update_interval_ms);
        deviceStatusIntervalScreenOff = getResources().getInteger(R.integer.device_status_update_screen_off_every_X_loop);
        clientSocketAddress = getString(R.string.client_socket_address);

        // initialize singleton helper classes and provide application context
        clientStatus = new ClientStatus(this);
        getAppPrefs().readPrefs(this);
        deviceStatus = new DeviceStatus(this, getAppPrefs());
        if (Logging.ERROR) Log.d(Logging.TAG, "Monitor onCreate(): singletons initialized");

        // set current screen on/off status
        PowerManager pm = (PowerManager)
                getSystemService(Context.POWER_SERVICE);
        screenOn = pm.isScreenOn();

        // initialize DeviceStatus wrapper
        deviceStatus = new DeviceStatus(getApplicationContext(), getAppPrefs());

        // register screen on/off receiver
        IntentFilter onFilter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        IntentFilter offFilter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        registerReceiver(screenOnOffReceiver, onFilter);
        registerReceiver(screenOnOffReceiver, offFilter);
    }

    @Override
    public void onDestroy() {
        if (Logging.ERROR) Log.d(Logging.TAG, "Monitor onDestroy()");

        updateBroadcastEnabled = false; // prevent broadcast from currently running update task
        updateTimer.cancel(); // cancel task

        // there might be still other AsyncTasks executing RPCs
        // close sockets in a synchronized way
        clientInterface.close();

        try {
            // remove screen on/off receiver
            unregisterReceiver(screenOnOffReceiver);
        } catch (Exception e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "Monitor.onDestroy error: ", e);
        }

        updateBroadcastEnabled = false; // prevent broadcast from currently running update task
        updateTimer.cancel(); // cancel task

        mutex.release(); // release BOINC mutex

        // release locks, if held.
        try {
            clientStatus.setWakeLock(false);
            clientStatus.setWifiLock(false);
        } catch (Exception e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "Monitor.onDestroy error: ", e);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        //this gets called after startService(intent) (either by BootReceiver or SplashActivity, depending on the user's autostart configuration)
        if (Logging.ERROR) Log.d(Logging.TAG, "Monitor onStartCommand()");

        // try to acquire BOINC mutex
        // run here in order to recover, if mutex holding app gets closed.
        if (!updateBroadcastEnabled && mutex.acquire()) {
            updateBroadcastEnabled = true;
            // register and start update task
            // using .scheduleAtFixedRate() can cause a series of bunched-up runs
            // when previous executions are delayed (e.g. during clientSetup() )
            updateTimer.schedule(statusUpdateTask, 0, clientStatusInterval);
        }
        if (!mutex.acquired) if (Logging.ERROR)
            Log.e(Logging.TAG, "Monitor.onStartCommand: mutex acquisition failed, do not start BOINC.");

        // execute action if one is explicitly requested (e.g. from notification)
        if (intent != null) {
            int actionCode = intent.getIntExtra("action", -1);
            if (Logging.DEBUG)
                Log.d(Logging.TAG, "Monitor.onStartCommand() with action code: " + actionCode);
            switch (actionCode) {
                case 1: // suspend
                    new SetClientRunModeAsync().execute(BOINCDefs.RUN_MODE_NEVER);
                    break;
                case 2: // resume
                    new SetClientRunModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
                    break;
            }
        }

        /*
         * START_STICKY causes service to stay in memory until stopSelf() is called, even if all
         * Activities get destroyed by the system. Important for GUI keep-alive
         * For detailed service documentation see
         * http://android-developers.blogspot.com.au/2010/02/service-api-changes-starting-with.html
         */
        return START_STICKY;
    }
// --end-- attributes and methods related to Android Service life-cycle

// singleton getter

    /**
     * Retrieve singleton of ClientStatus.
     *
     * @return ClientStatus, represents the data model of the BOINC client's status
     * @throws Exception if client status has not been initialized
     */
    public static ClientStatus getClientStatus() throws Exception { //singleton pattern
        if (clientStatus == null) {
            // client status needs application context, but context might not be available
            // in static code. functions have to deal with Exception!
            if (Logging.WARNING)
                Log.w(Logging.TAG, "getClientStatus: clientStatus not yet initialized");
            throw new Exception("clientStatus not initialized");
        }
        return clientStatus;
    }

    /**
     * Retrieve singleton of AppPreferences.
     *
     * @return AppPreferences, interface to Android applications persistent key-value store
     */
    public static AppPreferences getAppPrefs() { //singleton pattern
        if (appPrefs == null) {
            appPrefs = new AppPreferences();
        }
        return appPrefs;
    }

    /**
     * Retrieve singleton of DeviceStatus.
     *
     * @return DeviceStatus, represents data model of device information reported to the client
     * @throws Exception if deviceStatus hast not been initialized
     */
    public static DeviceStatus getDeviceStatus() throws Exception {//singleton pattern
        if (deviceStatus == null) {
            // device status needs application context, but context might not be available
            // in static code. functions have to deal with Exception!
            if (Logging.WARNING)
                Log.w(Logging.TAG, "getDeviceStatus: deviceStatus not yet initialized");
            throw new Exception("deviceStatus not initialized");
        }
        return deviceStatus;
    }
// --end-- singleton getter

// public methods for Activities

    /**
     * Indicates whether service was able to obtain BOINC mutex.
     * If not, BOINC has not started and all other calls will fail.
     *
     * @return BOINC mutex acquisition successful
     */
    public boolean boincMutexAcquired() {
        return mutex.acquired;
    }

    /**
     * Force refresh of client status data model, will fire Broadcast upon success.
     */
    public void forceRefresh() {
        if (!mutex.acquired) return; // do not try to update if client is not running
        if (Logging.DEBUG) Log.d(Logging.TAG, "forceRefresh()");
        try {
            updateTimer.schedule(new StatusUpdateTimerTask(), 0);
        } catch (Exception e) {
            if (Logging.WARNING) Log.w(Logging.TAG, "Monitor.forceRefresh error: ", e);
        } // throws IllegalStateException if called after timer got cancelled, i.e. after manual shutdown
    }

    /**
     * Determines BOINC platform name corresponding to device's cpu architecture (ARM, x86).
     * Defaults to ARM
     *
     * @return ID of BOINC platform name string in resources
     */
    public int getBoincPlatform() {
        int platformId;
        String arch = System.getProperty("os.arch");
        String normalizedArch = arch.toUpperCase(Locale.US);
        if (normalizedArch.contains("AARCH64")) platformId = R.string.boinc_platform_name_arm64;
        else if (normalizedArch.contains("ARM64")) platformId = R.string.boinc_platform_name_arm64;
        else if (normalizedArch.contains("X86_64"))
            platformId = R.string.boinc_platform_name_x86_64;
        else if (normalizedArch.contains("ARM")) platformId = R.string.boinc_platform_name_arm;
        else if (normalizedArch.contains("86")) platformId = R.string.boinc_platform_name_x86;
        else {
            if (Logging.ERROR)
                Log.w(Logging.TAG, "could not map os.arch (" + arch + ") to platform, default to arm.");
            platformId = R.string.boinc_platform_name_arm;
        }

        if (Logging.ERROR)
            Log.d(Logging.TAG, "BOINC platform: " + getString(platformId) + " for os.arch: " + arch);
        return platformId;
    }

    /**
     * Determines BOINC alt platform name corresponding to device's cpu architecture (ARM, x86).
     *
     * @return BOINC platform name string in resources
     */
    public String getBoincAltPlatform() {
        String platformName = "";
        String arch = System.getProperty("os.arch");
        String normalizedArch = arch.toUpperCase(Locale.US);
        if (normalizedArch.contains("AARCH64"))
            platformName = getString(R.string.boinc_platform_name_arm);
        else if (normalizedArch.contains("ARM64"))
            platformName = getString(R.string.boinc_platform_name_arm);
        else if (normalizedArch.contains("X86_64"))
            platformName = getString(R.string.boinc_platform_name_x86);

        if (Logging.ERROR)
            Log.d(Logging.TAG, "BOINC Alt platform: " + platformName + " for os.arch: " + arch);
        return platformName;
    }

    /**
     * Returns path to file in BOINC's working directory that contains GUI authentication key
     *
     * @return absolute path to file holding GUI authentication key
     */
    public String getAuthFilePath() {
        return boincWorkingDir + fileNameGuiAuthentication;
    }
// --end-- public methods for Activities

// multi-threaded frequent information polling

    /**
     * Task to frequently and asynchronously poll the client's status. Executed in different thread.
     */
    private final class StatusUpdateTimerTask extends TimerTask {
        @Override
        public void run() {
            updateStatus();
        }
    }

    /**
     * Reports current device status to client and reads current client status.
     * Updates ClientStatus and fires Broadcast.
     * Called frequently to poll current status.
     */
    private void updateStatus() {
        // check whether RPC client connection is alive
        if (!clientInterface.connectionAlive()) {
            if (clientSetup()) { // start setup routine
                // interact with client only if connection established successfully
                reportDeviceStatus();
                readClientStatus(true); // read initial data
            }
        }

        if (!screenOn && screenOffStatusOmitCounter < deviceStatusIntervalScreenOff)
            screenOffStatusOmitCounter++; // omit status reporting according to configuration
        else {
            // screen is on, or omit counter reached limit
            if (clientInterface.connectionAlive()) {
                reportDeviceStatus();
                readClientStatus(false); // readClientStatus is also required when screen is off, otherwise no wakeLock acquisition.
            }
        }
    }

    /**
     * Reads client status via RPCs
     * Optimized to retrieve only subset of information (required to determine wakelock state) if screen is turned off
     *
     * @param forceCompleteUpdate forces update of entire status information, regardless of screen status
     */
    private void readClientStatus(Boolean forceCompleteUpdate) {
        try {
            CcStatus status; // read independently of screen status

            // complete status read, depending on screen status
            // screen off: only read computing status to adjust wakelock, do not send broadcast
            // screen on: read complete status, set ClientStatus, send broadcast
            // forceCompleteUpdate: read complete status, independently of screen setting
            if (screenOn || forceCompleteUpdate) {
                // complete status read, with broadcast
                if (Logging.VERBOSE)
                    Log.d(Logging.TAG, "readClientStatus(): screen on, get complete status");
                status = clientInterface.getCcStatus();
                CcState state = clientInterface.getState();
                ArrayList<Transfer> transfers = clientInterface.getFileTransfers();
                AcctMgrInfo acctMgrInfo = clientInterface.getAcctMgrInfo();
                ArrayList<Notice> newNotices = clientInterface.getNotices(Monitor.getClientStatus().getMostRecentNoticeSeqNo());

                if ((status != null) && (state != null) && (state.results != null) && (state.projects != null) && (transfers != null) && (state.host_info != null) && (acctMgrInfo != null)) {
                    Monitor.getClientStatus().setClientStatus(status, state.results, state.projects, transfers, state.host_info, acctMgrInfo, newNotices);
                } else {
                    String nullValues = "";

                    if (state == null) {
                        nullValues += "state ";
                    } else {
                        if (state.results == null) nullValues += "state.results ";
                        if (state.projects == null) nullValues += "state.projects ";
                        if (state.host_info == null) nullValues += "state.host_info ";
                    }
                    if (transfers == null) nullValues += "transfers ";
                    if (acctMgrInfo == null) nullValues += "acctMgrInfo ";

                    if (Logging.ERROR)
                        Log.e(Logging.TAG, "readClientStatus(): connection problem, null: " + nullValues);
                }

                // update notices notification
                NoticeNotification.getInstance(getApplicationContext()).update(Monitor.getClientStatus().getRssNotices(), Monitor.getAppPrefs().getShowNotificationForNotices());

                // check whether monitor is still intended to update, if not, skip broadcast and exit...
                if (updateBroadcastEnabled) {
                    Intent clientStatus = new Intent();
                    clientStatus.setAction("edu.berkeley.boinc.clientstatus");
                    getApplicationContext().sendBroadcast(clientStatus);
                }
            } else {
                // read only ccStatus to adjust wakelocks and service state independently of screen status
                status = clientInterface.getCcStatus();
            }

            // independent of screen on off:
            // wake locks and foreground enabled when Client is not suspended, therefore also during
            // idle.
            // treat cpu throttling as if it was computing.
            Boolean computing = (status.task_suspend_reason == BOINCDefs.SUSPEND_NOT_SUSPENDED) || (status.task_suspend_reason == BOINCDefs.SUSPEND_REASON_CPU_THROTTLE);
            if (Logging.VERBOSE)
                Log.d(Logging.TAG, "readClientStatus(): computation enabled: " + computing);
            Monitor.getClientStatus().setWifiLock(computing);
            Monitor.getClientStatus().setWakeLock(computing);
            ClientNotification.getInstance(getApplicationContext()).update(Monitor.getClientStatus(), this, computing);

        } catch (Exception e) {
            if (Logging.ERROR)
                Log.e(Logging.TAG, "Monitor.readClientStatus excpetion: " + e.getMessage(), e);
        }
    }

    // reports current device status to the client via rpc
    // client uses data to enforce preferences, e.g. suspend on battery

    /**
     * Reports current device status to the client via RPC
     * BOINC client uses this data to enforce preferences, e.g. suspend battery but requires information only/best available through Java API calls.
     */
    private void reportDeviceStatus() {
        if (Logging.VERBOSE) Log.d(Logging.TAG, "reportDeviceStatus()");
        try {
            // set devices status
            if (deviceStatus != null) { // make sure deviceStatus is initialized
                Boolean reportStatusSuccess = clientInterface.reportDeviceStatus(deviceStatus.update(screenOn)); // transmit device status via rpc
                if (reportStatusSuccess) screenOffStatusOmitCounter = 0;
                else if (Logging.DEBUG)
                    Log.d(Logging.TAG, "reporting device status returned false.");
            } else if (Logging.WARNING)
                Log.w(Logging.TAG, "reporting device status failed, wrapper not initialized.");
        } catch (Exception e) {
            if (Logging.ERROR)
                Log.e(Logging.TAG, "Monitor.reportDeviceStatus excpetion: " + e.getMessage());
        }
    }
// --end-- multi-threaded frequent information polling

// BOINC client installation and run-time management

    /**
     * installs client binaries(if changed) and other required files
     * executes client process
     * triggers initial reads (e.g. preferences, project list etc)
     *
     * @return Boolean whether connection established successfully
     */
    private Boolean clientSetup() {
        if (Logging.ERROR) Log.d(Logging.TAG, "Monitor.clientSetup()");

        // try to get current client status from monitor
        ClientStatus status;
        try {
            status = Monitor.getClientStatus();
        } catch (Exception e) {
            if (Logging.WARNING)
                Log.w(Logging.TAG, "Monitor.clientSetup: Could not load data, clientStatus not initialized.");
            return false;
        }

        status.setSetupStatus(ClientStatus.SETUP_STATUS_LAUNCHING, true);
        String clientProcessName = boincWorkingDir + fileNameClient;

        String md5AssetClient = computeMd5(fileNameClient, true);
        //if(Logging.DEBUG) Log.d(Logging.TAG, "Hash of client (Asset): '" + md5AssetClient + "'");

        String md5InstalledClient = computeMd5(clientProcessName, false);
        //if(Logging.DEBUG) Log.d(Logging.TAG, "Hash of client (File): '" + md5InstalledClient + "'");

        // If client hashes do not match, we need to install the one that is a part
        // of the package. Shutdown the currently running client if needed.
        //
        if (forceReinstall || !md5InstalledClient.equals(md5AssetClient)) {
            if (Logging.DEBUG)
                Log.d(Logging.TAG, "Hashes of installed client does not match binary in assets - re-install.");

            // try graceful shutdown using RPC (faster)
            if (getPidForProcessName(clientProcessName) != null) {
                if (connectClient()) {
                    clientInterface.quit();
                    Integer attempts = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_attempts);
                    Integer sleepPeriod = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_rpc_check_rate_ms);
                    for (int x = 0; x < attempts; x++) {
                        try {
                            Thread.sleep(sleepPeriod);
                        } catch (Exception ignored) {
                        }
                        if (getPidForProcessName(clientProcessName) == null) { //client is now closed
                            if (Logging.DEBUG)
                                Log.d(Logging.TAG, "quitClient: gracefull RPC shutdown successful after " + x + " seconds");
                            x = attempts;
                        }
                    }
                }
            }

            // quit with OS signals
            if (getPidForProcessName(clientProcessName) != null) {
                quitProcessOsLevel(clientProcessName);
            }

            // at this point client is definitely not running. install new binary...
            if (!installClient()) {
                if (Logging.ERROR) Log.w(Logging.TAG, "BOINC client installation failed!");
                return false;
            }
        }

        // Start the BOINC client if we need to.
        //
        Integer clientPid = getPidForProcessName(clientProcessName);
        if (clientPid == null) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Starting the BOINC client");
            if (!runClient()) {
                if (Logging.ERROR) Log.d(Logging.TAG, "BOINC client failed to start");
                return false;
            }
        }

        // Try to connect to executed Client in loop
        //
        Integer retryRate = getResources().getInteger(R.integer.monitor_setup_connection_retry_rate_ms);
        Integer retryAttempts = getResources().getInteger(R.integer.monitor_setup_connection_retry_attempts);
        Boolean connected = false;
        Integer counter = 0;
        while (!connected && (counter < retryAttempts)) {
            if (Logging.DEBUG) Log.d(Logging.TAG, "Attempting BOINC client connection...");
            connected = connectClient();
            counter++;

            try {
                Thread.sleep(retryRate);
            } catch (Exception ignored) {
            }
        }

        Boolean init = false;
        if (connected) { // connection established
            try {
                // read preferences for GUI to be able to display data
                GlobalPreferences clientPrefs = clientInterface.getGlobalPrefsWorkingStruct();
                if (clientPrefs == null) throw new Exception("client prefs null");
                status.setPrefs(clientPrefs);

                // set Android model as hostinfo
                // should output something like "Samsung Galaxy SII - SDK:15 ABI:armeabi-v7a"
                String model = Build.MANUFACTURER + " " + Build.MODEL + " - SDK:" + Build.VERSION.SDK_INT + " ABI: " + Build.CPU_ABI;
                String version = Build.VERSION.RELEASE;
                if (Logging.ERROR) Log.d(Logging.TAG, "reporting hostinfo model name: " + model);
                if (Logging.ERROR) Log.d(Logging.TAG, "reporting hostinfo os name: Android");
                if (Logging.ERROR) Log.d(Logging.TAG, "reporting hostinfo os version: " + version);
                clientInterface.setHostInfo(model, version);

                init = true;
            } catch (Exception e) {
                if (Logging.ERROR)
                    Log.e(Logging.TAG, "Monitor.clientSetup() init failed: " + e.getMessage());
            }
        }

        if (init) {
            if (Logging.ERROR)
                Log.d(Logging.TAG, "Monitor.clientSetup() - setup completed successfully");
            status.setSetupStatus(ClientStatus.SETUP_STATUS_AVAILABLE, false);
        } else {
            if (Logging.ERROR)
                Log.e(Logging.TAG, "Monitor.clientSetup() - setup experienced an error");
            status.setSetupStatus(ClientStatus.SETUP_STATUS_ERROR, true);
        }

        return connected;
    }

    /**
     * Executes BOINC client.
     * Using Java Runtime exec method
     *
     * @return Boolean success
     */
    private Boolean runClient() {
        Boolean success = false;
        try {
            String[] cmd = new String[3];

            cmd[0] = boincWorkingDir + fileNameClient;
            cmd[1] = "--daemon";
            cmd[2] = "--gui_rpc_unix_domain";

            if (Logging.ERROR)
                Log.w(Logging.TAG, "Launching '" + cmd[0] + "' from '" + boincWorkingDir + "'");
            Runtime.getRuntime().exec(cmd, null, new File(boincWorkingDir));
            success = true;
        } catch (IOException e) {
            if (Logging.ERROR)
                Log.d(Logging.TAG, "Starting BOINC client failed with exception: " + e.getMessage());
            if (Logging.ERROR) Log.e(Logging.TAG, "IOException", e);
        }
        return success;
    }

    /**
     * Establishes connection to client and handles initial authentication
     *
     * @return Boolean success
     */
    private Boolean connectClient() {
        Boolean success = clientInterface.open(clientSocketAddress);
        if (!success) {
            if (Logging.ERROR) Log.e(Logging.TAG, "connection failed!");
            return success;
        }

        //authorize
        success = clientInterface.authorizeGuiFromFile(boincWorkingDir + fileNameGuiAuthentication);
        if (!success) {
            if (Logging.ERROR) Log.e(Logging.TAG, "authorization failed!");
        }
        return success;
    }

    /**
     * Installs required files from APK's asset directory to the applications' internal storage.
     * File attributes override and executable are defined here
     *
     * @return Boolean success
     */
    private Boolean installClient() {

        if (!installFile(fileNameClient, true, true, "")) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameClient);
            return false;
        }
        if (!installFile(fileNameCLI, true, true, "")) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameCLI);
            return false;
        }
        if (!installFile(fileNameCABundle, true, false, "")) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameCABundle);
            return false;
        }
        if (!installFile(fileNameClientConfig, true, false, "")) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameClientConfig);
            return false;
        }
        if (!installFile(fileNameAllProjectsList, true, false, "")) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameAllProjectsList);
            return false;
        }
        if (!installFile(fileNameNoMedia, false, false, "." + fileNameNoMedia)) {
            if (Logging.ERROR) Log.d(Logging.TAG, "Failed to install: " + fileNameNoMedia);
            return false;
        }

        return true;
    }

    /**
     * Copies given file from APK assets to internal storage.
     *
     * @param file       name of file as it appears in assets directory
     * @param override   define override, if already present in internal storage
     * @param executable set executable flag of file in internal storage
     * @param targetFile name of target file 
     * @return Boolean success
     */
    private Boolean installFile(String file, Boolean override, Boolean executable, String targetFile) {
        Boolean success = false;
        byte[] b = new byte[1024];
        int count;

        // If file is executable, cpu architecture has to be evaluated
        // and assets directory select accordingly
        String source;
        if (executable) source = getAssestsDirForCpuArchitecture() + file;
        else source = file;

        try {
            if (Logging.ERROR) Log.d(Logging.TAG, "installing: " + source);

            File target;
            if (!targetFile.isEmpty()) {
                target = new File(boincWorkingDir + targetFile);
            } else {
                target = new File(boincWorkingDir + file);
            }

            // Check path and create it
            File installDir = new File(boincWorkingDir);
            if (!installDir.exists()) {
                if (!installDir.mkdir()) {
                    if (Logging.ERROR)
                        Log.d(Logging.TAG, "Monitor.installFile(): mkdir() was not successful.");
                }

                if (!installDir.setWritable(true)) {
                    if (Logging.ERROR)
                        Log.d(Logging.TAG, "Monitor.installFile(): setWritable() was not successful.");
                }
            }

            if (target.exists()) {
                if (override) {
                    if (!target.delete()) {
                        if (Logging.ERROR)
                            Log.d(Logging.TAG, "Monitor.installFile(): delete() was not successful.");
                    }
                } else {
                    if (Logging.DEBUG)
                        Log.d(Logging.TAG, "skipped file, exists and ovverride is false");
                    return true;
                }
            }

            // Copy file from the asset manager to clientPath
            InputStream asset = getApplicationContext().getAssets().open(source);
            OutputStream targetData = new FileOutputStream(target);
            while ((count = asset.read(b)) != -1) {
                targetData.write(b, 0, count);
            }
            asset.close();
            targetData.close();

            success = true; //copy succeeded without exception

            // Set executable, if requested
            if (executable) {
                success = target.setExecutable(executable); // return false, if not executable
            }

            if (Logging.ERROR)
                Log.d(Logging.TAG, "install of " + source + " successfull. executable: " + executable + "/" + success);

        } catch (IOException e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "IOException: " + e.getMessage());
            if (Logging.ERROR) Log.d(Logging.TAG, "install of " + source + " failed.");
        }

        return success;
    }

    /**
     * Determines assets directory (contains BOINC client binaries) corresponding to device's cpu architecture (ARM, x86)
     *
     * @return name of assets directory for given platform, not an absolute path.
     */
    private String getAssestsDirForCpuArchitecture() {
        String archAssetsDirectory = "";
        switch (getBoincPlatform()) {
            case R.string.boinc_platform_name_arm:
                archAssetsDirectory = getString(R.string.assets_dir_arm);
                break;
            case R.string.boinc_platform_name_arm64:
                archAssetsDirectory = getString(R.string.assets_dir_arm64);
                break;
            case R.string.boinc_platform_name_x86:
                archAssetsDirectory = getString(R.string.assets_dir_x86);
                break;
            case R.string.boinc_platform_name_x86_64:
                archAssetsDirectory = getString(R.string.assets_dir_x86_64);
                break;
        }
        return archAssetsDirectory;
    }

    /**
     * Computes MD5 hash of requested file
     *
     * @param fileName absolute path or name of file in assets directory, see inAssets parameter
     * @param inAssets if true, fileName is file name in assets directory, if not, absolute path
     * @return md5 hash of file
     */
    private String computeMd5(String fileName, Boolean inAssets) {
        byte[] b = new byte[1024];
        int count;

        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");

            InputStream fs;
            if (inAssets)
                fs = getApplicationContext().getAssets().open(getAssestsDirForCpuArchitecture() + fileName);
            else fs = new FileInputStream(new File(fileName));

            while ((count = fs.read(b)) != -1) {
                md5.update(b, 0, count);
            }
            fs.close();

            byte[] md5hash = md5.digest();
            StringBuilder sb = new StringBuilder();
            for (byte singleMd5hash : md5hash) {
                sb.append(String.format("%02x", singleMd5hash));
            }

            return sb.toString();
        } catch (IOException e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "IOException: " + e.getMessage());
        } catch (NoSuchAlgorithmException e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "NoSuchAlgorithmException: " + e.getMessage());
        }

        return "";
    }

    /**
     * Determines ProcessID corresponding to given process name
     *
     * @param processName name of process, according to output of "ps"
     * @return process id, according to output of "ps"
     */
    private Integer getPidForProcessName(String processName) {
        int count;
        char[] buf = new char[1024];
        StringBuilder sb = new StringBuilder();

        //run ps and read output
        try {
            Process p = Runtime.getRuntime().exec("ps");
            p.waitFor();
            InputStreamReader isr = new InputStreamReader(p.getInputStream());
            while ((count = isr.read(buf)) != -1) {
                sb.append(buf, 0, count);
            }
        } catch (Exception e) {
            if (Logging.ERROR) Log.e(Logging.TAG, "Exception: " + e.getMessage());
            return null;
        }

        String[] processLinesAr = sb.toString().split("\n");
        if (processLinesAr.length < 2) {
            if (Logging.ERROR)
                Log.e(Logging.TAG, "getPidForProcessName(): ps output has less than 2 lines, failure!");
            return null;
        }

        // figure out what index PID has
        String[] headers = processLinesAr[0].split("[\\s]+");
        Integer PidIndex = -1;
        for (int x = 0; x < headers.length; x++) {
            if (headers[x].equals("PID")) {
                PidIndex = x;
                break;
            }
        }

        if (PidIndex == -1) {
            return null;
        }

        if (Logging.DEBUG)
            Log.d(Logging.TAG, "getPidForProcessName(): PID at index: " + PidIndex + " for output: " + processLinesAr[0]);

        Integer pid = null;
        for (int y = 1; y < processLinesAr.length; y++) {
            Boolean found = false;
            String[] comps = processLinesAr[y].split("[\\s]+");
            for (String arg : comps) {
                if (arg.equals(processName)) {
                    if (Logging.DEBUG)
                        Log.d(Logging.TAG, "getPidForProcessName(): " + processName + " found in line: " + y);
                    found = true;
                    break; // Break out of inner foreach (comps) loop
                }
            }
            if (found) {
                try {
                    pid = Integer.parseInt(comps[PidIndex]);
                    if (Logging.ERROR) Log.d(Logging.TAG, "getPidForProcessName(): pid: " + pid);
                } catch (NumberFormatException e) {
                    if (Logging.ERROR)
                        Log.e(Logging.TAG, "getPidForProcessName(): NumberFormatException for " + comps[PidIndex] + " at index: " + PidIndex);
                }
                break;// Break out of outer for (processLinesAr) loop
            }
        }
        // if not happen in ps output, not running?!
        if (pid == null)
            if (Logging.ERROR)
                Log.d(Logging.TAG, "getPidForProcessName(): " + processName + " not found in ps output!");

        // Find required pid
        return pid;
    }

    /**
     * Exits a process by sending it Linux SIGQUIT and SIGKILL signals
     *
     * @param processName name of process to be killed, according to output of "ps"
     */
    private void quitProcessOsLevel(String processName) {
        Integer clientPid = getPidForProcessName(processName);

        // client PID could not be read, client already ended / not yet started?
        if (clientPid == null) {
            if (Logging.ERROR)
                Log.d(Logging.TAG, "quitProcessOsLevel could not find PID, already ended or not yet started?");
            return;
        }

        if (Logging.DEBUG)
            Log.d(Logging.TAG, "quitProcessOsLevel for " + processName + ", pid: " + clientPid);

        // Do not just kill the client on the first attempt.  That leaves dangling
        // science applications running which causes repeated spawning of applications.
        // Neither the UI or client are happy and each are trying to recover from the
        // situation.  Instead send SIGQUIT and give the client time to clean up.
        //
        android.os.Process.sendSignal(clientPid, android.os.Process.SIGNAL_QUIT);

        // Wait for the client to shutdown gracefully
        Integer attempts = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_os_check_attempts);
        Integer sleepPeriod = getApplicationContext().getResources().getInteger(R.integer.shutdown_graceful_os_check_rate_ms);
        for (int x = 0; x < attempts; x++) {
            try {
                Thread.sleep(sleepPeriod);
            } catch (Exception ignored) {
            }
            if (getPidForProcessName(processName) == null) { //client is now closed
                if (Logging.DEBUG)
                    Log.d(Logging.TAG, "quitClient: gracefull SIGQUIT shutdown successful after " + x + " seconds");
                x = attempts;
            }
        }

        clientPid = getPidForProcessName(processName);
        if (clientPid != null) {
            // Process is still alive, send SIGKILL
            if (Logging.ERROR) Log.w(Logging.TAG, "SIGQUIT failed. SIGKILL pid: " + clientPid);
            android.os.Process.killProcess(clientPid);
        }

        clientPid = getPidForProcessName(processName);
        if (clientPid != null) {
            if (Logging.ERROR) Log.w(Logging.TAG, "SIGKILL failed. still living pid: " + clientPid);
        }
    }
// --end-- BOINC client installation and run-time management

// broadcast receiver
    /**
     * broadcast receiver to detect changes to screen on or off, used to adapt scheduling of StatusUpdateTimerTask
     * e.g. avoid polling GUI status RPCs while screen is off in order to save battery
     */
    BroadcastReceiver screenOnOffReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                screenOn = false;
                // forces report of device status at next scheduled update
                // allows timely reaction to screen off for resume of computation
                screenOffStatusOmitCounter = deviceStatusIntervalScreenOff;
                if (Logging.DEBUG) Log.d(Logging.TAG, "screenOnOffReceiver: screen turned off");
            }
            if (action.equals(Intent.ACTION_SCREEN_ON)) {
                screenOn = true;
                if (Logging.DEBUG)
                    Log.d(Logging.TAG, "screenOnOffReceiver: screen turned on, force data refresh...");
                forceRefresh();
            }
        }
    };
// --end-- broadcast receiver

    // async tasks
    private final class SetClientRunModeAsync extends AsyncTask<Integer, Void, Void> {
        @Override
        protected Void doInBackground(Integer... params) {
            try {
                mBinder.setRunMode(params[0]);
            } catch (RemoteException e) {
                if (Logging.ERROR)
                    Log.e(Logging.TAG, "Monitor.SetClientRunModeAsync: doInBackground() error: ", e);
            }
            return null;
        }
    }
// --end -- async tasks

    // remote service
    private final IMonitor.Stub mBinder = new IMonitor.Stub() {

        @Override
        public boolean transferOperation(List<Transfer> list, int op) throws RemoteException {
            return clientInterface.transferOperation((ArrayList<Transfer>) list, op);
        }

        @Override
        public boolean synchronizeAcctMgr(String url) throws RemoteException {
            return clientInterface.synchronizeAcctMgr(url);
        }

        @Override
        public boolean setRunMode(int mode) throws RemoteException {
            return clientInterface.setRunMode(mode);
        }

        @Override
        public boolean setNetworkMode(int mode) throws RemoteException {
            return clientInterface.setNetworkMode(mode);
        }

        @Override
        public boolean setGlobalPreferences(GlobalPreferences pref)
                throws RemoteException {
            return clientInterface.setGlobalPreferences(pref);
        }

        @Override
        public boolean setCcConfig(String config) throws RemoteException {
            return clientInterface.setCcConfig(config);
        }

        @Override
        public boolean setDomainName(String deviceName) throws RemoteException {
            return clientInterface.setDomainName(deviceName);
        }

        @Override
        public boolean resultOp(int op, String url, String name)
                throws RemoteException {
            return clientInterface.resultOp(op, url, name);
        }

        @Override
        public String readAuthToken(String path) throws RemoteException {
            return clientInterface.readAuthToken(path);
        }

        @Override
        public boolean projectOp(int status, String url) throws RemoteException {
            return clientInterface.projectOp(status, url);
        }

        @Override
        public int getBoincPlatform() throws RemoteException {
            return Monitor.this.getBoincPlatform();
        }

        @Override
        public AccountOut lookupCredentials(AccountIn credentials) throws RemoteException {
            return clientInterface.lookupCredentials(credentials);
        }

        @Override
        public boolean isStationaryDeviceSuspected() throws RemoteException {
            try {
                return Monitor.getDeviceStatus().isStationaryDeviceSuspected();
            } catch (Exception e) {
                if (Logging.ERROR)
                    Log.e(Logging.TAG, "Monitor.IMonitor.Stub: isStationaryDeviceSuspected() error: ", e);
            }
            return false;
        }

        @Override
        public List<Notice> getServerNotices() throws RemoteException {
            return clientStatus.getServerNotices();
        }

        @Override
        public ProjectConfig getProjectConfigPolling(String url)
                throws RemoteException {
            return clientInterface.getProjectConfigPolling(url);
        }

        @Override
        public List<Notice> getNotices(int seq) throws RemoteException {
            return clientInterface.getNotices(seq);
        }

        @Override
        public List<edu.berkeley.boinc.rpc.Message> getMessages(int seq) throws RemoteException {
            return clientInterface.getMessages(seq);
        }

        @Override
        public List<edu.berkeley.boinc.rpc.Message> getEventLogMessages(int seq, int num)
                throws RemoteException {
            return clientInterface.getEventLogMessages(seq, num);
        }

        @Override
        public int getBatteryChargeStatus() throws RemoteException {
            try {
                return getDeviceStatus().getStatus().battery_charge_pct;
            } catch (Exception e) {
                if (Logging.ERROR)
                    Log.e(Logging.TAG, "Monitor.IMonitor.Stub: getBatteryChargeStatus() error: ", e);
            }
            return 0;
        }

        @Override
        public AcctMgrInfo getAcctMgrInfo() throws RemoteException {
            return clientInterface.getAcctMgrInfo();
        }

        @Override
        public void forceRefresh() throws RemoteException {
            Monitor.this.forceRefresh();
        }

        @Override
        public AccountOut createAccountPolling(AccountIn information) throws RemoteException {
            return clientInterface.createAccountPolling(information);
        }

        @Override
        public boolean checkProjectAttached(String url) throws RemoteException {
            return clientInterface.checkProjectAttached(url);
        }

        @Override
        public boolean attachProject(String url, String projectName, String authenticator)
                throws RemoteException {
            return clientInterface.attachProject(url, projectName, authenticator);
        }

        @Override
        public ErrorCodeDescription addAcctMgrErrorNum(String url, String userName, String pwd)
                throws RemoteException {
            AcctMgrRPCReply acctMgr = clientInterface.addAcctMgr(url, userName, pwd);
            if (acctMgr != null) {
                return new ErrorCodeDescription(acctMgr.error_num, acctMgr.messages.isEmpty() ? "" : acctMgr.messages.toString());
            }
            return new ErrorCodeDescription(-1);
        }

        @Override
        public String getAuthFilePath() throws RemoteException {
            return Monitor.this.getAuthFilePath();
        }

        @Override
        public List<ProjectInfo> getAttachableProjects() throws RemoteException {
            return clientInterface.getAttachableProjects(getString(getBoincPlatform()), getBoincAltPlatform());
        }

        @Override
        public List<AccountManager> getAccountManagers() throws RemoteException {
            return clientInterface.getAccountManagers();
        }

        @Override
        public boolean getAcctMgrInfoPresent() throws RemoteException {
            return clientStatus.getAcctMgrInfo().present;
        }

        @Override
        public int getSetupStatus() throws RemoteException {
            return clientStatus.setupStatus;
        }

        @Override
        public int getComputingStatus() throws RemoteException {
            return clientStatus.computingStatus;
        }

        @Override
        public int getComputingSuspendReason() throws RemoteException {
            return clientStatus.computingSuspendReason;
        }

        @Override
        public int getNetworkSuspendReason() throws RemoteException {
            return clientStatus.networkSuspendReason;
        }

        @Override
        public HostInfo getHostInfo() throws RemoteException {
            return clientStatus.getHostInfo();
        }

        @Override
        public GlobalPreferences getPrefs() throws RemoteException {
            return clientStatus.getPrefs();
        }

        @Override
        public List<Project> getProjects() throws RemoteException {
            return clientStatus.getProjects();
        }

        @Override
        public AcctMgrInfo getClientAcctMgrInfo() throws RemoteException {
            return clientStatus.getAcctMgrInfo();
        }

        @Override
        public List<Transfer> getTransfers() throws RemoteException {
            return clientStatus.getTransfers();
        }

        @Override
        public void setAutostart(boolean isAutoStart) throws RemoteException {
            Monitor.getAppPrefs().setAutostart(isAutoStart);
        }

        @Override
        public void setShowNotificationForNotices(boolean isShow) throws RemoteException {
            Monitor.getAppPrefs().setShowNotificationForNotices(isShow);
        }

        @Override
        public boolean getShowAdvanced() throws RemoteException {
            return Monitor.getAppPrefs().getShowAdvanced();
        }

        @Override
        public boolean getAutostart() throws RemoteException {
            return Monitor.getAppPrefs().getAutostart();
        }

        @Override
        public boolean getShowNotificationForNotices() throws RemoteException {
            return Monitor.getAppPrefs().getShowNotificationForNotices();
        }

        @Override
        public int getLogLevel() throws RemoteException {
            return Monitor.getAppPrefs().getLogLevel();
        }

        @Override
        public void setLogLevel(int level) throws RemoteException {
            Monitor.getAppPrefs().setLogLevel(level);
        }

        @Override
        public void setPowerSourceAc(boolean src) throws RemoteException {
            Monitor.getAppPrefs().setPowerSourceAc(src);
        }

        @Override
        public void setPowerSourceUsb(boolean src) throws RemoteException {
            Monitor.getAppPrefs().setPowerSourceUsb(src);
        }

        @Override
        public void setPowerSourceWireless(boolean src) throws RemoteException {
            Monitor.getAppPrefs().setPowerSourceWireless(src);
        }

        @Override
        public List<Result> getTasks() throws RemoteException {
            return clientStatus.getTasks();
        }

        @Override
        public String getProjectStatus(String url) throws RemoteException {
            return clientStatus.getProjectStatus(url);
        }

        @Override
        public List<Notice> getRssNotices() throws RemoteException {
            return clientStatus.getRssNotices();
        }

        @Override
        public List<ImageWrapper> getSlideshowForProject(String url)
                throws RemoteException {
            return clientStatus.getSlideshowForProject(url);
        }

        @Override
        public boolean getStationaryDeviceMode() throws RemoteException {
            return Monitor.getAppPrefs().getStationaryDeviceMode();
        }

        @Override
        public boolean getPowerSourceAc() throws RemoteException {
            return Monitor.getAppPrefs().getPowerSourceAc();
        }

        @Override
        public boolean getPowerSourceUsb() throws RemoteException {
            return Monitor.getAppPrefs().getPowerSourceUsb();
        }

        @Override
        public boolean getPowerSourceWireless() throws RemoteException {
            return Monitor.getAppPrefs().getPowerSourceWireless();
        }

        @Override
        public void setShowAdvanced(boolean isShow) throws RemoteException {
            Monitor.getAppPrefs().setShowAdvanced(isShow);
        }

        @Override
        public void setStationaryDeviceMode(boolean mode)
                throws RemoteException {
            Monitor.getAppPrefs().setStationaryDeviceMode(mode);

        }

        @Override
        public Bitmap getProjectIconByName(String name) throws RemoteException {
            return clientStatus.getProjectIconByName(name);
        }

        @Override
        public Bitmap getProjectIcon(String id) throws RemoteException {
            return clientStatus.getProjectIcon(id);
        }

        @Override
        public boolean getSuspendWhenScreenOn() throws RemoteException {
            return Monitor.getAppPrefs().getSuspendWhenScreenOn();
        }

        @Override
        public void setSuspendWhenScreenOn(boolean swso) throws RemoteException {
            Monitor.getAppPrefs().setSuspendWhenScreenOn(swso);
        }

        @Override
        public String getCurrentStatusTitle() throws RemoteException {
            return clientStatus.getCurrentStatusTitle();
        }

        @Override
        public String getCurrentStatusDescription() throws RemoteException {
            return clientStatus.getCurrentStatusDescription();
        }

        @Override
        public void cancelNoticeNotification() throws RemoteException {
            NoticeNotification.getInstance(getApplicationContext()).cancelNotification();
        }

        @Override
        public boolean runBenchmarks() throws RemoteException {
            return clientInterface.runBenchmarks();
        }

        @Override
        public ProjectInfo getProjectInfo(String url) throws RemoteException {
            return clientInterface.getProjectInfo(url);
        }

        @Override
        public boolean boincMutexAcquired() throws RemoteException {
            return mutex.acquired;
        }
    };
// --end-- remote service
}