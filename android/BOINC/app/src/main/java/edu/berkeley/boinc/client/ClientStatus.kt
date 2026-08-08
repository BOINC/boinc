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

import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.wifi.WifiManager
import android.net.wifi.WifiManager.WifiLock
import android.os.PowerManager
import android.os.PowerManager.WakeLock
import android.text.format.DateUtils
import androidx.annotation.VisibleForTesting
import androidx.collection.ArraySet
import androidx.core.content.ContextCompat
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.AcctMgrInfo
import edu.berkeley.boinc.rpc.CcStatus
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.HostInfo
import edu.berkeley.boinc.rpc.ImageWrapper
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.Result
import edu.berkeley.boinc.rpc.Transfer
import edu.berkeley.boinc.utils.*
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.Logging.Category.CLIENT
import edu.berkeley.boinc.utils.Logging.logDebug
import edu.berkeley.boinc.utils.Logging.logError
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.Logging.logVerbose
import edu.berkeley.boinc.utils.translateRPCReason
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import java.nio.channels.FileChannel.MapMode
import java.nio.charset.Charset
import java.time.Duration
import java.time.Instant
import java.util.regex.Pattern
import javax.inject.Inject
import javax.inject.Singleton
import org.apache.commons.lang3.StringUtils

/*
 * Singleton that holds the client status data, as determined by the Monitor.
 * To get instance call Monitor.getClientStatus()
 */
