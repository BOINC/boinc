/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.client

import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.graphics.Bitmap
import android.os.*
import androidx.core.content.getSystemService
import androidx.lifecycle.LifecycleService
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.BOINCApplication
import edu.berkeley.boinc.BuildConfig
import edu.berkeley.boinc.R
import edu.berkeley.boinc.mutex.BoincMutex
import edu.berkeley.boinc.rpc.*
import edu.berkeley.boinc.utils.*
import java.io.File
import java.io.IOException
import java.io.InputStreamReader
import java.util.*
import javax.inject.Inject
import kotlin.properties.Delegates
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import okio.buffer
import okio.source

typealias Message = edu.berkeley.boinc.rpc.Message

/**
 * Main Service of BOINC on Android
 * - manages life-cycle of the BOINC Client.
 * - frequently polls the latest status of the client (e.g. running tasks, attached projects etc)
 * - reports device status (e.g. battery level, connected to charger etc) to the client
 * - holds singleton of client status data model and applications persistent preferences
 */
class Monitor : LifecycleService() {
    private val abi = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
        @Suppress("DEPRECATION")
        Build.CPU_ABI
    } else {
        Build.SUPPORTED_ABIS[0]
    }

    //hold the status of the app, controlled by AppPreferences
    @Inject
    lateinit var appPreferences: AppPreferences

    // holds the BOINC mutex, only compute if acquired
    @Inject
    lateinit var mutex: BoincMutex

    //provides functions for interaction with client via RPC
    @Inject
    lateinit var clientInterface: ClientInterfaceImplementation

    @Inject
    lateinit var clientNotification: ClientNotification

    //holds the status of the client as determined by the Monitor
    @Inject
    lateinit var clientStatus: ClientStatus

    // holds the status of the device, i.e. status information that can only be obtained trough Java APIs
    @Inject
    lateinit var deviceStatus: DeviceStatus

    @Inject
    lateinit var noticeNotification: NoticeNotification

    // XML defined variables, populated in onCreate
    private lateinit var fileNameClient: String
    private lateinit var fileNameCABundle: String
    private lateinit var fileNameClientConfig: String
    private lateinit var fileNameGuiAuthentication: String
    private lateinit var fileNameAllProjectsList: String
    private lateinit var fileNameNoMedia: String
    private lateinit var boincWorkingDir: String
    private lateinit var clientSocketAddress: String

    private var clientStatusInterval by Delegates.notNull<Int>()
    private var deviceStatusIntervalScreenOff: Int = 0
    private val updateTimer = Timer(true) // schedules frequent client status update
    private val statusUpdateTask: TimerTask = StatusUpdateTimerTask()
    private var updateBroadcastEnabled = false
    private var screenOffStatusOmitCounter = 0

    // screen on/off updated by screenOnOffBroadcastReceiver
    private var screenOn = false
    private val forceReinstall = false // for debugging purposes //TODO

    private var isRemote = false

    /**
     * Determines BOINC platform name corresponding to device's cpu architecture (ARM, x86).
     * Defaults to ARM
     *
     * @return ID of BOINC platform name string in resources
     */
    val boincPlatform: Int
        get() {
            val platformId: Int
            val arch = System.getProperty("os.arch") ?: ""
            val normalizedArch = arch.uppercase(Locale.US)
            platformId = when {
                normalizedArch.containsAny("ARM64", "AARCH64") -> R.string.boinc_platform_name_arm64
                "X86_64" in normalizedArch -> R.string.boinc_platform_name_x86_64
                "ARMV6" in normalizedArch -> R.string.boinc_platform_name_armv6
                "ARM" in normalizedArch -> R.string.boinc_platform_name_arm
                "86" in normalizedArch -> R.string.boinc_platform_name_x86
                else -> {
                    Logging.logWarning(
                        Logging.Category.MONITOR,
                        "could not map os.arch ($arch) to platform, default to arm."
                    )

                    R.string.boinc_platform_name_arm
                }
            }

            Logging.logInfo(
                Logging.Category.MONITOR,
                "BOINC platform: ${getString(platformId)} for os.arch: $arch"
            )

            return platformId
        }

    /**
     * Determines BOINC alt platform name corresponding to device's cpu architecture (ARM, x86).
     *
     * @return BOINC platform name string in resources
     */
    val boincAltPlatform: String
        get() {
            var platformName = ""
            val arch = System.getProperty("os.arch") ?: ""
            val normalizedArch = arch.uppercase(Locale.US)
            if (normalizedArch.containsAny("ARM64", "AARCH64", "ARMV6"))
                platformName = getString(R.string.boinc_platform_name_arm)
            else if ("X86_64" in normalizedArch)
                platformName = getString(R.string.boinc_platform_name_x86)

            Logging.logInfo(
                Logging.Category.MONITOR,
                "BOINC Alt platform: $platformName for os.arch: $arch"
            )

            return platformName
        }

    /**
     * Returns path to file in BOINC's working directory that contains GUI authentication key
     *
     * @return absolute path to file holding GUI authentication key
     */
    val authFilePath: String
        get() = boincWorkingDir + fileNameGuiAuthentication

    override fun onBind(intent: Intent): IBinder? {
        super.onBind(intent)

        Logging.logDebug(Logging.Category.MONITOR, "Monitor onBind")

        return mBinder
    }

    override fun onCreate() {
        (application as BOINCApplication).appComponent.inject(this)
        super.onCreate()

        // Read User log level and set logLevel of Logging Class
        Logging.setLogLevel(appPreferences.logLevel)
        Logging.setLogCategories(appPreferences.logCategories)

        Logging.logDebug(Logging.Category.MONITOR, "Monitor onCreate()")

        // populate attributes with XML resource values
        boincWorkingDir = applicationInfo.dataDir + "/" + getString(R.string.client_path)
        fileNameClient = getString(R.string.client_name)
        fileNameCABundle = getString(R.string.client_cabundle)
        fileNameClientConfig = getString(R.string.client_config)
        fileNameGuiAuthentication = getString(R.string.auth_file_name)
        fileNameAllProjectsList = getString(R.string.all_projects_list)
        fileNameNoMedia = getString(R.string.nomedia)
        clientStatusInterval = resources.getInteger(R.integer.status_update_interval_ms)
        deviceStatusIntervalScreenOff =
            resources.getInteger(R.integer.device_status_update_screen_off_every_X_loop)
        clientSocketAddress = getString(R.string.client_socket_address)

        Logging.logDebug(Logging.Category.MONITOR, "Monitor onCreate(): singletons initialized")

        // set current screen on/off status
        screenOn = getSystemService<PowerManager>()!!.isScreenOnCompat

        // register screen on/off receiver
        val onFilter = IntentFilter(Intent.ACTION_SCREEN_ON)
        val offFilter = IntentFilter(Intent.ACTION_SCREEN_OFF)
        registerReceiver(screenOnOffReceiver, onFilter)
        registerReceiver(screenOnOffReceiver, offFilter)
    }

    override fun onDestroy() {
        super.onDestroy()

        Logging.logDebug(Logging.Category.MONITOR, "Monitor onDestroy()")

        updateBroadcastEnabled = false // prevent broadcast from currently running update task
        updateTimer.cancel() // cancel task

        // there might be still other AsyncTasks executing RPCs
        // close sockets in a synchronized way
        clientInterface.close()
        try {
            // remove screen on/off receiver
            unregisterReceiver(screenOnOffReceiver)
        } catch (e: Exception) {
            Logging.logException(Logging.Category.MONITOR, "Monitor.onDestroy error: ", e)
        }
        updateBroadcastEnabled = false // prevent broadcast from currently running update task
        updateTimer.cancel() // cancel task
        mutex.release() // release BOINC mutex

        // release locks, if held.
        try {
            clientStatus.setWakeLock(false)
            clientStatus.setWifiLock(false)
        } catch (e: Exception) {
            Logging.logException(Logging.Category.MONITOR, "Monitor.onDestroy error: ", e)
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        super.onStartCommand(intent, flags, startId)
        //this gets called after startService(intent) (either by BootReceiver or SplashActivity, depending on the user's autostart configuration)
        Logging.logDebug(Logging.Category.MONITOR, "Monitor onStartCommand()")

        // try to acquire BOINC mutex
        // run here in order to recover, if mutex holding app gets closed.
        if (!updateBroadcastEnabled && mutex.acquire()) {
            updateBroadcastEnabled = true
            // register and start update task
            // using .scheduleAtFixedRate() can cause a series of bunched-up runs
            // when previous executions are delayed (e.g. during clientSetup() )
            updateTimer.schedule(statusUpdateTask, 0, clientStatusInterval.toLong())
        }
        if (!mutex.isAcquired) Logging.logError(
            Logging.Category.MONITOR,
            "Monitor.onStartCommand: mutex acquisition failed, do not start BOINC."
        )

        // execute action if one is explicitly requested (e.g. from notification)
        if (intent != null) {
            val actionCode = intent.getIntExtra("action", -1)

            Logging.logDebug(
                Logging.Category.MONITOR,
                "Monitor.onStartCommand() with action code: $actionCode"
            )

            when (actionCode) {
                1 -> lifecycleScope.launch { setClientRunMode(RUN_MODE_NEVER) }
                2 -> lifecycleScope.launch { setClientRunMode(RUN_MODE_AUTO) }
            }
        }

        /*
         * START_STICKY causes service to stay in memory until stopSelf() is called, even if all
         * Activities get destroyed by the system. Important for GUI keep-alive
         * For detailed service documentation see
         * http://android-developers.blogspot.com.au/2010/02/service-api-changes-starting-with.html
         */
        return Service.START_STICKY
    }
    // --end-- attributes and methods related to Android Service life-cycle
    // public methods for Activities
    /**
     * Force refresh of client status data model, will fire Broadcast upon success.
     */
    fun forceRefresh() {
        if (!mutex.isAcquired) return  // do not try to update if client is not running

        Logging.logDebug(Logging.Category.MONITOR, "forceRefresh()")

        try {
            updateTimer.schedule(StatusUpdateTimerTask(), 0)
        } catch (e: Exception) {
            Logging.logException(Logging.Category.MONITOR, "Monitor.forceRefresh error: ", e)
        } // throws IllegalStateException if called after timer got cancelled, i.e. after manual shutdown
    }

    //Kill boinc client nicely
    fun quitClient(): Boolean {
        if (clientInterface.isConnected) {
            return clientInterface.quit()
        }
        return true
    }
    // --end-- public methods for Activities
    // multi-threaded frequent information polling
    /**
     * Task to frequently and asynchronously poll the client's status. Executed in different thread.
     */
    private inner class StatusUpdateTimerTask : TimerTask() {
        override fun run() {
            updateStatus()
        }
    }

    /**
     * Reports current device status to client and reads current client status.
     * Updates ClientStatus and fires Broadcast.
     * Called frequently to poll current status.
     */
    private fun updateStatus() {
        // check whether RPC client connection is alive
        if (!clientInterface.connectionAlive() && clientSetup()) { // start setup routine
            // interact with client only if connection established successfully
            reportDeviceStatus()
            readClientStatus(true) // read initial data
        }
        if (!screenOn && screenOffStatusOmitCounter < deviceStatusIntervalScreenOff)
            screenOffStatusOmitCounter++ // omit status reporting according to configuration
        else {
            // screen is on, or omit counter reached limit
            if (clientInterface.connectionAlive()) {
                reportDeviceStatus()
                readClientStatus(false) // readClientStatus is also required when screen is off, otherwise no wakeLock acquisition.
            }
        }
    }

    /**
     * Reads client status via RPCs
     * Optimized to retrieve only subset of information (required to determine wakelock state) if screen is turned off
     *
     * @param forceCompleteUpdate forces update of entire status information, regardless of screen status
     */
    private fun readClientStatus(forceCompleteUpdate: Boolean) {
        try {
            val status: CcStatus? // read independently of screen status

            // complete status read, depending on screen status
            // screen off: only read computing status to adjust wakelock, do not send broadcast
            // screen on: read complete status, set ClientStatus, send broadcast
            // forceCompleteUpdate: read complete status, independently of screen setting
            if (screenOn || forceCompleteUpdate) {
                // complete status read, with broadcast
                Logging.logVerbose(
                    Logging.Category.MONITOR,
                    "readClientStatus(): screen on, get complete status"
                )

                status = clientInterface.ccStatus
                val state = clientInterface.state
                val transfers = clientInterface.fileTransfers
                val acctMgrInfo = clientInterface.acctMgrInfo
                val newNotices = clientInterface.getNotices(clientStatus.mostRecentNoticeSeqNo)
                if (allNotNull(status, state, state?.hostInfo, acctMgrInfo)) {
                    clientStatus.setClientStatus(
                        status, state!!.results, state.projects,
                        transfers, state.hostInfo, acctMgrInfo,
                        newNotices
                    )
                } else {
                    var nullValues = ""
                    if (state == null) {
                        nullValues += "state "
                    } else {
                        if (state.hostInfo == null) nullValues += "state.host_info "
                    }
                    if (acctMgrInfo == null) nullValues += "acctMgrInfo "

                    Logging.logError(
                        Logging.Category.MONITOR,
                        "readClientStatus(): connection problem, null: $nullValues"
                    )
                }

                // update notices notification
                noticeNotification.update(
                    clientStatus.getRssNotices(),
                    appPreferences.showNotificationForNotices
                )

                // check whether monitor is still intended to update, if not, skip broadcast and exit...
                if (updateBroadcastEnabled) {
                    applicationContext.sendBroadcast(Intent().apply {
                        action = "edu.berkeley.boinc.clientstatus"
                    })
                }
            } else {
                // read only ccStatus to adjust wakelocks and service state independently of screen status
                status = clientInterface.ccStatus
            }
            if (BuildConfig.DEBUG && status == null) {
                error("Assertion failed")
            }
            val computing = (status!!.taskSuspendReason == SUSPEND_NOT_SUSPENDED
                    || status.taskSuspendReason == SUSPEND_REASON_CPU_THROTTLE)

            Logging.logVerbose(
                Logging.Category.MONITOR,
                "readClientStatus(): computation enabled: $computing"
            )

            clientStatus.setWifiLock(computing)
            clientStatus.setWakeLock(computing)
            clientNotification.update(clientStatus, this, computing)
        } catch (e: Exception) {
            Logging.logException(
                Logging.Category.MONITOR,
                "Monitor.readClientStatus exception: " + e.message,
                e
            )
        }
    }

    /**
     * Reports current device status to the client via RPC
     * BOINC client uses this data to enforce preferences, e.g. suspend battery but requires information only/best available through Java API calls.
     */
    private fun reportDeviceStatus() {
        Logging.logVerbose(Logging.Category.MONITOR, "reportDeviceStatus()")

        try {
            // set devices status
            // make sure deviceStatus is initialized
            val reportStatusSuccess =
                clientInterface.reportDeviceStatus(deviceStatus.update(screenOn)) // transmit device status via rpc
            if (reportStatusSuccess)
                screenOffStatusOmitCounter = 0
            else
                Logging.logDebug(
                    Logging.Category.MONITOR,
                    "reporting device status returned false."
                )
        } catch (e: Exception) {
            Logging.logError(
                Logging.Category.MONITOR,
                "Monitor.reportDeviceStatus exception: " + e.message
            )
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
    private fun clientSetup(): Boolean {
        Logging.logVerbose(Logging.Category.MONITOR, "Monitor.clientSetup()")

        clientStatus.setSetupStatus(ClientStatus.SETUP_STATUS_LAUNCHING, true)
        val clientProcessName = boincWorkingDir + fileNameClient
        val md5AssetClient = computeMd5(fileNameClient, true)
        val md5InstalledClient = computeMd5(clientProcessName, false)

        // If client hashes do not match, we need to install the one that is a part
        // of the package. Shutdown the currently running client if needed.
        //
        if (forceReinstall || md5InstalledClient != md5AssetClient) {
            Logging.logDebug(
                Logging.Category.MONITOR,
                "Hashes of installed client does not match binary in assets - re-install."
            )

            // try graceful shutdown using RPC (faster)
            if (getPidForProcessName(clientProcessName) != null && connectClient()) {
                clientInterface.quit()
                val attempts =
                    applicationContext.resources.getInteger(R.integer.shutdown_graceful_rpc_check_attempts)
                val sleepPeriod =
                    applicationContext.resources.getInteger(R.integer.shutdown_graceful_rpc_check_rate_ms)
                var x = 0
                while (x < attempts) {
                    Thread.sleep(sleepPeriod.toLong())
                    if (getPidForProcessName(clientProcessName) == null) { //client is now closed
                        Logging.logDebug(
                            Logging.Category.MONITOR,
                            "quitClient: graceful RPC shutdown successful after " + x +
                                    " seconds"
                        )

                        x = attempts
                    }
                    x++
                }
            }

            // quit with OS signals
            if (getPidForProcessName(clientProcessName) != null) {
                quitProcessOsLevel(clientProcessName)
            }

            // at this point client is definitely not running. install new binary...
            if (!installClient()) {
                Logging.logError(Logging.Category.MONITOR, "BOINC client installation failed!")
                return false
            }
        }

        // Start the BOINC client if we need to.
        val clientPid = getPidForProcessName(clientProcessName)
        if (clientPid == null) {
            Logging.logInfo(Logging.Category.MONITOR, "Starting the BOINC client")

            if (!runClient(appPreferences.isRemote)) {
                Logging.logError(Logging.Category.MONITOR, "BOINC client failed to start")

                return false
            }
        }

        // Try to connect to executed Client in loop
        //
        val retryRate = resources.getInteger(R.integer.monitor_setup_connection_retry_rate_ms)
        val retryAttempts = resources.getInteger(R.integer.monitor_setup_connection_retry_attempts)
        var connected = false
        var counter = 0
        while (!connected && counter < retryAttempts) {
            Logging.logDebug(Logging.Category.MONITOR, "Attempting BOINC client connection...")
            connected = connectClient()
            counter++
            Thread.sleep(retryRate.toLong())
        }
        var init = false
        if (connected) { // connection established
            try {
                // read preferences for GUI to be able to display data
                val clientPrefs = clientInterface.globalPrefsWorkingStruct!!
                clientStatus.setPrefs(clientPrefs)

                // set Android model as hostinfo
                // should output something like "Samsung Galaxy SII - SDK:15 ABI:armeabi-v7a"
                val model =
                    "${Build.MANUFACTURER} ${Build.MODEL} - SDK: ${Build.VERSION.SDK_INT} ABI: $abi"
                val version = Build.VERSION.RELEASE

                Logging.logInfo(Logging.Category.MONITOR, "reporting hostinfo model name: $model")
                Logging.logInfo(Logging.Category.MONITOR, "reporting hostinfo os name: Android")
                Logging.logInfo(Logging.Category.MONITOR, "reporting hostinfo os version: $version")

                clientInterface.setHostInfo(model, version)
                init = true
            } catch (e: Exception) {
                Logging.logError(
                    Logging.Category.MONITOR,
                    "Monitor.clientSetup() init failed: " + e.message
                )
            }
        }
        if (init) {
            Logging.logDebug(
                Logging.Category.MONITOR,
                "Monitor.clientSetup() - setup completed successfully"
            )

            clientStatus.setSetupStatus(ClientStatus.SETUP_STATUS_AVAILABLE, false)
        } else {
            Logging.logError(
                Logging.Category.MONITOR,
                "Monitor.clientSetup() - setup experienced an error"
            )

            clientStatus.setSetupStatus(ClientStatus.SETUP_STATUS_ERROR, true)
        }
        return connected
    }

    /**
     * Executes BOINC client.
     * Using Java Runtime exec method
     *
     * @return Boolean success
     */
    private fun runClient(remote: Boolean): Boolean {
        isRemote = remote
        var success = false
        try {
            val param = if (remote) "--allow_remote_gui_rpc" else "--gui_rpc_unix_domain"
            val cmd = arrayOf(boincWorkingDir + fileNameClient, "--daemon", param)

            Logging.logInfo(
                Logging.Category.MONITOR,
                "Launching '${cmd[0]}' from '$boincWorkingDir'"
            )

            Runtime.getRuntime().exec(cmd, null, File(boincWorkingDir))
            success = true
        } catch (e: IOException) {
            Logging.logError(
                Logging.Category.MONITOR,
                "Starting BOINC client failed with exception: " + e.message
            )
            Logging.logException(Logging.Category.MONITOR, "IOException", e)
        }
        return success
    }

    /**
     * Establishes connection to client and handles initial authentication
     *
     * @return Boolean success
     */
    private fun connectClient(): Boolean {
        var success = if (isRemote) {
            clientInterface.connect()
        } else {
            clientInterface.open(clientSocketAddress)
        }
        if (!success) {
            Logging.logError(Logging.Category.MONITOR, "Connection failed!")

            return false
        }

        //authorize
        success = clientInterface.authorizeGuiFromFile(boincWorkingDir + fileNameGuiAuthentication)
        if (!success) {
            Logging.logError(Logging.Category.MONITOR, "Authorization failed!")
        }
        return success
    }

    /**
     * Installs required files from APK's asset directory to the applications' internal storage.
     * File attributes override and executable are defined here
     *
     * @return Boolean success
     */
    private fun installClient(): Boolean {
        if (!installFile(fileNameClient, true, "")) {
            Logging.logError(Logging.Category.MONITOR, INSTALL_FAILED + fileNameClient)

            return false
        }
        if (!installFile(fileNameCABundle, false, "")) {
            Logging.logError(Logging.Category.MONITOR, INSTALL_FAILED + fileNameCABundle)

            return false
        }
        if (!installFile(fileNameClientConfig, false, "")) {
            Logging.logError(Logging.Category.MONITOR, INSTALL_FAILED + fileNameClientConfig)

            return false
        }
        if (!installFile(fileNameAllProjectsList, false, "")) {
            Logging.logError(Logging.Category.MONITOR, INSTALL_FAILED + fileNameAllProjectsList)

            return false
        }
        if (!installFile(fileNameNoMedia, false, ".$fileNameNoMedia")) {
            Logging.logError(Logging.Category.MONITOR, INSTALL_FAILED + fileNameNoMedia)

            return false
        }
        return true
    }

    /**
     * Copies given file from APK assets to internal storage.
     *
     * @param file       name of file as it appears in assets directory
     * @param executable set executable flag of file in internal storage
     * @param targetFile name of target file
     * @return Boolean success
     */
    private fun installFile(file: String, executable: Boolean, targetFile: String): Boolean {
        var success = false

        // If file is executable, cpu architecture has to be evaluated
        // and assets directory select accordingly
        val source = if (executable) assetsDirForCpuArchitecture + file else file
        val target = if (targetFile.isNotEmpty()) {
            File(boincWorkingDir + targetFile)
        } else {
            File(boincWorkingDir + file)
        }
        try {
            // Copy file from the asset manager to clientPath
            applicationContext.assets.open(source).copyToFile(target)
            success = true //copy succeeded without exception

            // Set executable, if requested
            if (executable) {
                success = target.setExecutable(true) // return false, if not executable
            }

            Logging.logDebug(
                Logging.Category.MONITOR,
                "Installation of " + source + " successful. Executable: " +
                        executable + "/" + success
            )
        } catch (ioe: IOException) {
            Logging.logError(Logging.Category.MONITOR, IOEXCEPTION_LOG + ioe.message)
            Logging.logError(Logging.Category.MONITOR, "Install of $source failed.")
        }
        return success
    }

    /**
     * Determines assets directory (contains BOINC client binaries) corresponding to device's cpu architecture (ARM, x86)
     *
     * @return name of assets directory for given platform, not an absolute path.
     */
    private val assetsDirForCpuArchitecture: String
        get() {
            var archAssetsDirectory = ""
            when (boincPlatform) {
                R.string.boinc_platform_name_armv6 -> archAssetsDirectory =
                    getString(R.string.assets_dir_armv6)
                R.string.boinc_platform_name_arm -> archAssetsDirectory =
                    getString(R.string.assets_dir_arm)
                R.string.boinc_platform_name_arm64 -> archAssetsDirectory =
                    getString(R.string.assets_dir_arm64)
                R.string.boinc_platform_name_x86 -> archAssetsDirectory =
                    getString(R.string.assets_dir_x86)
                R.string.boinc_platform_name_x86_64 -> archAssetsDirectory =
                    getString(R.string.assets_dir_x86_64)
                else -> {
                }
            }
            return archAssetsDirectory
        }

    /**
     * Computes MD5 hash of requested file
     *
     * @param fileName absolute path or name of file in assets directory, see inAssets parameter
     * @param inAssets if true, fileName is file name in assets directory, if not, absolute path
     * @return md5 hash of file
     */
    private fun computeMd5(fileName: String, inAssets: Boolean): String {
        try {
            val source = if (inAssets) {
                applicationContext.assets.open(assetsDirForCpuArchitecture + fileName).source()
            } else {
                File(fileName).source()
            }.buffer()
            val md5 = source.readByteString().md5().hex()
            source.close()
            return md5
        } catch (e: IOException) {
            Logging.logError(Logging.Category.MONITOR, IOEXCEPTION_LOG + e.message)
        }
        return ""
    }

    /**
     * Determines ProcessID corresponding to given process name
     *
     * @param processName name of process, according to output of "ps"
     * @return process id, according to output of "ps"
     */
    private fun getPidForProcessName(processName: String): Int? {
        val processLines: List<String>

        //run ps and read output
        try {
            val p = Runtime.getRuntime().exec("ps")
            p.waitFor()
            val isr = InputStreamReader(p.inputStream)
            processLines = isr.readLines()
            isr.close()
        } catch (e: Exception) {
            Logging.logError(Logging.Category.MONITOR, "Exception: " + e.message)

            return null
        }
        if (processLines.size < 2) {
            Logging.logError(
                Logging.Category.MONITOR,
                "getPidForProcessName(): ps output has less than 2 lines, failure!"
            )

            return null
        }

        // figure out what index PID has
        val headers = processLines[0].split("[\\s]+".toRegex()).toTypedArray()
        val pidIndex = headers.indexOfFirst { it == "PID" }
        if (pidIndex == -1) {
            return null
        }

        Logging.logDebug(
            Logging.Category.MONITOR,
            "getPidForProcessName(): PID at index: $pidIndex for output:" +
                    " ${processLines[0]}"
        )

        var pid: Int? = null
        for (y in 1 until processLines.size) {
            var found = false
            val comps = processLines[y].split("[\\s]+".toRegex()).toTypedArray()
            for (arg in comps) {
                if (arg == processName) {
                    Logging.logDebug(
                        Logging.Category.MONITOR,
                        "getPidForProcessName(): $processName found in line: $y"
                    )

                    found = true
                    break // Break out of inner foreach (comps) loop
                }
            }
            if (found) {
                try {
                    pid = comps[pidIndex].toInt()

                    Logging.logDebug(Logging.Category.MONITOR, "getPidForProcessName(): pid: $pid")
                } catch (e: NumberFormatException) {
                    Logging.logError(
                        Logging.Category.MONITOR,
                        "getPidForProcessName(): NumberFormatException for " +
                                "${comps[pidIndex]} at index: $pidIndex"
                    )
                }
                break // Break out of outer for (processLinesAr) loop
            }
        }
        // if not happen in ps output, not running?!
        if (pid == null)
            Logging.logError(
                Logging.Category.MONITOR,
                "getPidForProcessName(): $processName not found in ps output!"
            )

        // Find required pid
        return pid
    }

    /**
     * Exits a process by sending it Linux SIGQUIT and SIGKILL signals
     *
     * @param processName name of process to be killed, according to output of "ps"
     */
    private fun quitProcessOsLevel(processName: String) {
        var clientPid = getPidForProcessName(processName)

        // client PID could not be read, client already ended / not yet started?
        if (clientPid == null) {
            Logging.logError(
                Logging.Category.MONITOR,
                "quitProcessOsLevel could not find PID, already ended or not" +
                        " yet started?"
            )

            return
        }

        Logging.logDebug(
            Logging.Category.MONITOR,
            "quitProcessOsLevel for $processName, pid: $clientPid"
        )

        // Do not just kill the client on the first attempt.  That leaves dangling
        // science applications running which causes repeated spawning of applications.
        // Neither the UI or client are happy and each are trying to recover from the
        // situation.  Instead send SIGQUIT and give the client time to clean up.
        Process.sendSignal(clientPid, Process.SIGNAL_QUIT)

        // Wait for the client to shutdown gracefully
        val attempts =
            applicationContext.resources.getInteger(R.integer.shutdown_graceful_os_check_attempts)
        val sleepPeriod =
            applicationContext.resources.getInteger(R.integer.shutdown_graceful_os_check_rate_ms)
        var x = 0
        while (x < attempts) {
            Thread.sleep(sleepPeriod.toLong())
            if (getPidForProcessName(processName) == null) { //client is now closed
                Logging.logDebug(
                    Logging.Category.MONITOR,
                    "quitClient: graceful SIGQUIT shutdown successful after" +
                            " $x seconds"
                )

                x = attempts
            }
            x++
        }
        clientPid = getPidForProcessName(processName)
        if (clientPid != null) {
            // Process is still alive, send SIGKILL
            Logging.logError(Logging.Category.MONITOR, "SIGQUIT failed. SIGKILL pid: $clientPid")

            Process.killProcess(clientPid)
        }
        clientPid = getPidForProcessName(processName)
        if (clientPid != null) {
            Logging.logError(
                Logging.Category.MONITOR,
                "SIGKILL failed. still living pid: $clientPid"
            )
        }
    }
    // --end-- BOINC client installation and run-time management

    /**
     * broadcast receiver to detect changes to screen on or off, used to adapt scheduling of StatusUpdateTimerTask
     * e.g. avoid polling GUI status RPCs while screen is off in order to save battery
     */
    private var screenOnOffReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (action == Intent.ACTION_SCREEN_OFF) {
                screenOn = false
                // forces report of device status at next scheduled update
                // allows timely reaction to screen off for resume of computation
                screenOffStatusOmitCounter = deviceStatusIntervalScreenOff

                Logging.logDebug(Logging.Category.MONITOR, "screenOnOffReceiver: screen turned off")
            }
            if (action == Intent.ACTION_SCREEN_ON) {
                screenOn = true

                Logging.logDebug(
                    Logging.Category.MONITOR,
                    "screenOnOffReceiver: screen turned on, force data refresh..."
                )

                forceRefresh()
            }
        }
    }

    private suspend fun setClientRunMode(runMode: Int) = coroutineScope {
        try {
            mBinder.setRunMode(runMode)
        } catch (e: RemoteException) {
            Logging.logException(Logging.Category.MONITOR, "setClientRunMode() error: ", e)
        }
        return@coroutineScope
    }

    // remote service
    val mBinder: IMonitor.Stub = object : IMonitor.Stub() {
        @Throws(RemoteException::class)
        override fun transferOperation(list: List<Transfer>, op: Int): Boolean {
            return clientInterface.transferOperation(list, op)
        }

        @Throws(RemoteException::class)
        override fun synchronizeAcctMgr(url: String): Boolean {
            return clientInterface.synchronizeAcctMgr(url)
        }

        @Throws(RemoteException::class)
        override fun setRunMode(mode: Int): Boolean {
            return clientInterface.setRunMode(mode)
        }

        @Throws(RemoteException::class)
        override fun setNetworkMode(mode: Int): Boolean {
            return clientInterface.setNetworkMode(mode)
        }

        @Throws(RemoteException::class)
        override fun setGlobalPreferences(pref: GlobalPreferences): Boolean {
            return clientInterface.setGlobalPreferences(pref)
        }

        @Throws(RemoteException::class)
        override fun setCcConfig(config: String): Boolean {
            return clientInterface.setCcConfig(config)
        }

        @Throws(RemoteException::class)
        override fun setDomainName(deviceName: String): Boolean {
            return clientInterface.setDomainName(deviceName)
        }

        @Throws(RemoteException::class)
        override fun resultOp(op: Int, url: String, name: String): Boolean {
            return clientInterface.resultOp(op, url, name)
        }

        @Throws(RemoteException::class)
        override fun readAuthToken(path: String): String {
            return clientInterface.readAuthToken(path)
        }

        @Throws(RemoteException::class)
        override fun projectOp(status: Int, url: String): Boolean {
            return clientInterface.projectOp(status, url)
        }

        @Throws(RemoteException::class)
        override fun getBoincPlatform(): Int {
            return this@Monitor.boincPlatform
        }

        @Throws(RemoteException::class)
        override fun lookupCredentials(credentials: AccountIn): AccountOut {
            return clientInterface.lookupCredentials(credentials) ?:
                throw RemoteException("Credentials lookup failed")
        }

        @Throws(RemoteException::class)
        override fun isStationaryDeviceSuspected(): Boolean {
            try {
                return deviceStatus.isStationaryDeviceSuspected
            } catch (e: Exception) {
                Logging.logException(
                    Logging.Category.MONITOR,
                    "Monitor.IMonitor.Stub: isStationaryDeviceSuspected() error: ",
                    e
                )
            }
            return false
        }

        @Throws(RemoteException::class)
        override fun getServerNotices(): List<Notice> {
            return clientStatus.getServerNotices()
        }

        @Throws(RemoteException::class)
        override fun getProjectConfigPolling(url: String): ProjectConfig {
            val result = clientInterface.getProjectConfigPolling(url)
            if (result != null) {
                return result
            }
            throw RemoteException("Project config polling failed")
        }

        @Throws(RemoteException::class)
        override fun getNotices(seq: Int): List<Notice> {
            return clientInterface.getNotices(seq)
        }

        @Throws(RemoteException::class)
        override fun getMessages(seq: Int): List<Message> {
            return clientInterface.getMessages(seq)
        }

        @Throws(RemoteException::class)
        override fun getEventLogMessages(seq: Int, num: Int): List<Message> {
            return clientInterface.getEventLogMessages(seq, num)
        }

        @Throws(RemoteException::class)
        override fun getBatteryChargeStatus(): Int {
            try {
                return deviceStatus.status.batteryChargePct
            } catch (e: Exception) {
                Logging.logException(
                    Logging.Category.MONITOR,
                    "Monitor.IMonitor.Stub: getBatteryChargeStatus() error: ",
                    e
                )
            }
            return 0
        }

        @Throws(RemoteException::class)
        override fun getAcctMgrInfo(): AcctMgrInfo? {
            return clientInterface.acctMgrInfo
        }

        @Throws(RemoteException::class)
        override fun forceRefresh() {
            this@Monitor.forceRefresh()
        }

        @Throws(RemoteException::class)
        override fun createAccountPolling(information: AccountIn): AccountOut {
            return clientInterface.createAccountPolling(information)?:
                throw RemoteException("Account creation failed")
        }

        @Throws(RemoteException::class)
        override fun checkProjectAttached(url: String): Boolean {
            return clientInterface.checkProjectAttached(url)
        }

        @Throws(RemoteException::class)
        override fun attachProject(
            url: String,
            projectName: String,
            authenticator: String
        ): Boolean {
            return clientInterface.attachProject(url, projectName, authenticator)
        }

        override fun addAcctMgrErrorNum(
            url: String,
            userName: String,
            pwd: String
        ): ErrorCodeDescription {
            val acctMgr = clientInterface.addAcctMgr(url, userName, pwd)
            return if (acctMgr != null) {
                ErrorCodeDescription(
                    acctMgr.errorNum,
                    if (acctMgr.messages.isEmpty()) "" else acctMgr.messages.toString()
                )
            } else ErrorCodeDescription(-1)
        }

        @Throws(RemoteException::class)
        override fun getAuthFilePath(): String {
            return this@Monitor.authFilePath
        }

        @Throws(RemoteException::class)
        override fun getAttachableProjects(): List<ProjectInfo> {
            return clientInterface.getAttachableProjects(getString(boincPlatform), boincAltPlatform)
        }

        @Throws(RemoteException::class)
        override fun getAccountManagers(): List<AccountManager> {
            return clientInterface.accountManagers
        }

        @Throws(RemoteException::class)
        override fun getAcctMgrInfoPresent(): Boolean {
            // Check if acctMgrInfo is present in clientStatus
            if (clientStatus.acctMgrInfo == null) {
                throw RemoteException("AcctMgrInfo is not initialized")
            }
            return clientStatus.acctMgrInfo!!.isPresent
        }

        @Throws(RemoteException::class)
        override fun getSetupStatus(): Int {
            return clientStatus.setupStatus
        }

        @Throws(RemoteException::class)
        override fun getComputingStatus(): Int {
            return clientStatus.computingStatus
        }

        @Throws(RemoteException::class)
        override fun getComputingSuspendReason(): Int {
            return clientStatus.computingSuspendReason
        }

        @Throws(RemoteException::class)
        override fun getNetworkSuspendReason(): Int {
            return clientStatus.networkSuspendReason
        }

        @Throws(RemoteException::class)
        override fun getHostInfo(): HostInfo {
            if (clientStatus.hostInfo == null) {
                throw RemoteException("HostInfo is not initialized")
            }
            return clientStatus.hostInfo!!
        }

        @Throws(RemoteException::class)
        override fun getPrefs(): GlobalPreferences {
            val prefs = clientStatus.getPrefs()
                ?: throw RemoteException("GlobalPreferences is not initialized")
            return prefs
        }

        @Throws(RemoteException::class)
        override fun getProjects(): List<Project> {
            return clientStatus.getProjects()
        }

        @Throws(RemoteException::class)
        override fun getClientAcctMgrInfo(): AcctMgrInfo {
            if (clientStatus.acctMgrInfo == null) {
                throw RemoteException("AcctMgrInfo is not initialized")
            }
            return clientStatus.acctMgrInfo!!
        }

        @Throws(RemoteException::class)
        override fun getTransfers(): List<Transfer> {
            return clientStatus.getTransfers()
        }

        @Throws(RemoteException::class)
        override fun setAutostart(isAutoStart: Boolean) {
            appPreferences.autostart = isAutoStart
        }

        @Throws(RemoteException::class)
        override fun setShowNotificationForNotices(isShow: Boolean) {
            appPreferences.showNotificationForNotices = isShow
        }

        @Throws(RemoteException::class)
        override fun getShowAdvanced(): Boolean {
            return appPreferences.showAdvanced
        }

        @Throws(RemoteException::class)
        override fun getIsRemote(): Boolean {
            return appPreferences.isRemote
        }

        @Throws(RemoteException::class)
        override fun getAutostart(): Boolean {
            return appPreferences.autostart
        }

        @Throws(RemoteException::class)
        override fun getShowNotificationForNotices(): Boolean {
            return appPreferences.showNotificationForNotices
        }

        @Throws(RemoteException::class)
        override fun getLogLevel(): Int {
            return appPreferences.logLevel
        }

        @Throws(RemoteException::class)
        override fun setLogLevel(level: Int) {
            appPreferences.logLevel = level
        }

        @Throws(RemoteException::class)
        override fun getLogCategories(): List<String> {
            return appPreferences.logCategories
        }

        @Throws(RemoteException::class)
        override fun setLogCategories(categories: List<String>) {
            appPreferences.logCategories = categories
        }

        @Throws(RemoteException::class)
        override fun setPowerSourceAc(src: Boolean) {
            appPreferences.powerSourceAc = src
        }

        @Throws(RemoteException::class)
        override fun setPowerSourceUsb(src: Boolean) {
            appPreferences.powerSourceUsb = src
        }

        @Throws(RemoteException::class)
        override fun setPowerSourceWireless(src: Boolean) {
            appPreferences.powerSourceWireless = src
        }

        @Throws(RemoteException::class)
        override fun getTasks(start: Int, count: Int, isActive: Boolean): List<Result> {
            return clientStatus.getTasks(start, count, isActive)
        }

        @Throws(RemoteException::class)
        override fun getTasksCount(): Int {
            return clientStatus.tasksCount
        }

        @Throws(RemoteException::class)
        override fun getProjectStatus(url: String): String {
            return clientStatus.getProjectStatus(url)
        }

        @Throws(RemoteException::class)
        override fun getRssNotices(): List<Notice> {
            return clientStatus.getRssNotices()
        }

        @Throws(RemoteException::class)
        override fun getSlideshowForProject(url: String): List<ImageWrapper> {
            return clientStatus.getSlideshowForProject(url)
        }

        @Throws(RemoteException::class)
        override fun getStationaryDeviceMode(): Boolean {
            return appPreferences.stationaryDeviceMode
        }

        @Throws(RemoteException::class)
        override fun getPowerSourceAc(): Boolean {
            return appPreferences.powerSourceAc
        }

        @Throws(RemoteException::class)
        override fun getPowerSourceUsb(): Boolean {
            return appPreferences.powerSourceUsb
        }

        @Throws(RemoteException::class)
        override fun getPowerSourceWireless(): Boolean {
            return appPreferences.powerSourceWireless
        }

        @Throws(RemoteException::class)
        override fun setShowAdvanced(isShow: Boolean) {
            appPreferences.showAdvanced = isShow
        }

        @Throws(RemoteException::class)
        override fun setIsRemote(isRemote: Boolean) {
            appPreferences.isRemote = isRemote
        }

        @Throws(RemoteException::class)
        override fun setStationaryDeviceMode(mode: Boolean) {
            appPreferences.stationaryDeviceMode = mode
        }

        @Throws(RemoteException::class)
        override fun getProjectIconByName(name: String): Bitmap? {
            return clientStatus.getProjectIconByName(name)
        }

        @Throws(RemoteException::class)
        override fun getProjectIcon(id: String): Bitmap? {
            return clientStatus.getProjectIcon(id)
        }

        @Throws(RemoteException::class)
        override fun getSuspendWhenScreenOn(): Boolean {
            return appPreferences.suspendWhenScreenOn
        }

        @Throws(RemoteException::class)
        override fun setSuspendWhenScreenOn(swso: Boolean) {
            appPreferences.suspendWhenScreenOn = swso
        }

        @Throws(RemoteException::class)
        override fun getCurrentStatusTitle(): String {
            return clientStatus.currentStatusTitle
        }

        @Throws(RemoteException::class)
        override fun getCurrentStatusDescription(): String {
            return clientStatus.currentStatusDescription
        }

        @Throws(RemoteException::class)
        override fun cancelNoticeNotification() {
            noticeNotification.cancelNotification()
        }

        @Throws(RemoteException::class)
        override fun setShowNotificationDuringSuspend(isShow: Boolean) {
            appPreferences.showNotificationDuringSuspend = isShow
        }

        @Throws(RemoteException::class)
        override fun getShowNotificationDuringSuspend(): Boolean {
            return appPreferences.showNotificationDuringSuspend
        }

        @Throws(RemoteException::class)
        override fun runBenchmarks(): Boolean {
            return clientInterface.runBenchmarks()
        }

        @Throws(RemoteException::class)
        override fun getProjectInfo(url: String): ProjectInfo? {
            return clientInterface.getProjectInfo(url)
        }

        @Throws(RemoteException::class)
        override fun boincMutexAcquired(): Boolean {
            return mutex.isAcquired
        }

        @Throws(RemoteException::class)
        override fun quitClient(): Boolean {
            return this@Monitor.quitClient()
        }
    } // --end-- remote service

    companion object {
        private const val INSTALL_FAILED = "Failed to install: "
        private const val IOEXCEPTION_LOG = "IOException: "
    }
}
