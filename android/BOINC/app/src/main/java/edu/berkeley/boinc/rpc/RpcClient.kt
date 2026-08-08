/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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
package edu.berkeley.boinc.rpc

import android.net.LocalSocket
import android.net.LocalSocketAddress
import android.util.Xml
import androidx.annotation.VisibleForTesting
import com.google.common.io.CharSource
import edu.berkeley.boinc.rpc.MessageCountParser.Companion.getSeqnoOfReply
import edu.berkeley.boinc.utils.Logging.Category.CLIENT
import edu.berkeley.boinc.utils.Logging.Category.RPC
import edu.berkeley.boinc.utils.Logging.Category.XML
import edu.berkeley.boinc.utils.Logging.Level.DEBUG
import edu.berkeley.boinc.utils.Logging.isLoggable
import edu.berkeley.boinc.utils.Logging.logDebug
import edu.berkeley.boinc.utils.Logging.logError
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.Logging.logVerbose
import edu.berkeley.boinc.utils.RUN_MODE_ALWAYS
import edu.berkeley.boinc.utils.RUN_MODE_AUTO
import edu.berkeley.boinc.utils.RUN_MODE_NEVER
import edu.berkeley.boinc.utils.RUN_MODE_RESTORE
import edu.berkeley.boinc.utils.readLineLimit
import java.io.BufferedReader
import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket
import java.time.Duration
import java.time.Instant
import java.util.*
import kotlin.text.Charsets.ISO_8859_1
import okio.BufferedSink
import okio.BufferedSource
import okio.ByteString.Companion.encodeUtf8
import okio.buffer
import okio.sink
import okio.source
import org.apache.commons.lang3.BooleanUtils
import org.xml.sax.SAXException
import org.xml.sax.helpers.DefaultHandler

/**
 * GUI RPC - the way the GUI can manage the BOINC core client and retrieve the information
 * from the client.
 * This tries to be the same as the original BOINC C++ GUI, but the names are rewritten
 * for the sake of naming conventions. Therefore, the original RPC_CLIENT becomes RpcClient,
 * get_cc_status() becomes getCcStatus(), etc.
 */
open class RpcClient {
    private var mSocket: LocalSocket? = null
    private var mTcpSocket: Socket? = null
    private var socketSource: BufferedSource? = null
    private var socketSink: BufferedSink? = null
    private val mReadBuffer = ByteArray(READ_BUF_SIZE)
    private var mResult = StringBuilder(RESULT_BUILDER_INIT_SIZE)

    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    var mRequest = StringBuilder(REQUEST_BUILDER_INIT_SIZE)

    private var mLastErrorMessage: String? = null

    internal class Auth1Parser(var mResult: StringBuilder) : DefaultHandler() {
        private var mCurrentElement: String? = null
        private var mNonceParsed = false

        @Throws(SAXException::class)
        override fun characters(ch: CharArray?, start: Int, length: Int) {
            super.characters(ch, start, length)
            // put it into StringBuilder
            mCurrentElement = String(ch!!, start, length)
        }

        @Throws(SAXException::class)
        override fun endElement(uri: String?, localName: String, qName: String?) {
            super.endElement(uri, localName, qName)
            if (localName.equals("nonce", ignoreCase = true) && !mNonceParsed) {
                mResult.append(mCurrentElement)
                mNonceParsed = true
            }
            mCurrentElement = null
        }
    }

    internal class Auth2Parser(var mResult: StringBuilder) : DefaultHandler() {
        private var mParsed = false

        @Throws(SAXException::class)
        override fun endElement(uri: String?, localName: String?, qName: String?) {
            super.endElement(uri, localName, qName)
            if (localName != null && (localName.equals(
                    AUTHORIZED,
                    ignoreCase = true
                ) || localName.equals(
                    UNAUTHORIZED, ignoreCase = true
                )) && !mParsed
            ) {
                mResult.append(localName.lowercase(Locale.getDefault()))
                mParsed = true
            }
        }
    }
    /*
     * Methods for connection - opening/closing/authorization/status
     */
    /**
     * Connect to BOINC core client via TCP Socket (abstract, "boinc_socket")
     *
     * @return true for success, false for failure
     */
    fun open(address: String?, port: Int): Boolean {
        closeOpenConnection()
        try {
            mTcpSocket = Socket()
            mTcpSocket!!.connect(InetSocketAddress(address, port), CONNECT_TIMEOUT)
            mTcpSocket!!.soTimeout = READ_TIMEOUT
        } catch (e: IOException) {
            logException(CLIENT, "connect failure: IO", e)
            mTcpSocket = null
            return false
        }
        return initBuffersFromSocket(false)
    }