@VisibleForTesting
@Singleton
open class ClientStatus @Inject constructor(
    // application context in order to fire broadcast events
    private val context: Context,
    private val appPreferences: AppPreferences,
    private val deviceStatus: DeviceStatus
) {

    // CPU WakeLock
    private val wakeLock: WakeLock

    // WiFi lock
    private val wifiLock: WifiLock

    //RPC wrapper
    private var status: CcStatus? = null
    private var results: List<Result>? = null
    private var projects: List<Project>? = null
    private var transfers: List<Transfer>? = null
    private var prefs: GlobalPreferences? = null
    private var hostinfo: HostInfo? = null

    // can be null
    @get:Synchronized
    var acctMgrInfo: AcctMgrInfo? = null
        private set

    // setup status
    var setupStatus: Int = 0
    private var setupStatusParseError = false

    // computing status
    var computingStatus: Int = 2

    // reason why computing got suspended, only if COMPUTING_STATUS_SUSPENDED
    var computingSuspendReason: Int = 0

    // indicates that status could not be parsed and is therefore invalid
    private var computingParseError = false

    // network status
    private var networkStatus: Int = 2

    // reason why network activity got suspended, only if NETWORK_STATUS_SUSPENDED
    var networkSuspendReason: Int = 0

    // indicates that status could not be parsed and is therefore invalid
    private var networkParseError = false

    // notices
    private val rssNotices: MutableList<Notice> = ArrayList()
    private val serverNotices: MutableList<Notice> = ArrayList()
    var mostRecentNoticeSeqNo: Int = 0
        private set

    init {
        // set up CPU wakelock
        // see documentation at http://developer.android.com/reference/android/os/PowerManager.html
        val pm = ContextCompat.getSystemService(context, PowerManager::class.java)
        wakeLock = pm!!.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, Logging.WAKELOCK)
        // "one call to release() is sufficient to undo the effect of all previous calls to acquire()"
        wakeLock.setReferenceCounted(false)

        // Set up WiFi wakelock
        // On versions prior to Android N (24), initializing the WifiManager via Context#getSystemService
        // can cause a memory leak if the context is not the application context.
        // You should consider using context.getApplicationContext().getSystemService() rather than
        // context.getSystemService()
        val wm = ContextCompat.getSystemService(
            context.applicationContext,
            WifiManager::class.java
        )
        wifiLock = wm!!.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, "MyWifiLock")
        wifiLock.setReferenceCounted(false)
    }

    // call to acquire or release resources held by the WakeLock.
    // acquisition: every time the Monitor loop calls setClientStatus and computingStatus == COMPUTING_STATUS_COMPUTING
    // release: every time acquisition criteria is not met , and in Monitor.onDestroy()
    @SuppressLint("WakelockTimeout") // To run BOINC in the background, wakeLock has to be used without timeout.
    fun setWakeLock(acquire: Boolean) {
        try {
            if (wakeLock.isHeld == acquire) {
                return  // wakeLock already in desired state
            }

            if (acquire) { // acquire wakeLock
                wakeLock.acquire()

                logVerbose(CLIENT, "wakeLock acquired")
            } else { // release wakeLock
                wakeLock.release()

                logVerbose(CLIENT, "wakeLock released")
            }
        } catch (e: Exception) {
            logException(
                CLIENT,
                "Exception during setWakeLock $acquire", e
            )
        }
    }

    // call to acquire or release resources held by the WifiLock.
    // acquisition: every time the Monitor loop calls setClientStatus: computingStatus == COMPUTING_STATUS_COMPUTING || COMPUTING_STATUS_IDLE
    // release: every time acquisition criteria is not met , and in Monitor.onDestroy()
    fun setWifiLock(acquire: Boolean) {
        try {
            if (wifiLock.isHeld == acquire) {
                return  // wifiLock already in desired state
            }

            if (acquire) { // acquire wakeLock
                wifiLock.acquire()

                logVerbose(CLIENT, "wifiLock acquired")
            } else { // release wakeLock
                wifiLock.release()

                logVerbose(CLIENT, "wifiLock released")
            }
        } catch (e: Exception) {
            logException(
                CLIENT,
                "Exception during setWifiLock $acquire", e
            )
        }
    }

    /*
     * fires "clientstatuschange" broadcast, so registered Activities can update their model.
     */
    @Synchronized
    fun fire() {
        val clientChanged = Intent()
        clientChanged.setAction("edu.berkeley.boinc.clientstatuschange")
        context.sendBroadcast(clientChanged, null)
    }

    /*
     * called frequently by Monitor to set the RPC data. These objects are used to determine the client status and parse it in the data model of this class.
     */
    @Synchronized
    fun setClientStatus(
        status: CcStatus?,
        results: List<Result>,
        projects: List<Project>,
        transfers: List<Transfer>,
        hostinfo: HostInfo?,
        acctMgrInfo: AcctMgrInfo?,
        newNotices: List<Notice>
    ) {
        this.status = status
        this.results = results
        this.projects = projects
        this.transfers = transfers
        this.hostinfo = hostinfo
        this.acctMgrInfo = acctMgrInfo
        parseClientStatus()
        appendNewNotices(newNotices)

        logVerbose(
            CLIENT,
            "setClientStatus: #results: " + results.size + " #projects: " + projects.size +
                    " #transfers: " + transfers.size + " // computing: " +
                    " computingParseError: " + computingParseError + " computingStatus: " + computingStatus +
                    " computingSuspendReason: " + computingSuspendReason + " - network: " +
                    " networkParseError: " + networkParseError + " networkStatus: " + networkStatus +
                    " networkSuspendReason: " + networkSuspendReason
        )

        if (!computingParseError && !networkParseError && !setupStatusParseError) {
            fire() // broadcast that status has changed
        } else {
            logDebug(
                CLIENT,
                "ClientStatus discard status change due to parse error: " +
                        " computingParseError: " + computingParseError + " computingStatus: " + computingStatus +
                        " computingSuspendReason: " + computingSuspendReason + " networkParseError: " + networkParseError +
                        " networkStatus: " + networkStatus + " networkSuspendReason: " + networkSuspendReason + " - " +
                        " setupStatusParseError: " + setupStatusParseError
            )
        }
    }

    /*
     * called when setup status needs to be manipulated by Java routine
     * either during setup or closing of client.
     * this function does not effect the state of the client!
     */
    @Synchronized
    fun setSetupStatus(newStatus: Int, fireStatusChangeEvent: Boolean) {
        setupStatus = newStatus
        if (fireStatusChangeEvent) {
            fire()
        }
    }

    /*
     * called after reading global preferences, e.g. during ClientStartAsync
     */
    @Synchronized
    fun setPrefs(prefs: GlobalPreferences?) {
        this.prefs = prefs
    }

    @Synchronized
    fun getRssNotices(): List<Notice> {
        return rssNotices
    }

    @Synchronized
    fun getServerNotices(): List<Notice> {
        return serverNotices
    }

    @get:Synchronized
    val clientStatus: CcStatus?
        get() {
            if (results == null) { //check in case monitor is not set up yet (e.g. while logging in)
                logDebug(
                    CLIENT,
                    "state is null"
                )

                return null
            }
            return status
        }

    @Synchronized
    fun getTasks(start: Int, count: Int, isActive: Boolean): List<Result> {
        if (results == null) { //check in case monitor is not set up yet (e.g. while logging in)
            logDebug(CLIENT, "tasks is null")

            return emptyList()
        }

        val tasks: MutableList<Result> = ArrayList()
        var counter = 0

        for (res in results!!) {
            if (tasks.size == count) {
                break
            }

            if (start > counter++) {
                continue
            }

            if ((isActive && res.isActiveTask) || (!isActive && !res.isActiveTask)) {
                tasks.add(res)
            }
        }

        return tasks
    }

    @get:Synchronized
    val tasksCount: Int
        get() {
            if (results == null) { //check in case monitor is not set up yet (e.g. while logging in)
                logDebug(
                    CLIENT,
                    "tasksCount is null"
                )

                return 0
            }
            return results!!.size
        }

    @Synchronized
    fun getTransfers(): List<Transfer> {
        if (transfers == null) { //check in case monitor is not set up yet (e.g. while logging in)
            logDebug(CLIENT, "transfers is null")

            return emptyList()
        }
        return transfers as List<Transfer>
    }

    @Synchronized
    fun getPrefs(): GlobalPreferences? {
        if (prefs == null) { //check in case monitor is not set up yet (e.g. while logging in)
            logDebug(CLIENT, "prefs is null")

            return null
        }
        return prefs
    }

    @Synchronized
    fun getProjects(): List<Project> {
        if (projects == null) { //check in case monitor is not set up yet (e.g. while logging in)
            logDebug(CLIENT, "getProject() state is null")

            return emptyList()
        }
        return projects as List<Project>
    }

    @Synchronized
    fun getProjectStatus(master_url: String): String {
        val sb = StringBuffer()
        for ((masterURL, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, minRPCTime1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, scheduledRPCPending, _, _, _, _, suspendedViaGUI, doNotRequestMoreWork, schedulerRPCInProgress, _, detachWhenDone, ended, trickleUpPending) in projects!!) {
            if (masterURL != master_url) {
                continue
            }

            if (suspendedViaGUI) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_suspendedviagui)
                )
            }
            if (doNotRequestMoreWork) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_dontrequestmorework)
                )
            }
            if (ended) {
                appendToStatus(sb, context.resources.getString(R.string.projects_status_ended))
            }
            if (detachWhenDone) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_detachwhendone)
                )
            }
            if (scheduledRPCPending > 0) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_schedrpcpending)
                )
                appendToStatus(sb, context.translateRPCReason(scheduledRPCPending))
            }
            if (schedulerRPCInProgress) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_schedrpcinprogress)
                )
            }
            if (trickleUpPending) {
                appendToStatus(
                    sb,
                    context.resources.getString(R.string.projects_status_trickleuppending)
                )
            }

            val now = Instant.now()
            val minRPCTime = Instant.ofEpochSecond(minRPCTime1.toLong())
            if (minRPCTime > now) {
                val elapsedTime = DateUtils.formatElapsedTime(
                    Duration.between(now, minRPCTime)
                        .seconds
                )
                val backoff = context.resources.getString(
                    R.string.projects_status_backoff,
                    elapsedTime
                )
                appendToStatus(sb, backoff)
            }
        }
        return sb.toString()
    }

    private fun appendToStatus(existing: StringBuffer, additional: String) {
        if (existing.isEmpty()) {
            existing.append(additional)
        } else {
            existing.append(", ")
            existing.append(additional)
        }
    }

    @get:Synchronized
    val hostInfo: HostInfo?
        get() {
            if (hostinfo == null) {
                logDebug(
                    CLIENT,
                    "getHostInfo() state is null"
                )

                return null
            }
            return hostinfo
        }

    // returns all slideshow images for given project
    // images: 126 * 290 pixel from /projects/PNAME/slideshow_appname_n
    // not aware of application!
    @Synchronized
    fun getSlideshowForProject(masterUrl: String): List<ImageWrapper> {
        val images: MutableList<ImageWrapper> = ArrayList()
        for ((masterURL, projectDir, _, projectName) in projects!!) {
            if (masterURL != masterUrl) {
                continue
            }
            // get file paths of soft link files
            val dir = File(projectDir)
            val foundFiles = dir.listFiles { _: File?, name: String ->
                name.startsWith("slideshow_")
                        && !name.endsWith(".png")
            }
            if (foundFiles == null) {
                continue  // prevent NPE
            }

            val allImagePaths: MutableSet<String?> = ArraySet()
            for (file in foundFiles) {
                val slideshowImagePath = parseSoftLinkToAbsPath(
                    file.absolutePath,
                    projectDir
                )
                //check whether path is not empty, and avoid duplicates (slideshow images can
                //re-occur for multiple apps, since we do not distinct apps, skip duplicates.
                if (StringUtils.isNotEmpty(slideshowImagePath)) {
                    allImagePaths.add(slideshowImagePath)
                }
            }

            // load images from paths
            for (filePath in allImagePaths) {
                val tmp = BitmapFactory.decodeFile(filePath)
                if (tmp != null) {
                    images.add(ImageWrapper(tmp, projectName, filePath))
                } else {
                    logDebug(
                        CLIENT,
                        "loadSlideshowImagesFromFile(): null for path: $filePath"
                    )
                }
            }
        }
        return images
    }

    // returns project icon for given master url
    // bitmap: 40 * 40 pixel, symbolic link in /projects/PNAME/stat_icon
    @Synchronized
    fun getProjectIcon(masterUrl: String): Bitmap? {
        logVerbose(
            CLIENT,
            "getProjectIcon for: $masterUrl"
        )

        try {
            // loop through all projects
            for ((masterURL, projectDir) in projects!!) {
                if (masterURL == masterUrl) {
                    // read file name of icon
                    val iconAbsPath =
                        parseSoftLinkToAbsPath(
                            "$projectDir/stat_icon",
                            projectDir
                        )
                    if (iconAbsPath == null) {
                        logDebug(
                            CLIENT, "getProjectIcon could not parse sym link for project: " +
                                    masterUrl
                        )

                        return null
                    }
                    return BitmapFactory.decodeFile(iconAbsPath)
                }
            }
        } catch (e: Exception) {
            logException(CLIENT, "getProjectIcon failed", e)
        }

        logError(CLIENT, "getProjectIcon: project not found.")

        return null
    }

    // returns project icon for given project name
    // bitmap: 40 * 40 pixel, symbolic link in /projects/PNAME/stat_icon
    @Synchronized
    fun getProjectIconByName(projectName: String): Bitmap? {
        logVerbose(
            CLIENT,
            "getProjectIconByName for: $projectName"
        )

        try {
            // loop through all projects
            for ((_, projectDir, _, projectName1) in projects!!) {
                if (projectName1 == projectName) {
                    // read file name of icon
                    val iconAbsPath =
                        parseSoftLinkToAbsPath(
                            "$projectDir/stat_icon",
                            projectDir
                        )
                    if (iconAbsPath == null) {
                        logDebug(
                            CLIENT,
                            "getProjectIconByName could not parse sym link for project: " +
                                    projectName
                        )

                        return null
                    }
                    return BitmapFactory.decodeFile(iconAbsPath)
                }
            }
        } catch (e: Exception) {
            logException(CLIENT, "getProjectIconByName failed", e)
        }

        logError(CLIENT, "getProjectIconByName: project not found.")

        return null
    }

    val executingTasks: List<Result>
        get() {
            val activeTasks: MutableList<Result> =
                ArrayList()
            if (results != null && results!!.isNotEmpty()) {
                for (tmp in results!!) {
                    if (tmp.isActiveTask && tmp.activeTaskState == PROCESS_EXECUTING) {
                        activeTasks.add(tmp)
                    }
                }
            }
            return activeTasks
        }

    val currentStatusTitle: String
        get() {
            var statusTitle = ""
            try {
                when (setupStatus) {
                    SETUP_STATUS_AVAILABLE -> when (computingStatus) {
                        COMPUTING_STATUS_COMPUTING -> statusTitle =
                            context.getString(R.string.status_running)

                        COMPUTING_STATUS_IDLE -> statusTitle =
                            context.getString(R.string.status_idle)

                        COMPUTING_STATUS_SUSPENDED -> statusTitle =
                            context.getString(R.string.status_paused)

                        COMPUTING_STATUS_NEVER -> statusTitle =
                            context.getString(R.string.status_computing_disabled)
                    }

                    SETUP_STATUS_LAUNCHING -> statusTitle =
                        context.getString(R.string.status_launching)

                    SETUP_STATUS_NOPROJECT -> statusTitle =
                        context.getString(R.string.status_noproject)
                }
            } catch (e: Exception) {
                logException(
                    CLIENT,
                    "error parsing setup status string",
                    e
                )
            }
            return statusTitle
        }

    val currentStatusDescription: String
        get() {
            var statusString = ""
            try {
                when (computingStatus) {
                    COMPUTING_STATUS_COMPUTING -> statusString =
                        context.getString(R.string.status_running_long)

                    COMPUTING_STATUS_IDLE -> statusString =
                        when (networkSuspendReason) {
                            SUSPEND_REASON_WIFI_STATE -> {
                                // Network suspended due to wifi state
                                context.getString(R.string.suspend_wifi)
                            }
                            SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED -> {
                                // network suspend due to traffic quota
                                context.getString(R.string.suspend_network_quota)
                            }
                            else -> {
                                context.getString(R.string.status_idle_long)
                            }
                        }

                    COMPUTING_STATUS_SUSPENDED -> when (computingSuspendReason) {
                        SUSPEND_REASON_USER_REQ ->                             // restarting after user has previously manually suspended computation
                            statusString = context.getString(R.string.suspend_user_req)

                        SUSPEND_REASON_BENCHMARKS -> statusString =
                            context.getString(R.string.status_benchmarking)

                        SUSPEND_REASON_BATTERIES -> statusString =
                            context.getString(R.string.suspend_batteries)

                        SUSPEND_REASON_BATTERY_CHARGING -> {
                            statusString = context.getString(R.string.suspend_battery_charging)
                            try {
                                val minCharge = prefs!!.batteryChargeMinPct.toInt()
                                val currentCharge = deviceStatus.status.batteryChargePct
                                statusString = context.getString(
                                    R.string.suspend_battery_charging_long,
                                    minCharge, currentCharge
                                )
                            } catch (e: Exception) {
                                logException(
                                    CLIENT,
                                    "ClientStatus.getCurrentStatusDescription error: ",
                                    e
                                )
                            }
                        }

                        SUSPEND_REASON_BATTERY_OVERHEATED -> statusString =
                            context.getString(R.string.suspend_battery_overheating)

                        SUSPEND_REASON_USER_ACTIVE -> {
                            val suspendDueToScreenOn =
                                appPreferences.suspendWhenScreenOn
                            statusString = if (suspendDueToScreenOn) {
                                context.getString(R.string.suspend_screen_on)
                            } else {
                                context.getString(R.string.suspend_useractive)
                            }
                        }

                        SUSPEND_REASON_TIME_OF_DAY -> statusString =
                            context.getString(R.string.suspend_tod)

                        SUSPEND_REASON_DISK_SIZE -> statusString =
                            context.getString(R.string.suspend_disksize)

                        SUSPEND_REASON_CPU_THROTTLE -> statusString =
                            context.getString(R.string.suspend_cputhrottle)

                        SUSPEND_REASON_NO_RECENT_INPUT -> statusString =
                            context.getString(R.string.suspend_noinput)

                        SUSPEND_REASON_INITIAL_DELAY -> statusString =
                            context.getString(R.string.suspend_delay)

                        SUSPEND_REASON_EXCLUSIVE_APP_RUNNING -> statusString =
                            context.getString(R.string.suspend_exclusiveapp)

                        SUSPEND_REASON_CPU_USAGE -> statusString =
                            context.getString(R.string.suspend_cpu)

                        SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED -> statusString =
                            context.getString(R.string.suspend_network_quota)

                        SUSPEND_REASON_OS -> statusString = context.getString(R.string.suspend_os)
                        SUSPEND_REASON_WIFI_STATE -> statusString =
                            context.getString(R.string.suspend_wifi)

                        else -> statusString = context.getString(R.string.suspend_unknown)
                    }

                    COMPUTING_STATUS_NEVER -> statusString =
                        context.getString(R.string.status_computing_disabled_long)
                }
            } catch (e: Exception) {
                logException(
                    CLIENT,
                    "error parsing setup status string",
                    e
                )
            }
            return statusString
        }

    /*
     * parses RPC data to ClientStatus data model.
     */
    private fun parseClientStatus() {
        parseComputingStatus()
        parseProjectStatus()
        parseNetworkStatus()
    }

    private fun parseProjectStatus() {
        try {
            if (projects!!.isNotEmpty()) {
                setupStatus = SETUP_STATUS_AVAILABLE
                setupStatusParseError = false
            } else { //not projects attached
                setupStatus = SETUP_STATUS_NOPROJECT
                setupStatusParseError = false
            }
        } catch (e: Exception) {
            setupStatusParseError = true

            logException(CLIENT, "parseProjectStatus - Exception: ", e)
        }
    }

    private fun parseComputingStatus() {
        computingParseError = true
        try {
            if (status!!.taskMode == RUN_MODE_NEVER) {
                computingStatus = COMPUTING_STATUS_NEVER
                computingSuspendReason =
                    status!!.taskSuspendReason // = 4 - SUSPEND_REASON_USER_REQ????
                computingParseError = false
                return
            }
            if ((status!!.taskMode == RUN_MODE_AUTO) &&
                (status!!.taskSuspendReason != SUSPEND_NOT_SUSPENDED) &&
                (status!!.taskSuspendReason != SUSPEND_REASON_CPU_THROTTLE)
            ) {
                // do not expose cpu throttling as suspension to UI
                computingStatus = COMPUTING_STATUS_SUSPENDED
                computingSuspendReason = status!!.taskSuspendReason
                computingParseError = false
                return
            }
            if ((status!!.taskMode == RUN_MODE_AUTO) &&
                ((status!!.taskSuspendReason == SUSPEND_NOT_SUSPENDED) ||
                        (status!!.taskSuspendReason == SUSPEND_REASON_CPU_THROTTLE))
            ) {
                // treat cpu throttling as if client was active (either idle, or computing, depending on tasks)
                //figure out whether we have an active task
                var activeTask = false
                if (results != null) {
                    for ((_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, isActiveTask) in results!!) {
                        if (isActiveTask) { // this result has corresponding "active task" in RPC XML
                            activeTask = true
                            break // amount of active tasks does not matter.
                        }
                    }
                }

                if (activeTask) { // client is currently computing
                    computingStatus = COMPUTING_STATUS_COMPUTING
                    computingSuspendReason =
                        status!!.taskSuspendReason // = 0 - SUSPEND_NOT_SUSPENDED
                    computingParseError = false
                } else { // client "is able but idle"
                    computingStatus = COMPUTING_STATUS_IDLE
                    computingSuspendReason =
                        status!!.taskSuspendReason // = 0 - SUSPEND_NOT_SUSPENDED
                    computingParseError = false
                }
            }
        } catch (e: Exception) {
            logException(CLIENT, "ClientStatus parseComputingStatus - Exception: ", e)
        }
    }

    private fun parseNetworkStatus() {
        networkParseError = true
        try {
            if (status!!.networkMode == RUN_MODE_NEVER) {
                networkStatus = NETWORK_STATUS_NEVER
                networkSuspendReason =
                    status!!.networkSuspendReason // = 4 - SUSPEND_REASON_USER_REQ????
                networkParseError = false
                return
            }
            if ((status!!.networkMode == RUN_MODE_AUTO) &&
                (status!!.networkSuspendReason != SUSPEND_NOT_SUSPENDED)
            ) {
                networkStatus = NETWORK_STATUS_SUSPENDED
                networkSuspendReason = status!!.networkSuspendReason
                networkParseError = false
                return
            }
            if ((status!!.networkMode == RUN_MODE_AUTO) &&
                (status!!.networkSuspendReason == SUSPEND_NOT_SUSPENDED)
            ) {
                networkStatus = NETWORK_STATUS_AVAILABLE
                networkSuspendReason = status!!.networkSuspendReason // = 0 - SUSPEND_NOT_SUSPENDED
                networkParseError = false
            }
        } catch (e: Exception) {
            logException(CLIENT, "ClientStatus parseNetworkStatus - Exception", e)
        }
    }

    private fun appendNewNotices(newNotices: List<Notice>) {
        for (newNotice in newNotices) {
            logDebug(
                CLIENT, "ClientStatus.appendNewNotices new notice with seq number: " +
                        newNotice.seqno + " is server notice: " + newNotice.isServerNotice
            )

            if (newNotice.seqno > mostRecentNoticeSeqNo) {
                if (!newNotice.isClientNotice && !newNotice.isServerNotice) {
                    rssNotices.add(newNotice)
                }
                if (newNotice.isServerNotice) {
                    serverNotices.add(newNotice)
                }
                mostRecentNoticeSeqNo = newNotice.seqno
            }
        }
    }

    // helper method for loading images from file
    // reads the symbolic link provided in pathOfSoftLink file
    // and returns absolute path to an image file.
    private fun parseSoftLinkToAbsPath(pathOfSoftLink: String, projectDir: String): String? {
        // setup file
        val softLink = File(pathOfSoftLink)
        if (!softLink.exists()) {
            return null // return if file does not exist
        }

        // reading text of symbolic link
        var softLinkContent = ""
        try {
            try {
                FileInputStream(softLink).use { stream ->
                    val fc = stream.channel
                    val bb = fc.map(MapMode.READ_ONLY, 0, fc.size())
                    /* Instead of using default, pass in a decoder. */
                    softLinkContent = Charset.defaultCharset().decode(bb).toString()
                }
            } catch (e: IOException) {
                logException(CLIENT, "IOException in parseIconFileName()", e)
            }
        } catch (e: Exception) {
            // probably FileNotFoundException
            return null
        }

        // matching relevant path of String
        // matching 1+ word characters and 0 or 1 dot . and 0+ word characters
        // e.g. "icon.png", "icon", "icon.bmp"
        val statIconPattern = Pattern.compile("/(\\w+?\\.?\\w*?)</soft_link>")
        val m = statIconPattern.matcher(softLinkContent)
        if (!m.find()) {
            logError(
                CLIENT,
                "parseSoftLinkToAbsPath() could not match pattern in soft link file: $pathOfSoftLink"
            )

            return null
        }
        val fileName = m.group(1)

        return "$projectDir/$fileName"
    }

    companion object {
        // 0 = client is in setup routine (default)
        const val SETUP_STATUS_LAUNCHING: Int = 0

        // 1 = client is launched and available for RPC (connected and authorized)
        const val SETUP_STATUS_AVAILABLE: Int = 1

        // 2 = client is in a permanent error state
        const val SETUP_STATUS_ERROR: Int = 2

        // 3 = client is launched but not attached to a project (login)
        const val SETUP_STATUS_NOPROJECT: Int = 3
        const val COMPUTING_STATUS_NEVER: Int = 0
        const val COMPUTING_STATUS_SUSPENDED: Int = 1
        const val COMPUTING_STATUS_IDLE: Int = 2
        const val COMPUTING_STATUS_COMPUTING: Int = 3
        const val NETWORK_STATUS_NEVER: Int = 0
        const val NETWORK_STATUS_SUSPENDED: Int = 1
        const val NETWORK_STATUS_AVAILABLE: Int = 2
    }
}
