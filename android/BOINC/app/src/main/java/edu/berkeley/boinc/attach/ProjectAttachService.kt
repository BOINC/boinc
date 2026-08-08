/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
package edu.berkeley.boinc.attach

import android.app.Service
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.Binder
import android.os.IBinder
import android.os.RemoteException
import android.widget.Toast
import androidx.lifecycle.LifecycleService
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.BOINCApplication
import edu.berkeley.boinc.R
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.client.PersistentStorage
import edu.berkeley.boinc.rpc.*
import edu.berkeley.boinc.utils.*
import java.util.*
import javax.inject.Inject
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class ProjectAttachService : LifecycleService() {
    @Inject
    lateinit var store: PersistentStorage

    // life-cycle
    private val mBinder: IBinder = LocalBinder()
    val selectedProjects: MutableList<ProjectAttachWrapper> = ArrayList()
    var projectConfigRetrievalFinished = true // shows whether project retrieval is ongoing

    //credentials
    private var email = ""
    private var user = ""
    private var pwd = ""

    // monitor service binding
    private var monitor: IMonitor? = null
    private var mIsBound = false
    private val mConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service)
            mIsBound = true
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false
        }
    }
    /**
     * Returns last user input to be able to pre-populate fields.
     *
     * @return array of values, index 0: email address, index 1: user name
     */
    val userDefaultValues: List<String?>
        get() = listOf(store.lastEmailAddress, store.lastUserName)

    val numberOfSelectedProjects: Int
        get() = selectedProjects.size

    internal inner class LocalBinder : Binder() {
        val service: ProjectAttachService
            get() = this@ProjectAttachService
    }

    override fun onBind(intent: Intent): IBinder? {
        super.onBind(intent)

        Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.onBind")

        return mBinder
    }

    override fun onCreate() {
        (application as BOINCApplication).appComponent.inject(this)
        super.onCreate()

        Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.onCreate")

        doBindService()
    }

    override fun onDestroy() {
        super.onDestroy()

        Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.onDestroy")

        doUnbindService()
    }

    // --END-- life-cycle
    private fun doBindService() {
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(Intent(this, Monitor::class.java), mConnection, Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            unbindService(mConnection)
            mIsBound = false
        }
    }
    // --END-- monitor service binding
    /**
     * Set credentials to be used in account RPCs.
     * Set / update prior to calling attach or register
     * Saves email and user persistently to pre-populate fields
     *
     * @param email email address of user
     * @param user  user name
     * @param pwd   password
     */
    fun setCredentials(email: String, user: String, pwd: String) {
        this.email = email
        this.user = user
        this.pwd = pwd
        store.lastEmailAddress = email
        store.lastUserName = user
    }

    /**
     * sets selected projects and downloads their configuration files.
     * configuration download in new thread, returns immediately.
     * Check projectConfigRetrievalFinished to see whether job finished.
     *
     * @param selected list of selected projects
     */
    fun setSelectedProjects(selected: List<ProjectInfo?>) {
        if (!projectConfigRetrievalFinished) {
            Logging.logError(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.setSelectedProjects: stop, async task already running.")

            return
        }
        selectedProjects.clear()
        selectedProjects.addAll(selected.filterNotNull().map { ProjectAttachWrapper(it) })
        if (mIsBound) {
            lifecycleScope.launch { getProjectConfigs() }
        } else {
            Logging.logError(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.setSelectedProjects: could not load configuration files, monitor not bound.")

            return
        }
        Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                "ProjectAttachService.setSelectedProjects: number of selected projects: " + selectedProjects.size)
    }

    /**
     * sets single selected project with URL inserted manually, not chosen from list.
     * Starts configuration download in new thread and returns immediately.
     * Check projectConfigRetrievalFinished to see whether job finished.
     *
     * @param url URL of project
     */
    fun setManuallySelectedProject(url: String) {
        if (!projectConfigRetrievalFinished) {
            Logging.logError(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.setManuallySelectedProject: stop, async task already running.")

            return
        }
        selectedProjects.clear()
        selectedProjects.add(ProjectAttachWrapper(url))

        // get projectConfig
        if (mIsBound) {
            lifecycleScope.launch { getProjectConfigs() }
        } else {
            Logging.logError(Logging.Category.MONITOR, "ProjectAttachService.setManuallySelectedProject: could not load configuration file, monitor not bound.")

            return
        }

        Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                "ProjectAttachService.setManuallySelectedProject: url of selected project: " + url + ", list size: " +
                selectedProjects.size)
    }

    /**
     * Checks user input, e.g. length of input. Shows an error toast if problem detected
     *
     * @param email email address of user
     * @param user  user name
     * @param pwd   password
     * @return true if input verified
     */
    fun verifyInput(email: String, user: String, pwd: String): Boolean {
        // check input
        val stringResource = when {
            email.isEmpty() -> R.string.attachproject_error_no_email
            user.isEmpty() -> R.string.attachproject_error_no_name
            pwd.isEmpty() -> R.string.attachproject_error_no_pwd
            pwd.length < 6 -> R.string.attachproject_error_short_pwd
            else -> 0
        }
        return if (stringResource != 0) {
            val toast = Toast.makeText(applicationContext, stringResource, Toast.LENGTH_LONG)
            toast.show()
            false
        } else {
            true
        }
    }

    /**
     * Returns true as long as there have been unresolved conflicts.
     *
     * @return indicator whether conflicts exist
     */
    fun anyUnresolvedConflicts() = selectedProjects.any { it.result != RESULT_SUCCESS }

    /**
     * Attempts attach of account manager with credentials provided as parameter.
     * Does not require to select project or set credentials beforehand.
     *
     * @param url  acct mgr url
     * @param name user name
     * @param pwd  password
     * @return result code, see BOINCErrors
     */
    fun attachAcctMgr(url: String, name: String, pwd: String?): ErrorCodeDescription {
        var reply = ErrorCodeDescription()
        val maxAttempts = resources.getInteger(R.integer.attach_acctmgr_retries)
        var attemptCounter = 0
        var retry = true

        Logging.logDebug(Logging.Category.PROJECT_SERVICE, "account manager with: $url, $name, $maxAttempts")

        // retry a defined number of times, if non deterministic failure occurs.
        // makes login more robust on bad network connections
        while (retry && attemptCounter < maxAttempts) {
            try {
                reply = monitor!!.addAcctMgrErrorNum(url, name, pwd)
            } catch (e: RemoteException) {
                Logging.logException(Logging.Category.MONITOR, "ProjectAttachService.attachAcctMgr error: ", e)
            }

            Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.attachAcctMgr returned: $reply")

            when (reply.code) {
                ERR_GETHOSTBYNAME, ERR_CONNECT, ERR_HTTP_TRANSIENT -> attemptCounter++ // limit number of retries
                ERR_RETRY -> {
                }
                else -> retry = false
            }
            if (retry) {
                Thread.sleep(resources.getInteger(R.integer.attach_step_interval_ms).toLong())
            }
        }
        if (reply.isOK) {
            return reply
        }
        var info: AcctMgrInfo? = null
        try {
            info = monitor!!.acctMgrInfo
        } catch (e: RemoteException) {
            Logging.logException(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.attachAcctMgr error: ", e)
        }
        if (info == null) {
            return ErrorCodeDescription(-1)
        }

        Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                "ProjectAttachService.attachAcctMgr successful: " + info.acctMgrUrl +
                info.acctMgrName + info.isHavingCredentials)

        return reply
    }

    inner class ProjectAttachWrapper {
        // URL, manually inserted, or from projectInfo
        var url: String
        // chosen from list
        var info: ProjectInfo? = null
        // name of project in debug messages, do not use otherwise!
        var name: String
        // has to be downloaded, available if RESULT_READY
        var config: ProjectConfig? = null
        var result = Companion.RESULT_UNINITIALIZED

        constructor(info: ProjectInfo) {
            this.info = info
            name = info.name
            url = info.url
        }

        constructor(url: String) {
            this.url = url
            name = url
        }

        val resultDescription: String
            get() = when (result) {
                RESULT_UNINITIALIZED -> getString(R.string.attachproject_login_loading)
                RESULT_READY -> getString(R.string.attachproject_credential_input_sing_desc)
                RESULT_ONGOING -> getString(R.string.attachproject_working_attaching) + " " + name + "..."
                RESULT_UNDEFINED, RESULT_CONFIG_DOWNLOAD_FAILED -> getString(R.string.attachproject_conflict_undefined)
                RESULT_NAME_NOT_UNIQUE -> getString(R.string.attachproject_conflict_not_unique)
                RESULT_BAD_PASSWORD -> getString(R.string.attachproject_conflict_bad_password)
                RESULT_UNKNOWN_USER -> if (config!!.clientAccountCreationDisabled) {
                    getString(R.string.attachproject_conflict_unknown_user_creation_disabled)
                } else {
                    getString(R.string.attachproject_conflict_unknown_user)
                }
                else -> ""
            }

        /**
         * Attaches this project to BOINC client.
         * Account lookup/registration using credentials set at service.
         *
         * Using registration RPC if client side registration is enabled,
         * succeeds also if account exists and password is correct.
         *
         * Using login RPC if client side registration is disabled.
         *
         * Attaches project if account lookup succeeded.
         *
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return returns status conflict
         */
        fun lookupAndAttach(forceLookup: Boolean): Int {
            var isForceLookup = forceLookup

            Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachWrapper.attach: attempting: $name")

            // check if project config is loaded, return if not.
            // activity needs to check, wait and re-try
            if (result == Companion.RESULT_UNINITIALIZED || !projectConfigRetrievalFinished || config == null) {
                Logging.logError(Logging.Category.PROJECT_SERVICE, "ProjectAttachWrapper.attach: no projectConfig for: $name")

                result = Companion.RESULT_UNDEFINED
                return Companion.RESULT_UNDEFINED
            }
            result = Companion.RESULT_ONGOING

            var statusCredentials : AccountOut? = null

            var retry = true
            while (retry) {
                retry = false
                // get credentials
                // check if project allows registration
                statusCredentials = if (isForceLookup || config!!.clientAccountCreationDisabled) {
                    // registration disabled, e.g. WCG
                    Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                            "AttachProjectAsyncTask: account creation disabled, try login. for: " + config!!.name)

                    login()
                } else {
                    // registration enabled
                    register()
                }

                Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                        "AttachProjectAsyncTask: retrieving credentials returned: " +
                        statusCredentials!!.errorNum + ":" + statusCredentials.errorMsg +
                        ". for: " + config!!.name)

                // check success
                @Suppress("SENSELESS_COMPARISON")
                if (statusCredentials == null) {
                    Logging.logError(Logging.Category.GUI_ACTIVITY,
                        "AttachProjectAsyncTask: credential retrieval failed, is null, for: $name")

                    result = Companion.RESULT_UNDEFINED
                    return Companion.RESULT_UNDEFINED
                } else if (statusCredentials.errorNum != ERR_OK) {
                    Logging.logError(Logging.Category.PROJECT_SERVICE, "AttachProjectAsyncTask: credential retrieval failed, returned error: " +
                            statusCredentials.errorNum)

                    return when (statusCredentials.errorNum) {
                        ERR_DB_NOT_UNIQUE -> {
                            result = Companion.RESULT_NAME_NOT_UNIQUE
                            Companion.RESULT_NAME_NOT_UNIQUE
                        }
                        ERR_BAD_PASSWD -> {
                            result = Companion.RESULT_BAD_PASSWORD
                            Companion.RESULT_BAD_PASSWORD
                        }
                        ERR_DB_NOT_FOUND -> {
                            result = Companion.RESULT_UNKNOWN_USER
                            Companion.RESULT_UNKNOWN_USER
                        }
                        else -> {
                            if (!isForceLookup) {
                                retry = true
                                isForceLookup = true
                                continue
                            }

                            Logging.logWarning(Logging.Category.MONITOR,
                                "AttachProjectAsyncTask: unable to map error number, returned error: " +
                                    statusCredentials.errorNum)

                            result = Companion.RESULT_UNDEFINED
                            Companion.RESULT_UNDEFINED
                        }
                    }
                }
            }

            if (statusCredentials == null) {
                Logging.logError(Logging.Category.PROJECT_SERVICE, "AttachProjectAsyncTask: credential retrieval failed, is null, for: $name")

                result = Companion.RESULT_UNDEFINED
                return Companion.RESULT_UNDEFINED
            }

            // attach project
            val statusAttach = attach(statusCredentials.authenticator)

            Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                    "AttachProjectAsyncTask: attach returned: " + statusAttach + ". for: " + config!!.name)

            if (!statusAttach) {
                result = Companion.RESULT_UNDEFINED
                return Companion.RESULT_UNDEFINED
            }
            result = Companion.RESULT_SUCCESS
            return Companion.RESULT_SUCCESS
        }

        /**
         * Attempts account registration with the credentials previously set in service.
         * Registration also succeeds if account exists and password is correct.
         *
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return credentials
         */
        private fun register(): AccountOut? {
            var credentials: AccountOut? = null
            var retry = true
            var attemptCounter = 0
            val maxAttempts = resources.getInteger(R.integer.attach_creation_retries)
            // retry a defined number of times, if non deterministic failure occurs.
            // makes login more robust on bad network connections
            while (retry && attemptCounter < maxAttempts) {
                if (mIsBound) {
                    try {
                        credentials = monitor!!.createAccountPolling(getAccountIn(email, user, pwd))
                    } catch (e: RemoteException) {
                        Logging.logException(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.register error: ", e)
                    }
                }
                if (credentials == null) {
                    // call failed
                    Logging.logWarning(Logging.Category.PROJECT_SERVICE, "ProjectAttachWrapper.register register: auth null, retry...")

                    attemptCounter++ // limit number of retries
                } else {
                    Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                            "ProjectAttachWrapper.register returned: " + config!!.errorNum + " for " + name)

                    when (config!!.errorNum) {
                        ERR_GETHOSTBYNAME, ERR_CONNECT, ERR_HTTP_TRANSIENT -> attemptCounter++ // limit number of retries
                        ERR_RETRY -> {
                        }
                        else -> retry = false
                    }
                }
            }
            return credentials
        }

        /**
         * Attempts account lookup with the credentials previously set in service.
         *
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return credentials
         */
        fun login(): AccountOut? {
            var credentials: AccountOut? = null
            var retry = true
            var attemptCounter = 0
            val maxAttempts = resources.getInteger(R.integer.attach_login_retries)
            // retry a defined number of times, if non deterministic failure occurs.
            // makes login more robust on bad network connections
            while (retry && attemptCounter < maxAttempts) {
                if (mIsBound) {
                    try {
                        credentials = monitor!!.lookupCredentials(getAccountIn(email, user, pwd))
                    } catch (e: RemoteException) {
                        Logging.logException(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.login error: ", e)
                    }
                }
                if (credentials == null) {
                    // call failed
                    Logging.logWarning(Logging.Category.PROJECT_SERVICE, "ProjectAttachWrapper.login failed: auth null, retry...")

                    attemptCounter++ // limit number of retries
                } else {
                    Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachWrapper.login returned: " + config!!.errorNum + " for " + name)

                    when (config!!.errorNum) {
                        ERR_GETHOSTBYNAME, ERR_HTTP_TRANSIENT, ERR_CONNECT -> attemptCounter++ // limit number of retries
                        ERR_RETRY -> {
                        }
                        else -> retry = false
                    }
                }
                if (retry) {
                    Thread.sleep(resources.getInteger(R.integer.attach_step_interval_ms).toLong())
                }
            }
            return credentials
        }

        private fun attach(authenticator: String?): Boolean {
            if (mIsBound) {
                try {
                    return monitor!!.attachProject(config!!.masterUrl, config!!.name, authenticator)
                } catch (e: RemoteException) {
                    Logging.logException(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.attach error: ", e)
                }
            }
            return false
        }

        private fun getAccountIn(email: String, user: String, pwd: String): AccountIn {
            return AccountIn(config!!.secureUrlIfAvailable, email, user, pwd, "",
                    config!!.usesName)
        }
    }

    private suspend fun getProjectConfigs() {
        projectConfigRetrievalFinished = false

        withContext(Dispatchers.Default) {
            Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.GetProjectConfigAsync: number of selected projects: " +
                    selectedProjects.size)

            for (tmp in selectedProjects) {
                Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                        "ProjectAttachService.GetProjectConfigAsync: configuration download started for: " +
                        tmp.name + " with URL: " + tmp.url)

                val config = getProjectConfig(tmp.url)
                if (config?.errorNum == ERR_OK) {
                    Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                            "ProjectAttachService.GetProjectConfigAsync: configuration download succeeded for: " +
                            tmp.name)

                    tmp.config = config
                    tmp.name = config.name
                    tmp.result = RESULT_READY
                } else {
                    // error occurred
                    Logging.logError(Logging.Category.PROJECT_SERVICE,
                            "ProjectAttachService.GetProjectConfigAsync: could not load configuration for: " +
                            tmp.name)

                    tmp.result = RESULT_CONFIG_DOWNLOAD_FAILED
                }
            }

            Logging.logDebug(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.GetProjectConfigAsync: end.")
        }

        projectConfigRetrievalFinished = true
    }

    private suspend fun getProjectConfig(url: String): ProjectConfig? {
        val maxAttempts = resources.getInteger(R.integer.attach_get_project_config_retries)
        var config: ProjectConfig? = null
        var retry = true
        var attemptCounter = 0
        // retry a defined number of times, if non deterministic failure occurs.
        // makes login more robust on bad network connections
        while (retry && attemptCounter < maxAttempts) {
            if (mIsBound) {
                try {
                    config = monitor!!.getProjectConfigPolling(url)
                } catch (e: RemoteException) {
                    Logging.logException(Logging.Category.PROJECT_SERVICE, "ProjectAttachService.getProjectConfig error: ", e)
                }
            }
            if (config == null) {
                // call failed
                Logging.logWarning(Logging.Category.PROJECT_SERVICE,
                        "ProjectAttachWrapper.getProjectConfig failed: config null, mIsBound: " + mIsBound +
                        " for " + url + ". Retry...")

                attemptCounter++ // limit number of retries
            } else {
                Logging.logDebug(Logging.Category.PROJECT_SERVICE,
                        "GetProjectConfigsAsync.getProjectConfig returned: " + config.errorNum + " for " + url)

                when (config.errorNum) {
                    ERR_GETHOSTBYNAME, ERR_HTTP_TRANSIENT -> attemptCounter++ // limit number of retries
                    ERR_RETRY -> { }
                    else -> retry = false
                }
            }
            if (retry) {
                delay(resources.getInteger(R.integer.attach_step_interval_ms).toLong())
            }
        }
        return config
    }

    companion object {
        // config not downloaded yet, download!
        const val RESULT_UNINITIALIZED = 0

        // config is available, project is ready to be attached
        const val RESULT_READY = 1

        // ongoing attach
        const val RESULT_ONGOING = 2

        // successful, -X otherwise
        const val RESULT_SUCCESS = 3
        const val RESULT_UNDEFINED = -1

        // registration failed, either password wrong or ID taken
        const val RESULT_NAME_NOT_UNIQUE = -2

        // login failed (creation disabled or login button pressed), password wrong
        const val RESULT_BAD_PASSWORD = -3

        // login failed (creation disabled or login button pressed), user does not exist
        const val RESULT_UNKNOWN_USER = -4

        // download of configuration failed, but required for attach (retry?)
        const val RESULT_CONFIG_DOWNLOAD_FAILED = -6
    }
}