    /**
     * Connect to BOINC core client via Unix Domain Socket (abstract, "boinc_socket")
     *
     * @return true for success, false for failure
     */
    fun open(socketAddress: String?): Boolean {
        closeOpenConnection()
        try {
            mSocket = LocalSocket()
            mSocket!!.connect(LocalSocketAddress(socketAddress))
            mSocket!!.soTimeout = READ_TIMEOUT
        } catch (e: IOException) {
            logException(CLIENT, "connect failure: IO", e)
            mSocket = null
            return false
        }
        return initBuffersFromSocket(true)
    }

    /**
     * Closes the currently opened connection to BOINC core client
     */
    @Synchronized
    fun close() {
        if (!isConnected) {
            // Not connected - just return (can be cleanup "for sure")
            return
        }
        try {
            socketSource!!.close()
        } catch (e: IOException) {
            logException(CLIENT, "input close failure", e)
        }
        try {
            socketSink!!.close()
        } catch (e: IOException) {
            logException(CLIENT, "output close failure", e)
        }
        try {
            if (mTcpSocket != null) mTcpSocket!!.close()
        } catch (e: IOException) {
            logException(CLIENT, "Tcp socket close failure", e)
        }
        try {
            if (mSocket != null) mSocket!!.close()
        } catch (e: IOException) {
            logException(CLIENT, "Local socket close failure", e)
        }
        logDebug(CLIENT, "close() - Socket closed")
        mSocket = null
        mTcpSocket = null
    }

    /**
     * Performs the BOINC authorization towards currently connected client.
     * The authorization uses MD5 hash of client's password and random value.
     * Clear-text password is never sent over network.
     *
     * @param password Clear text password used for authorization
     * @return true for success, false for failure
     */
    @Synchronized
    fun authorize(password: String): Boolean {
        if (!isConnected || password.isEmpty()) {
            return false
        }
        try {
            // Phase 1: get nonce
            sendRequest("<auth1/>\n")
            val auth1Rsp = receiveReply()
            mRequest.setLength(0)
            Xml.parse(auth1Rsp, Auth1Parser(mRequest)) // get nonce value
            // Operation: combine nonce & password, make MD5 hash
            mRequest.append(password)
            val nonceHash = mRequest.toString().encodeUtf8().md5().hex()
            // Phase 2: send hash to client
            mRequest.setLength(0)
            mRequest.append("<auth2>\n<nonce_hash>")
            mRequest.append(nonceHash)
            mRequest.append("</nonce_hash>\n</auth2>\n")
            sendRequest(mRequest.toString())
            val auth2Rsp = receiveReply()
            mRequest.setLength(0)
            Xml.parse(auth2Rsp, Auth2Parser(mRequest))
            if (mRequest.toString() != AUTHORIZED) {
                logDebug(RPC, "authorize() - Failure")
                return false
            }
        } catch (e: IOException) {
            logException(RPC, "error in authorize()", e)
            return false
        } catch (e: SAXException) {
            logError(XML, "Malformed XML received in authorize()")
            return false
        }
        logDebug(RPC, "authorize() - Successful")
        return true
    }

    /**
     * Checks the current state of connection
     *
     * @return true if connected to BOINC core client, false if not connected
     */
    val isConnected: Boolean
        get() = (mTcpSocket != null && mTcpSocket!!.isConnected) ||
                (mSocket != null && mSocket!!.isConnected)

    /**
     * Checks whether current connection can be used (data can be sent and received)
     * This is achieved by sending the empty BOINC command - so in case there is
     * socket still opened, but other side already closed connection, it will be detected.
     *
     * @return true if other side responds, false if data cannot be sent or received
     */
    @Synchronized
    fun connectionAlive(): Boolean {
        if (!isConnected) return false
        return try {
            // We just get the status via socket and do not parse reply
            sendRequest("<get_cc_status/>\n")
            val result = receiveReply()
            // If end of stream reached and no data were received in reply
            // we assume that socket is closed on the other side, most probably client shut down
            result.isNotEmpty()
        } catch (e: IOException) {
            false
        }
    }
    /*
     * Private methods for send/receive data
     */
    /**
     * Send RPC request to BOINC core client (XML-formatted)
     *
     * @param request The request itself
     * @throws IOException if error occurs when sending the request
     */
    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    @Throws(IOException::class)
    fun sendRequest(request: String) {
        logVerbose(RPC, "mRequest.capacity() = " + mRequest.capacity())
        logDebug(
            RPC,
            "Sending request: \n$request"
        )
        if (socketSink == null) {
            return
        }
        val requestBody =
            "<boinc_gui_rpc_request>\n$request</boinc_gui_rpc_request>\n\u0003"
        socketSink!!.writeString(requestBody, ISO_8859_1)
        socketSink!!.flush()
    }

