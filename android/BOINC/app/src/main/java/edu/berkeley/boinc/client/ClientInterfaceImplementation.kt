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

import androidx.annotation.VisibleForTesting
import edu.berkeley.boinc.rpc.AccountIn
import edu.berkeley.boinc.rpc.AccountManager
import edu.berkeley.boinc.rpc.AccountOut
import edu.berkeley.boinc.rpc.AcctMgrRPCReply
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.Message
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.ProjectConfig
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpc.Transfer
import edu.berkeley.boinc.utils.ERR_IN_PROGRESS
import edu.berkeley.boinc.utils.ERR_OK
import edu.berkeley.boinc.utils.Logging.Category.CLIENT
import edu.berkeley.boinc.utils.Logging.logDebug
import edu.berkeley.boinc.utils.Logging.logError
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.Logging.logVerbose
import java.io.BufferedReader
import java.io.File
import java.io.FileNotFoundException
import java.io.FileReader
import java.io.IOException
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Class implements RPC commands with the client.
 * Extends RpcClient with polling, re-try and other mechanisms
 * Most functions can block executing thread, do not call them from UI thread!
 */
@VisibleForTesting
@Singleton
open class ClientInterfaceImplementation @Inject constructor(private val clientStatus: ClientStatus) :
    RpcClient() {
    // interval between polling retries in ms
    private val minRetryInterval = 1000

    /**
     * Reads authentication key from specified file path and authenticates GUI for advanced RPCs with the client
     *
     * @param authFilePath absolute path to file containing gui authentication key
     * @return success
     */
    fun authorizeGuiFromFile(authFilePath: String): Boolean {
        val authToken = readAuthToken(authFilePath)
        return authorize(authToken)
    }

    /**
     * Sets run mode of BOINC client
     *
     * @param mode see class BOINCDefs
     * @return success
     */
    fun setRunMode(mode: Int): Boolean {
        return setRunMode(mode, 0.0)
    }

    /**
     * Sets network mode of BOINC client
     *
     * @param mode see class BOINCDefs
     * @return success
     */
    fun setNetworkMode(mode: Int): Boolean {
        return setNetworkMode(mode, 0.0)
    }

    /**
     * Writes the given GlobalPreferences via RPC to the client. After writing, the active preferences are read back and written to ClientStatus.
     *
     * @param prefs new target preferences for the client
     * @return success
     */
    fun setGlobalPreferences(prefs: GlobalPreferences?): Boolean {
        val retval1 = setGlobalPrefsOverrideStruct(prefs) //set new override settings
        val retval2 = readGlobalPrefsOverride() //trigger reload of override settings
        if (!retval1 || !retval2) {
            return false
        }
        val workingPrefs = globalPrefsWorkingStruct
        if (workingPrefs != null) {
            clientStatus.setPrefs(workingPrefs)
            return true
        }
        return false
    }

    /**
     * Reads authentication token for GUI RPC authentication from file
     *
     * @param authFilePath absolute path to file containing GUI RPC authentication
     * @return GUI RPC authentication code
     */
    fun readAuthToken(authFilePath: String): String {
        var authKey = ""
        try {
            BufferedReader(FileReader(File(authFilePath))).use { br ->
                authKey = br.readLine()
            }
        } catch (fnfe: FileNotFoundException) {
            logException(CLIENT, "Auth file not found: ", fnfe)
        } catch (ioe: IOException) {
            logException(CLIENT, "IOException: ", ioe)
        }

        logDebug(
            CLIENT,
            "Authentication key acquired. length: $authKey.length"
        )

        return authKey
    }

    /**
     * Reads project configuration for specified master URL.
     *
     * @param url master URL of the project
     * @return project configuration information
     */
    fun getProjectConfigPolling(url: String?): ProjectConfig? {
        var config: ProjectConfig? = null

        val success = getProjectConfig(url) //asynchronous call
        if (success) { //only continue if attach command did not fail
            // verify success of getProjectConfig with poll function
            var loop = true
            while (loop) {
                loop = false
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                config = projectConfigPoll
                if (config == null) {
                    logError(
                        CLIENT,
                        "ClientInterfaceImplementation.getProjectConfigPolling: returned null."
                    )

                    return null
                }
                if (config.errorNum == ERR_IN_PROGRESS) {
                    loop = true //no result yet, keep looping
                } else {
                    //final result ready
                    if (config.errorNum == 0) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.getProjectConfigPolling: ProjectConfig retrieved: " +
                                    config.name
                        )
                    } else {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.getProjectConfigPolling: final result with error_num: " +
                                    config.errorNum
                        )
                    }
                }
            }
        }
        return config
    }

    /**
     * Attaches project, requires authenticator
     *
     * @param url           URL of project to be attached, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
     * @param projectName   name of project as shown in the manager
     * @param authenticator user authentication key, has to be obtained first
     * @return success
     */
    fun attachProject(url: String?, projectName: String?, authenticator: String?): Boolean {
        val success =
            projectAttach(url, authenticator, projectName) //asynchronous call to attach project
        if (success) {
            // verify success of projectAttach with poll function
            var reply = projectAttachPoll()
            while (reply != null && reply.errorNum ==
                ERR_IN_PROGRESS
            ) { // loop as long as reply.error_num == BOINCErrors.ERR_IN_PROGRESS
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                reply = projectAttachPoll()
            }
            return (reply != null && reply.errorNum == ERR_OK)
        } else {
            logDebug(CLIENT, "rpc.projectAttach failed.")
        }
        return false
    }

    /**
     * Checks whether project of given master URL is currently attached to BOINC client
     *
     * @param url master URL of the project
     * @return true if attached
     */
    fun checkProjectAttached(url: String): Boolean {
        try {
            val attachedProjects = projectStatus
            for ((masterURL) in attachedProjects) {
                logVerbose(
                    CLIENT,
                    "$masterURL vs $url"
                )

                if (masterURL == url) {
                    return true
                }
            }
        } catch (e: Exception) {
            logException(CLIENT, "ClientInterfaceImplementation.checkProjectAttached() error: ", e)
        }
        return false
    }

    /**
     * Looks up account credentials for given user data.
     * Contains authentication key for project attachment.
     *
     * @param credentials account credentials
     * @return account credentials
     */
    fun lookupCredentials(credentials: AccountIn?): AccountOut? {
        var auth: AccountOut? = null
        val success = lookupAccount(credentials) //async
        if (success) {
            // get authentication token from lookupAccountPoll
            var loop = true
            while (loop) {
                loop = false
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                auth = lookupAccountPoll()
                if (auth == null) {
                    logError(
                        CLIENT,
                        "ClientInterfaceImplementation.lookupCredentials: returned null."
                    )

                    return null
                }
                if (auth.errorNum == ERR_IN_PROGRESS) {
                    loop = true //no result yet, keep looping
                } else {
                    //final result ready
                    if (auth.errorNum == 0) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.lookupCredentials: authenticator retrieved."
                        )
                    } else {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.lookupCredentials: final result with error_num: " +
                                    auth.errorNum
                        )
                    }
                }
            }
        } else {
            logDebug(CLIENT, "rpc.lookupAccount failed.")
        }
        return auth
    }

    /**
     * Runs transferOp for a list of given transfers.
     * E.g. batch pausing of transfers
     *
     * @param transfers list of transferred operation gets executed for
     * @param operation see BOINCDefs
     * @return success
     */
    fun transferOperation(transfers: List<Transfer>, operation: Int): Boolean {
        var success = true
        for ((name, projectUrl) in transfers) {
            success = success && transferOp(operation, projectUrl, name)

            logDebug(
                CLIENT,
                "transfer: $name $success"
            )
        }
        return success
    }

    /**
     * Creates account for given user information and returns account credentials if successful.
     *
     * @param information account credentials
     * @return account credentials (see status inside, to check success)
     */
    fun createAccountPolling(information: AccountIn?): AccountOut? {
        var auth: AccountOut? = null

        val success = createAccount(information) //asynchronous call to attach project
        if (success) {
            var loop = true
            while (loop) {
                loop = false
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                auth = createAccountPoll()
                if (auth == null) {
                    logError(
                        CLIENT,
                        "ClientInterfaceImplementation.createAccountPolling: returned null."
                    )

                    return null
                }
                if (auth.errorNum == ERR_IN_PROGRESS) {
                    loop = true //no result yet, keep looping
                } else {
                    //final result ready
                    if (auth.errorNum == 0) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.createAccountPolling: authenticator retrieved."
                        )
                    } else {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.createAccountPolling: final result with error_num: "
                                    + auth.errorNum
                        )
                    }
                }
            }
        } else {
            logDebug(CLIENT, "rpc.createAccount returned false.")
        }
        return auth
    }

    /**
     * Adds account manager to BOINC client.
     * There can only be a single account manager be active at a time.
     *
     * @param url      URL of account manager
     * @param userName user name
     * @param pwd      password
     * @return status of attachment
     */
    fun addAcctMgr(url: String?, userName: String?, pwd: String?): AcctMgrRPCReply? {
        var reply: AcctMgrRPCReply? = null
        val success = acctMgrRPC(url, userName, pwd)
        if (success) {
            var loop = true
            while (loop) {
                reply = acctMgrRPCPoll()
                if (reply == null || reply.errorNum != ERR_IN_PROGRESS) {
                    loop = false
                    //final result ready
                    if (reply == null) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.addAcctMgr: failed, reply null."
                        )
                    } else {
                        logDebug(
                            CLIENT, "ClientInterfaceImplementation.addAcctMgr: returned " +
                                    reply.errorNum
                        )
                    }
                } else {
                    try {
                        Thread.sleep(minRetryInterval.toLong())
                    } catch (ignored: Exception) {
                    }
                }
            }
        } else {
            logDebug(CLIENT, "rpc.acctMgrRPC returned false.")
        }
        return reply
    }


    /**
     * Synchronized BOINC client projects with information of account manager.
     * Sequence copied from BOINC's desktop manager.
     *
     * @param url URL of account manager
     * @return success
     */
    fun synchronizeAcctMgr(url: String?): Boolean {
        // 1st get_project_config for account manager url
        var success = getProjectConfig(url)
        var reply: ProjectConfig?
        if (success) {
            var loop = true
            while (loop) {
                loop = false
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                reply = projectConfigPoll
                if (reply == null) {
                    logError(
                        CLIENT,
                        "ClientInterfaceImplementation.synchronizeAcctMgr: getProjectConfig returned null."
                    )

                    return false
                }
                if (reply.errorNum == ERR_IN_PROGRESS) {
                    loop = true //no result yet, keep looping
                } else {
                    //final result ready
                    if (reply.errorNum == 0) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.synchronizeAcctMgr: project config retrieved."
                        )
                    } else {
                        logDebug(
                            CLIENT, "ClientInterfaceImplementation.synchronize" +
                                    "AcctMgr: final result with error_num: " + reply.errorNum
                        )
                    }
                }
            }
        } else {
            logDebug(CLIENT, "rpc.getProjectConfig returned false.")
        }

        // 2nd acct_mgr_rpc with <use_config_file/>
        var reply2: AcctMgrRPCReply?
        success = acctMgrRPC() //asynchronous call to synchronize account manager
        if (success) {
            var loop = true
            while (loop) {
                loop = false
                try {
                    Thread.sleep(minRetryInterval.toLong())
                } catch (ignored: Exception) {
                }
                reply2 = acctMgrRPCPoll()
                if (reply2 == null) {
                    logError(
                        CLIENT,
                        "ClientInterfaceImplementation.synchronizeAcctMgr: acctMgrRPCPoll returned null."
                    )

                    return false
                }
                if (reply2.errorNum == ERR_IN_PROGRESS) {
                    loop = true //no result yet, keep looping
                } else {
                    //final result ready
                    if (reply2.errorNum == 0) {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.synchronizeAcctMgr: Account Manager reply retrieved."
                        )
                    } else {
                        logDebug(
                            CLIENT,
                            "ClientInterfaceImplementation.synchronizeAcctMgr: final result with error_num: " + reply2.errorNum
                        )
                    }
                }
            }
        } else {
            logDebug(CLIENT, "rpc.acctMgrRPC returned false.")
        }

        return true
    }

    override fun setCcConfig(ccConfig: String): Boolean {
        // set CC config and trigger re-read.
        super.setCcConfig(ccConfig)
        return super.readCcConfig()
    }

    /**
     * Returns List of event log messages
     *
     * @param seqNo  lower bound of sequence number
     * @param number number of messages returned max, can be less
     * @return list of messages
     */
    // returns given number of client messages, older than provided seqNo
    // if seqNo <= 0 initial data retrieval
    fun getEventLogMessages(seqNo: Int, number: Int): List<Message> {
        // determine oldest message seqNo for data retrieval
        var lowerBound: Int
        lowerBound = if (seqNo > 0) seqNo - number - 2
        else messageCount - number - 1 // can result in >number results, if client writes message btwn. here and rpc.getMessages!


        // less than desired number of messages available, adapt lower bound
        if (lowerBound < 0) lowerBound = 0
        val msgs = getMessages(lowerBound).toMutableList() // returns ever messages with seqNo > lowerBound

        if (seqNo > 0) {
            // remove messages that are >= seqNo
            msgs.removeIf { message: Message -> message.seqno >= seqNo }
        }

        if (msgs.isNotEmpty()) {
            logDebug(
                CLIENT, ("getEventLogMessages: returning array with " + msgs.size
                        + " entries. for lowerBound: " + lowerBound + " at 0: "
                        + msgs[0].seqno + " at " + (msgs.size - 1) + ": "
                        + msgs[msgs.size - 1].seqno)
            )
        }
        return msgs
    }

    /**
     * Returns list of projects from all_projects_list.xml that...
     * - support Android
     * - support CPU architecture
     * - are not yet attached
     *
     * @return list of attachable projects
     */
    fun getAttachableProjects(
        boincPlatformName: String,
        boincAltPlatformName: String
    ): List<ProjectInfo> {
        logDebug(
            CLIENT,
            "getAttachableProjects for platform: $boincPlatformName or $boincAltPlatformName"
        )

        val allProjectsList = allProjectsList // all_projects_list.xml
        val attachedProjects: List<Project> = state!!.projects // currently attached projects

        val attachableProjects: MutableList<ProjectInfo> =
            ArrayList() // array to be filled and returned

        if (allProjectsList.isEmpty()) return emptyList()

        //filter projects that do not support Android
        for (candidate in allProjectsList) {
            // check whether already attached
            var alreadyAttached = false
            for ((masterURL) in attachedProjects) {
                if (masterURL == candidate.url) {
                    alreadyAttached = true
                    break
                }
            }
            if (alreadyAttached) continue

            // project is not yet attached, check whether it supports CPU architecture
            for (supportedPlatform in candidate.platforms) {
                if (supportedPlatform.contains(boincPlatformName) ||
                    (boincAltPlatformName.isNotEmpty() && supportedPlatform.contains(
                        boincAltPlatformName
                    ))
                ) {
                    // project is not yet attached and does support platform
                    // add to list, if not already in it
                    if (!attachableProjects.contains(candidate)) attachableProjects.add(candidate)
                    break
                }
            }
        }

        logDebug(
            CLIENT, "getAttachableProjects: number of candidates found: " +
                    attachableProjects.size
        )

        return attachableProjects
    }

    val accountManagers: List<AccountManager>
        /**
         * Returns list of account managers from all_projects_list.xml
         *
         * @return list of account managers
         */
        get() {
            val accountManagers =
                accountManagersList // from all_projects_list.xml

            logDebug(
                CLIENT,
                "getAccountManagers: number of account managers found: " + accountManagers.size
            )

            return accountManagers
        }

    fun getProjectInfo(url: String): ProjectInfo? {
        val allProjectsList = allProjectsList // all_projects_list.xml
        for (tmp in allProjectsList) {
            if (tmp.url == url) return tmp
        }

        logError(
            CLIENT,
            "getProjectInfo: could not find info for: $url"
        )

        return null
    }

    fun setDomainName(deviceName: String?): Boolean {
        val success = setDomainNameRpc(deviceName)

        logDebug(
            CLIENT,
            "setDomainName: success $success"
        )

        return success
    }

    /**
     * Establishes socket connection to BOINC client.
     * Requirement for information exchange via RPC
     * @return success
     */
    fun connect(): Boolean {
        return open("localhost", 31416)
    }
}