    /**
     * Read the reply from BOINC core client
     *
     * @return the data read from socket
     * @throws IOException if error occurs when reading from socket
     */
    @Throws(IOException::class)
    protected fun receiveReply(): String {
        mResult.setLength(0)
        logVerbose(RPC, "mResult.capacity() = " + mResult.capacity())
        val start = Instant.now()

        // Speed is (with large data): ~ 45 KB/s for buffer size 1024
        //                             ~ 90 KB/s for buffer size 2048
        //                             ~ 95 KB/s for buffer size 4096
        // The chosen buffer size is 2048
        var bytesRead: Int
        if (socketSource == null) return mResult.toString() // empty string
        do {
            bytesRead = socketSource!!.read(mReadBuffer)
            if (bytesRead == -1) break
            mResult.append(String(mReadBuffer, 0, bytesRead))
            if (mReadBuffer[bytesRead - 1] == '\u0003'.code.toByte()) {
                // Last read byte marks the end of transfer
                mResult.setLength(mResult.length - 1)
                break
            }
        } while (true)
        if (isLoggable(DEBUG, RPC)) {
            var duration = Duration.between(Instant.now(), start).seconds.toFloat()
            val bytesCount = mResult.length.toLong()
            if (duration == 0f) duration = 0.001f
            logDebug(
                RPC,
                "Reading from socket took " + duration + " seconds, " +
                        bytesCount + " bytes read (" + bytesCount / duration +
                        " bytes/second)"
            )
        }
        logVerbose(RPC, "mResult.capacity() = " + mResult.capacity())
        if (isLoggable(DEBUG, RPC)) {
            val dbr = BufferedReader(CharSource.wrap(mResult).openStream())
            var dl: String?
            var ln = 0
            try {
                while ((dbr.readLineLimit(4096).also { dl = it }) != null) {
                    ++ln
                    logDebug(RPC, String.format("%4d: %s", ln, dl))
                }
            } catch (e: IOException) {
                logException(RPC, "RpcClient.receiveReply error: ", e)
            }
        }
        return mResult.toString()
    }
    /*
     * GUI RPC calls
     */
    /**
     * Performs get_cc_status RPC towards BOINC client
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val ccStatus: CcStatus?
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_cc_status/>\n")
                CcStatusParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getCcStatus()", e)
                e.printStackTrace()
                null
            }
        }

    /**
     * Performs get_file_transfers RPC towards BOINC client
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val fileTransfers: List<Transfer>
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_file_transfers/>\n")
                TransfersParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getFileTransfers()", e)
                emptyList()
            }
        }

    /**
     * Performs get_message_count RPC towards BOINC client
     * Returns highest seqNo
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val messageCount: Int
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_message_count/>\n")
                val seqNo = getSeqnoOfReply(receiveReply())
                logDebug(
                    RPC,
                    "RpcClient.getMessageCount returning: $seqNo"
                )
                seqNo
            } catch (e: IOException) {
                logException(RPC, "error in getMessageCount()", e)
                -1
            }
        }

    /**
     * Performs get_messages RPC towards BOINC client
     * Returns client messages that are more recent than seqNo param
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @Synchronized
    fun getMessages(seqNo: Int): List<Message> {
        mLastErrorMessage = null
        return try {
            val request: String = if (seqNo == 0) {
                // get all messages
                "<get_messages/>\n"
            } else {
                ("<get_messages>\n" +
                        " <seqno>" + seqNo + "</seqno>\n" +
                        "</get_messages>\n")
            }
            sendRequest(request)
            MessagesParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in getMessages()", e)
            emptyList()
        }
    }

    /**
     * Performs get_notices PRC towards BOINC client
     * Returns client messages that are more recent than seqNo param
     *
     * @return List of Notices
     */
    @Synchronized
    fun getNotices(seqNo: Int): List<Notice> {
        mLastErrorMessage = null
        return try {
            val request: String = if (seqNo == 0) {
                // get all notices
                "<get_notices/>\n"
            } else {
                ("<get_notices>\n" +
                        " <seqno>" + seqNo + "</seqno>\n" +
                        "</get_notices>\n")
            }
            sendRequest(request)
            NoticesParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in getMessages()", e)
            ArrayList()
        }
    }

    /**
     * Performs get_project_status RPC towards BOINC client
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val projectStatus: List<Project>
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_project_status/>\n")
                ProjectsParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getProjectStatus()", e)
                emptyList()
            }
        }

    /**
     * Performs get_results RPC towards BOINC client (all results)
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val results: List<Result>
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_results/>\n")
                ResultsParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getResults()", e)
                emptyList()
            }
        }

    /**
     * Performs get_state RPC towards BOINC client
     *
     * @return result of RPC call in case of success, null otherwise
     */
    @get:Synchronized
    val state: CcState?
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<get_state/>\n")
                CcStateParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getState()", e)
                null
            }
        }

    /**
     * Reports the current device state to the BOINC core client,
     * if not called frequently, BOINC core client will suspend
     *
     * @return true for success, false for failure
     */
    @Synchronized
    fun reportDeviceStatus(deviceStatus: DeviceStatusData): Boolean {
        mLastErrorMessage = null
        mRequest.setLength(0)
        mRequest.append("<report_device_status>\n <device_status>\n  <on_ac_power>")
        mRequest.append(BooleanUtils.toInteger(deviceStatus.isOnACPower))
        mRequest.append("</on_ac_power>\n  <on_usb_power>")
        mRequest.append(BooleanUtils.toInteger(deviceStatus.isOnUSBPower))
        mRequest.append("</on_usb_power>\n  <battery_charge_pct>")
        mRequest.append(deviceStatus.batteryChargePct)
        mRequest.append("</battery_charge_pct>\n  <battery_state>")
        mRequest.append(deviceStatus.batteryState)
        mRequest.append("</battery_state>\n  <battery_temperature_celsius>")
        mRequest.append(deviceStatus.batteryTemperatureCelsius)
        mRequest.append("</battery_temperature_celsius>\n  <wifi_online>")
        mRequest.append(BooleanUtils.toInteger(deviceStatus.isWiFiOnline))
        mRequest.append("</wifi_online>\n  <user_active>")
        mRequest.append(BooleanUtils.toInteger(deviceStatus.isUserActive))
        mRequest.append("</user_active>\n </device_status>\n</report_device_status>\n")
        try {
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: Exception) {
            logException(RPC, "RpcClient.reportDeviceStatus() error: ", e)
            return false
        }
    }

    /**
     * Reports the Android model as host info to the client
     *
     */
    @Synchronized
    fun setHostInfo(hostInfo: String?, version: String?) {
        mLastErrorMessage = null
        mRequest.setLength(0)
        mRequest.append("<set_host_info>\n")
        mRequest.append("  <host_info>\n")
        mRequest.append("    <product_name>")
        mRequest.append(hostInfo)
        mRequest.append("    </product_name>\n")
        mRequest.append("    <os_name>Android</os_name>")
        mRequest.append("    <os_version>")
        mRequest.append(version)
        mRequest.append("    </os_version>\n")
        mRequest.append("  </host_info>\n")
        mRequest.append("</set_host_info>\n")
        try {
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return
            mLastErrorMessage = parser.errorMessage
        } catch (e: Exception) {
            logException(RPC, "RpcClient.setHostInfo() error: ", e)
        }
    }

    /**
     * Reports the device name as host info to the client
     *
     * @param deviceName The name you want to set as device name.
     * @return true for success, false for failure
     */
    @Synchronized
    fun setDomainNameRpc(deviceName: String?): Boolean {
        mLastErrorMessage = null
        mRequest.setLength(0)
        mRequest.append("<set_host_info>\n")
        mRequest.append("  <host_info>\n")
        mRequest.append("    <domain_name>")
        mRequest.append(deviceName)
        mRequest.append("    </domain_name>\n")
        mRequest.append("  </host_info>\n")
        mRequest.append("</set_host_info>\n")
        try {
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: Exception) {
            logException(RPC, "RpcClient.setDomainNameRpc() error: ", e)
            return false
        }
    }

    /**
     * Triggers change of state of project in BOINC core client
     *
     * @param operation  operation to be triggered
     * @param projectUrl master URL of project
     * @return true for success, false for failure
     */
    @Synchronized
    fun projectOp(operation: Int, projectUrl: String): Boolean {
        try {
            val opTag: String
            when (operation) {
                PROJECT_UPDATE -> opTag = "project_update"
                PROJECT_SUSPEND -> opTag = "project_suspend"
                PROJECT_RESUME -> opTag = "project_resume"
                PROJECT_NNW -> opTag = "project_nomorework"
                PROJECT_ANW -> opTag = "project_allowmorework"
                PROJECT_DETACH -> opTag = "project_detach"
                PROJECT_RESET -> opTag = "project_reset"
                else -> {
                    logError(
                        RPC,
                        "projectOp() - unsupported operation: $operation"
                    )
                    return false
                }
            }
            val request = ("<" + opTag + ">\n" +
                    "<project_url>" + projectUrl + "</project_url>\n" +
                    "</" + opTag + ">\n")
            sendRequest(request)
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in projectOp()", e)
            return false
        }
    }

    /**
     * Creates account
     *
     * @param accountIn - account info
     * @return true for success, false for failure
     */
    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    @Synchronized
    fun createAccount(accountIn: AccountIn?): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<create_account>\n   <url>")
            mRequest.append(accountIn!!.url)
            mRequest.append("</url>\n   <email_addr>")
            mRequest.append(accountIn.emailAddress)
            mRequest.append("</email_addr>\n   <passwd_hash>")
            val string = accountIn.password + accountIn.emailAddress
            mRequest.append(string.encodeUtf8().md5().hex())
            mRequest.append("</passwd_hash>\n   <user_name>")
            if (accountIn.userName != null) mRequest.append(accountIn.userName)
            mRequest.append("</user_name>\n   <team_name>")
            if (accountIn.teamName != null) mRequest.append(accountIn.teamName)
            mRequest.append("</team_name>\n</create_account>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in createAccount()", e)
            return false
        }
    }

    /**
     * polling create account
     *
     * @return account output
     */
    @Synchronized
    fun createAccountPoll(): AccountOut? {
        return try {
            mRequest.setLength(0)
            mRequest.append("<create_account_poll/>")
            sendRequest(mRequest.toString())
            AccountOutParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in getCreateAccountPoll()", e)
            null
        }
    }

    /**
     * Looks up account
     *
     * @param accountIn - account info
     * @return true for success, false for failure
     */
    @Synchronized
    fun lookupAccount(accountIn: AccountIn?): Boolean {
        try {
            val id: String? = if (accountIn!!.usesName) accountIn.userName else accountIn.emailAddress
            mRequest.setLength(0)
            mRequest.append("<lookup_account>\n <url>")
            mRequest.append(accountIn.url)
            mRequest.append("</url>\n <email_addr>")
            mRequest.append(id!!.lowercase())
            mRequest.append("</email_addr>\n <passwd_hash>")
            mRequest.append((accountIn.password + id.lowercase()).encodeUtf8().md5().hex())
            mRequest.append("</passwd_hash>\n</lookup_account>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in lookupAccount()", e)
            return false
        }
    }

    /**
     * polling lookup account
     *
     * @return account output
     */
    @Synchronized
    fun lookupAccountPoll(): AccountOut? {
        return try {
            mRequest.setLength(0)
            mRequest.append("<lookup_account_poll/>")
            sendRequest(mRequest.toString())
            AccountOutParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in getLookupAccountPoll()", e)
            null
        }
    }

    /**
     * Attach to project
     *
     * @param url           project url
     * @param authenticator account key
     * @param name          project name
     * @return success
     */
    @Synchronized
    fun projectAttach(url: String?, authenticator: String?, name: String?): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<project_attach>\n   <project_url>")
            mRequest.append(url)
            mRequest.append("</project_url>\n   <authenticator>")
            mRequest.append(authenticator)
            mRequest.append("</authenticator>\n   <project_name>")
            mRequest.append(name)
            mRequest.append("</project_name>\n</project_attach>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in projectAttach()", e)
            return false
        }
    }

    /**
     * polling project attach
     *
     * @return project attach reply
     */
    @Synchronized
    fun projectAttachPoll(): ProjectAttachReply? {
        return try {
            mRequest.setLength(0)
            mRequest.append("<project_attach_poll/>")
            sendRequest(mRequest.toString())
            ProjectAttachReplyParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in projectAttachPoll()", e)
            null
        }
    }

    /**
     * performs acct_mgr_rpc towards client
     * attaches account manager to client
     * requires polling of status
     *
     * @param url    URL of project
     * @param name   user name
     * @param passwd password
     * @return success
     */
    @Synchronized
    fun acctMgrRPC(url: String?, name: String?, passwd: String?): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<acct_mgr_rpc>\n   <url>")
            mRequest.append(url)
            mRequest.append("</url>\n   <name>")
            mRequest.append(name)
            mRequest.append("</name>\n   <password>")
            mRequest.append(passwd)
            mRequest.append("</password>\n</acct_mgr_rpc>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply())
            mLastErrorMessage = parser!!.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in acctMgrRPC()", e)
            return false
        }
    }

    /**
     * performs acct_mgr_rpc with <use_config_file></use_config_file> instead of login information
     *
     * @return success
     */
    @Synchronized
    fun acctMgrRPC(): Boolean {
        return try {
            mRequest.setLength(0)
            mRequest.append("<acct_mgr_rpc>\n<use_config_file/>\n</acct_mgr_rpc>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply())
            mLastErrorMessage = parser!!.errorMessage
            parser.result
        } catch (e: IOException) {
            logException(RPC, "error in acctMgrRPC()", e)
            false
        }
    }

    /**
     * performs acct_mgr_rpc_poll towards client
     * polls status of acct_mgr_rpc
     *
     * @return status class AcctMgrRPCReply
     */
    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    @Synchronized
    fun acctMgrRPCPoll(): AcctMgrRPCReply? {
        return try {
            mRequest.setLength(0)
            mRequest.append("<acct_mgr_rpc_poll/>")
            sendRequest(mRequest.toString())
            AcctMgrRPCReplyParser.parse(receiveReply())
        } catch (e: IOException) {
            logException(RPC, "error in acctMgrRPCPoll()", e)
            null
        }
    }

    /**
     * performs acct_mgr_info towards client
     *
     * @return status class AcctMgrInfo
     */
    @get:Synchronized
    val acctMgrInfo: AcctMgrInfo?
        get() {
            mLastErrorMessage = null
            return try {
                sendRequest("<acct_mgr_info/>\n")
                AcctMgrInfoParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getAcctMgrInfo()", e)
                null
            }
        }

    @Synchronized
    fun getProjectConfig(url: String?): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<get_project_config>\n   <url>")
            mRequest.append(url)
            mRequest.append("</url>\n</get_project_config>\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in getProjectConfig()", e)
            return false
        }
    }

    @get:Synchronized
    val projectConfigPoll: ProjectConfig?
        get() {
            return try {
                mRequest.setLength(0)
                mRequest.append("<get_project_config_poll/>")
                sendRequest(mRequest.toString())
                ProjectConfigReplyParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getProjectConfigPoll()", e)
                null
            }
        }

    @get:Synchronized
    protected val allProjectsList: List<ProjectInfo>
        get() {
            return try {
                mRequest.setLength(0)
                mRequest.append("<get_all_projects_list/>")
                sendRequest(mRequest.toString())
                ProjectInfoParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getAllProjectsList()", e)
                emptyList()
            }
        }

    @get:Synchronized
    protected val accountManagersList: List<AccountManager>
        get() {
            return try {
                mRequest.setLength(0)
                mRequest.append("<get_all_projects_list/>")
                sendRequest(mRequest.toString())
                AccountManagerParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in getAccountManagersList()", e)
                emptyList()
            }
        }

    @get:Synchronized
    val globalPrefsWorkingStruct: GlobalPreferences?
        get() {
            return try {
                mRequest.setLength(0)
                mRequest.append("<get_global_prefs_working/>")
                sendRequest(mRequest.toString())
                GlobalPreferencesParser.parse(receiveReply())
            } catch (e: IOException) {
                logException(RPC, "error in globalPrefsWorking()", e)
                null
            }
        }

    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    @Synchronized
    fun setGlobalPrefsOverrideStruct(globalPrefs: GlobalPreferences?): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<set_global_prefs_override>\n<global_preferences>\n  <run_on_batteries>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs!!.runOnBatteryPower))
            mRequest.append("</run_on_batteries>\n  <battery_charge_min_pct>")
            mRequest.append(globalPrefs.batteryChargeMinPct)
            mRequest.append("</battery_charge_min_pct>\n  <battery_max_temperature>")
            mRequest.append(globalPrefs.batteryMaxTemperature)
            mRequest.append("</battery_max_temperature>\n  <run_gpu_if_user_active>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs.runGpuIfUserActive))
            mRequest.append("</run_gpu_if_user_active>\n  <run_if_user_active>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs.runIfUserActive))
            mRequest.append("</run_if_user_active>\n  <idle_time_to_run>")
            mRequest.append(globalPrefs.idleTimeToRun)
            mRequest.append("</idle_time_to_run>\n  <suspend_cpu_usage>")
            mRequest.append(globalPrefs.suspendCpuUsage)
            mRequest.append("</suspend_cpu_usage>\n  <start_hour>")
            mRequest.append(globalPrefs.cpuTimes.startHour)
            mRequest.append("</start_hour>\n  <end_hour>")
            mRequest.append(globalPrefs.cpuTimes.endHour)
            mRequest.append("</end_hour>\n  <net_start_hour>")
            mRequest.append(globalPrefs.netTimes.startHour)
            mRequest.append("</net_start_hour>\n  <net_end_hour>")
            mRequest.append(globalPrefs.netTimes.endHour)
            mRequest.append("</net_end_hour>\n  <max_ncpus_pct>")
            mRequest.append(globalPrefs.maxNoOfCPUsPct)
            mRequest.append("</max_ncpus_pct>\n  <leave_apps_in_memory>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs.leaveAppsInMemory))
            mRequest.append("</leave_apps_in_memory>\n  <dont_verify_images>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs.doNotVerifyImages))
            mRequest.append("</dont_verify_images>\n  <work_buf_min_days>")
            mRequest.append(globalPrefs.workBufMinDays)
            mRequest.append("</work_buf_min_days>\n  <work_buf_additional_days>")
            mRequest.append(globalPrefs.workBufAdditionalDays)
            mRequest.append("</work_buf_additional_days>\n  <disk_interval>")
            mRequest.append(globalPrefs.diskInterval)
            mRequest.append("</disk_interval>\n  <cpu_scheduling_period_minutes>")
            mRequest.append(globalPrefs.cpuSchedulingPeriodMinutes)
            mRequest.append("</cpu_scheduling_period_minutes>\n  <disk_max_used_gb>")
            mRequest.append(globalPrefs.diskMaxUsedGB)
            mRequest.append("</disk_max_used_gb>\n  <disk_max_used_pct>")
            mRequest.append(globalPrefs.diskMaxUsedPct)
            mRequest.append("</disk_max_used_pct>\n  <disk_min_free_gb>")
            mRequest.append(globalPrefs.diskMinFreeGB)
            mRequest.append("</disk_min_free_gb>\n  <ram_max_used_busy_pct>")
            mRequest.append(globalPrefs.ramMaxUsedBusyFrac)
            mRequest.append("</ram_max_used_busy_pct>\n  <ram_max_used_idle_pct>")
            mRequest.append(globalPrefs.ramMaxUsedIdleFrac)
            mRequest.append("</ram_max_used_idle_pct>\n  <max_bytes_sec_up>")
            mRequest.append(globalPrefs.maxBytesSecUp)
            mRequest.append("</max_bytes_sec_up>\n  <max_bytes_sec_down>")
            mRequest.append(globalPrefs.maxBytesSecDown)
            mRequest.append("</max_bytes_sec_down>\n  <cpu_usage_limit>")
            mRequest.append(globalPrefs.cpuUsageLimit)
            mRequest.append("</cpu_usage_limit>\n  <daily_xfer_limit_mb>")
            mRequest.append(globalPrefs.dailyTransferLimitMB)
            mRequest.append("</daily_xfer_limit_mb>\n  <daily_xfer_period_days>")
            mRequest.append(globalPrefs.dailyTransferPeriodDays)
            mRequest.append("</daily_xfer_period_days>\n  <network_wifi_only>")
            mRequest.append(BooleanUtils.toInteger(globalPrefs.networkWiFiOnly))
            mRequest.append("</network_wifi_only>\n")

            // write days prefs
            var weekPrefs = globalPrefs.cpuTimes.weekPrefs
            for (i in weekPrefs.indices) {
                val timeSpan = weekPrefs[i] ?: continue
                mRequest.append("  <day_prefs>\n    <day_of_week>")
                mRequest.append(i)
                mRequest.append("</day_of_week>\n    <start_hour>")
                mRequest.append(timeSpan.startHour)
                mRequest.append("</start_hour>\n    <end_hour>")
                mRequest.append(timeSpan.endHour)
                mRequest.append("</end_hour>\n  </day_prefs>\n")
            }
            weekPrefs = globalPrefs.netTimes.weekPrefs
            for (i in weekPrefs.indices) {
                val timeSpan = weekPrefs[i] ?: continue
                mRequest.append("  <day_prefs>\n    <day_of_week>")
                mRequest.append(i)
                mRequest.append("</day_of_week>\n    <net_start_hour>")
                mRequest.append(timeSpan.startHour)
                mRequest.append("</net_start_hour>\n    <net_end_hour>")
                mRequest.append(timeSpan.endHour)
                mRequest.append("</net_end_hour>\n  </day_prefs>\n")
            }
            mRequest.append("</global_preferences>\n</set_global_prefs_override>\n")
            sendRequest(mRequest.toString())
            receiveReply()
            return true
        } catch (e: IOException) {
            logException(RPC, "error in setGlobalPrefsOverrideStruct()", e)
            return false
        }
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    @Synchronized
    fun readGlobalPrefsOverride(): Boolean {
        return try {
            mRequest.setLength(0)
            mRequest.append("<read_global_prefs_override/>")
            sendRequest(mRequest.toString())

            // TODO: handle errors
            receiveReply()
            true
        } catch (e: IOException) {
            logException(RPC, "error in setGlobalPrefsOverrideStruct()", e)
            false
        }
    }

    /**
     * Tells the BOINC core client to exit.
     */
    @Synchronized
    fun quit(): Boolean {
        try {
            sendRequest("<quit/>\n")
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return true
            mLastErrorMessage = parser.errorMessage
        } catch (e: IOException) {
            logException(RPC, "error in quit()", e)
        }
        return false
    }

    /**
     * Set the network mode
     *
     * @param mode     1 = always, 2 = auto, 3 = never, 4 = restore
     * @param duration If duration is zero, mode is permanent. Otherwise revert to
     * last permanent mode after duration seconds elapse.
     * @return true for success, false for failure
     */
    @Synchronized
    fun setNetworkMode(mode: Int, duration: Double): Boolean {
        val request = ("<set_network_mode>\n" +
                modeName(mode) + "\n" +
                "<duration>" + duration + "</duration>\n" +
                "</set_network_mode>\n")
        try {
            sendRequest(request)
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in setNetworkMode()", e)
            return false
        }
    }

    /**
     * Set the run mode
     *
     * @param mode     1 = always, 2 = auto, 3 = never, 4 = restore
     * @param duration If duration is zero, mode is permanent. Otherwise revert to
     * last permanent mode after duration seconds elapse.
     * @return true for success, false for failure
     */
    @Synchronized
    fun setRunMode(mode: Int, duration: Double): Boolean {
        val request = ("<set_run_mode>\n" +
                modeName(mode) + "\n" +
                "<duration>" + duration + "</duration>\n" +
                "</set_run_mode>\n")
        try {
            sendRequest(request)
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in setRunMode()", e)
            return false
        }
    }

    /**
     * Triggers operation on transfer in BOINC core client
     *
     * @param operation  operation to be triggered
     * @param projectUrl master URL of project
     * @param fileName   name of the file
     * @return true for success, false for failure
     */
    @Synchronized
    fun transferOp(operation: Int, projectUrl: String?, fileName: String?): Boolean {
        try {
            val opTag: String = when (operation) {
                TRANSFER_RETRY -> "retry_file_transfer"
                TRANSFER_ABORT -> "abort_file_transfer"
                else -> {
                    logException(
                        RPC,
                        "transferOp() - unsupported operation: $operation", null
                    )
                    return false
                }
            }
            mRequest.setLength(0)
            mRequest.append("<")
            mRequest.append(opTag)
            mRequest.append(">\n   <project_url>")
            mRequest.append(projectUrl)
            mRequest.append("</project_url>\n   <filename>")
            mRequest.append(fileName)
            mRequest.append("</filename>\n</")
            mRequest.append(opTag)
            mRequest.append(">\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in transferOp()", e)
            return false
        }
    }

    /**
     * Triggers operation on task in BOINC core client
     *
     * @param operation  operation to be triggered
     * @param projectUrl master URL of project
     * @return true for success, false for failure
     */
    fun resultOp(operation: Int, projectUrl: String?, resultName: String?): Boolean {
        try {
            val opTag: String = when (operation) {
                RESULT_SUSPEND -> "suspend_result"
                RESULT_RESUME -> "resume_result"
                RESULT_ABORT -> "abort_result"
                else -> {
                    logException(
                        RPC,
                        "resultOp() - unsupported operation: $operation", null
                    )
                    return false
                }
            }
            mRequest.setLength(0)
            mRequest.append("<")
            mRequest.append(opTag)
            mRequest.append(">\n   <project_url>")
            mRequest.append(projectUrl)
            mRequest.append("</project_url>\n   <name>")
            mRequest.append(resultName)
            mRequest.append("</name>\n</")
            mRequest.append(opTag)
            mRequest.append(">\n")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in transferOp()", e)
            return false
        }
    }

    @Synchronized
    open fun setCcConfig(ccConfig: String): Boolean {
        val request = ("<set_cc_config>\n" +
                ccConfig +
                "\n</set_cc_config>\n")
        try {
            sendRequest(request)
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in setCcConfig()", e)
            return false
        }
    }

    @Synchronized
    fun readCcConfig(): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<read_cc_config/>")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in readCcConfig()", e)
            return false
        }
    }

    @Synchronized
    fun runBenchmarks(): Boolean {
        try {
            mRequest.setLength(0)
            mRequest.append("<run_benchmarks/>")
            sendRequest(mRequest.toString())
            val parser = SimpleReplyParser.parse(receiveReply()) ?: return false
            mLastErrorMessage = parser.errorMessage
            return parser.result
        } catch (e: IOException) {
            logException(RPC, "error in runBenchmark()", e)
            return false
        }
    }

    private fun initBuffersFromSocket(isLocal: Boolean): Boolean {
        try {
            socketSource = (if (isLocal) mSocket!!.inputStream else mTcpSocket!!.getInputStream()).source()
                .buffer()
            socketSink = (if (isLocal) mSocket!!.outputStream else mTcpSocket!!.getOutputStream()).sink()
                .buffer()
        } catch (e: IllegalArgumentException) {
            logException(RPC, "connect failure: illegal argument", e)
            mSocket = null
            mTcpSocket = null
            return false
        } catch (e: IOException) {
            logException(RPC, "connect failure: IO", e)
            mSocket = null
            mTcpSocket = null
            return false
        } catch (e: Exception) {
            logException(RPC, "connect failure", e)
            mSocket = null
            mTcpSocket = null
            return false
        }
        logDebug(RPC, "Connected successfully")
        return true
    }

    private fun closeOpenConnection() {
        if (isConnected) {
            // Already connected
            logError(RPC, "Attempt to connect when already connected")
            // We better close current connection and reconnect (address/port could be different)
            close()
        }
        mSocket = null
        mTcpSocket = null
    }

    companion object {
        const val AUTHORIZED = "authorized"
        const val UNAUTHORIZED = "unauthorized"
        private const val CONNECT_TIMEOUT = 30000
        private const val READ_TIMEOUT = 15000 // 15s
        private const val READ_BUF_SIZE = 2048
        private const val RESULT_BUILDER_INIT_SIZE = 131072 // Yes, 128K
        private const val REQUEST_BUILDER_INIT_SIZE = 80
        const val PROJECT_UPDATE = 1
        const val PROJECT_SUSPEND = 2
        const val PROJECT_RESUME = 3
        const val PROJECT_NNW = 4
        const val PROJECT_ANW = 5
        const val PROJECT_DETACH = 6
        const val PROJECT_RESET = 7
        const val RESULT_SUSPEND = 10
        const val RESULT_RESUME = 11
        const val RESULT_ABORT = 12
        const val TRANSFER_RETRY = 20
        const val TRANSFER_ABORT = 21
        const val MGR_DETACH = 30
        const val MGR_SYNC = 31

        /*
     * Helper methods
     */
        private fun modeName(mode: Int): String {
            return when (mode) {
                RUN_MODE_ALWAYS -> "<always/>"
                RUN_MODE_AUTO -> "<auto/>"
                RUN_MODE_NEVER -> "<never/>"
                RUN_MODE_RESTORE -> "<restore/>"
                else -> ""
            }
        }
    }
}
