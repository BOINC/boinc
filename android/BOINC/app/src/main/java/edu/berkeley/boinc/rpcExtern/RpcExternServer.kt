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

package edu.berkeley.boinc.rpcExtern

import android.R.string
import android.content.Intent
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpcExtern.authenticate.RpcExternAuthorizeAes
import edu.berkeley.boinc.rpcExtern.authenticate.RpcExternAuthorizeMd5
import java.io.*
import java.net.InetAddress
import java.net.InetAddress.getByAddress
import java.net.InetAddress.getByName
import java.net.ServerSocket
import java.net.Socket
import java.net.SocketTimeoutException
import java.nio.ByteBuffer.allocate
import java.nio.ByteOrder.LITTLE_ENDIAN


class RpcExternServer : RpcClient() {
    val mAuthenticateMd5 = RpcExternAuthorizeMd5()
    val mRpcExternString = RpcExternString()
    val mRpcExtern = RpcExtern()
    val mThis = this
    var mWiFiIpInt = 0
    var mThreadID : Long = -1

    var mClientSocketAddress = ""

    var mServerRunning = false
    lateinit var mMonitor: Monitor
    var mSendUpdates = false

//    val DEFAULT_PORT = 31416
    lateinit var mSettingsData : RpcSettingsData
//    var serverPort = DEFAULT_PORT
    var mConnectServerSocket = false
    var mExternPasswrdAes :String = ""
    var mThreadMakeCon: Thread? = null
    var mMessage: String = ""
    var mMessageIp: String = ""

    fun start(wiFiIp: Int, monitorIn: Monitor, socketAddress: String, data: RpcSettingsData)
    {
        if (mServerRunning)
        {
            return  // prevent starting more than once
        }
        mWiFiIpInt = wiFiIp
        mClientSocketAddress = socketAddress
        mServerRunning = true
        mMonitor = monitorIn
        mSettingsData = data

        if (mSettingsData.externEnabled && mWiFiIpInt > 0 ) {
            sendToApplication("START") // Start
            mThreadMakeCon = Thread(mainThreadLoop())
            mThreadMakeCon!!.start()
            mThreadID = mThreadMakeCon!!.id
        }
        else
        {
            if (mWiFiIpInt == 0)
            {
                sendToApplication("NOWIFI")
            }
            else {
                sendToApplication("IDLE")
            }
        }
    }

    fun update(wiFiIp: Int, data: RpcSettingsData?)
    {
        if ((wiFiIp == mWiFiIpInt) && (data == null))
        {
            return  // sends multiple updates on wifi change
        }

        sendToApplication("CLOSING")
        mWiFiIpInt = wiFiIp

        // wait for the thread to signal it has shut down
        if (mThreadMakeCon != null) {
            while (mThreadMakeCon!!.isAlive) {
                val id = mThreadMakeCon!!.id
                if (id != mThreadID)    // for debugging, checking if we find the same thread
                {
                }
                if (mThreadMakeCon == null) {
                    break
                }
                mThreadMakeCon!!.interrupt()    // signal thread to shutdown
                Thread.sleep(500)  // wait for things to shut down
            }
        }
        mThreadMakeCon = null
        // Thread stopped

        // Set the new data before the Thread starts
        if (data != null) {
            mSettingsData = data
        }

        // Start the connection thread if enabled and there is WiFi
        if ((mSettingsData.externEnabled) && mWiFiIpInt > 0) {
            mConnectServerSocket = true // restart the server socket
            sendToApplication("START") // Start
            mThreadMakeCon = Thread(mainThreadLoop())
            mThreadMakeCon!!.start()
            mThreadID = mThreadMakeCon!!.id
        }
        else
        {
            if (mWiFiIpInt == 0)
            {
                sendToApplication("NOWIFI")
            }
            else {
                sendToApplication("IDLE")
            }
        }
    }

    // The SettingsFragment send a start when it's visible and stop when it's done
    fun command(data: String)
    {
        when (data)
        {
            "START_UPDATE" -> {
                mSendUpdates = true
                sendToApplication(mMessage, mMessageIp)
            }
            "STOP_UPDATE" -> {
                mSendUpdates = false
                sendToApplication(mMessage, mMessageIp)
            }
        }
    }

    // Connection Thread
    private val inputs: DataInputStream? = null
    inner class mainThreadLoop : Runnable {
        val settingsData = mSettingsData
        val ipAllowedList = settingsData.ipAllowedList
        var externPasswrdAes = ""
        var externPasswrd = ""

        val boincEol = '\u0003'
        val socketTimeout = 5000
        var serverSocket: ServerSocket? = null
        var connectServerSocket = mConnectServerSocket
        var aes = RpcExternAuthorizeAes()
        var closeDownInterrupt = false
        var serverPort = mSettingsData.externPort
        var encryption = mSettingsData.externEncryption
        var bAuthorized = false
        val notAuthorizedReply = mRpcExternString.mRpcReplyBegin + "\n" + "\"<unauthorized/>\"" + mRpcExternString.mRpcReplyEnd + "\n" + boincEol
        var authorizedIp = ""
        var connectedIp = ""
        var timeout = 0

        lateinit var wiFiIp: InetAddress
        override fun run() {
            externPasswrd =  mSettingsData.externPasswrd
            var key = externPasswrd
            key += "leub[rehf!$&*()a"   // must be identical in the GUI
            externPasswrdAes = key.substring(0, 16) // string must be 16 bytes long. Warning using escaping char like \ may cause problems.
            val wiFiStr = getByAddress(allocate(4).order(LITTLE_ENDIAN).putInt(mWiFiIpInt).array()).hostAddress
            wiFiIp = getByName(wiFiStr)

            while (!closeDownInterrupt) {
                try {
                    connectToClient(false)  // make sure we are connected to the internal BOINC client
                    if (serverSocket == null)
                    {
                        connectServerSocket = true
                    }

                    if (connectServerSocket) {
                        if (serverSocket != null) { // close the socket to free it for connecting again
                            serverSocket!!.close()
                        }
                        serverSocket = ServerSocket(serverPort, 0, wiFiIp)  // backlog default
                        serverSocket!!.soTimeout = socketTimeout
//                        connectServerSocket = false
                    }
                    connectServerSocket = true
                    ConnectAndRead()
                } catch (e: InterruptedException) { // must be the first in catch
                    closeDownInterrupt = true
                } catch (e: Exception) {
                    var ii = 1 // debug break point
                    ii += 1
                }
            }
            serverSocket!!.close()  // interrupted
        }

        // Inside the connection Thread wait for the socket to get a request and leave accept
        private fun ConnectAndRead() {
            while (!closeDownInterrupt) {
                if (Thread.currentThread().isInterrupted) {
                    // InterruptedException doesn't catch in .accept but goes to InterruptedIOException
                    closeDownInterrupt = true
                    return // interrupted
                }
                try {
                    val socket = serverSocket!!.accept()
                    if (socket != null) {
                        socket.setSoTimeout(socketTimeout)
                        handleConnection(socket) // we have a connection, handle the handshake
                        //                       socket.close()
                    }
                } catch (e: InterruptedException) { // must be the first in catch
                    closeDownInterrupt = true
                    return // interrupted
                } catch (i: InterruptedIOException) {
                    // ends here on an external interrupt
                    return
                } catch (e: Exception) {
                    var ii = 1
                    ii += 1
                }
            }
        }

        // We accepted the socket request, now handle it
        var istream: InputStream? = null
        var out: BufferedWriter? = null
        fun handleConnection(socket: Socket) {
            var status = true
            Thread.sleep(500)  // don't allow flooding
            try {
                connectedIp = socket.remoteSocketAddress.toString()
                if (ipAllowedList.isNotEmpty()) {  // check the ip allow list
                    var ip = connectedIp
                    var bFound = false
                    ip = ip.replace("/", "")
                    ip = ip.substringBefore(':')
                    for (item in ipAllowedList) {
                        if (ip.contains(item)) {
                            bFound = true
                            break
                        }
                    }
                    if (!bFound) {  // if no match just close the socket and don't reply
                        closeSocket(socket, out)
                        sendToApplication("IPNOT", ip) //IP not allowed
                        return
                    }
                }
                if (!bAuthorized) {
                    sendToApplication("CON_NOT") // not connected
                }
                istream = socket.getInputStream()
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                closeSocket(socket, out)
                return  // interrupted
            } catch (e: Exception) {
                var ii = 1
                ii += 1
            }
            try {
                out = BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
            } catch (e: Exception) {
            }
            // process the connection
            while (status && !closeDownInterrupt) {
                try {
                    // check if this is the same IP as the authorized one
                    if (bAuthorized) {
                        if (!connectedIp.equals(authorizedIp)) {
                            bAuthorized = false // authorizing ip mismatch
                            sendToApplication("CONIPMATCH")
                            closeSocket(socket, out)
                            return
                        }
                        else
                        {
                            sendToApplication("CON_OK", authorizedIp ) // connected and authorized
                        }
                    }

                    status = ProcessRequests(istream, out)
                    if (Thread.currentThread().isInterrupted) {
                        // Catch doesn't always trigger InterruptedException
                        closeSocket(socket, out)
                        closeDownInterrupt = true
                        return
                    }
                } catch (e: InterruptedException) { // must be the first in catch
                    closeDownInterrupt = true
                    closeSocket(socket, out)
                    return  // interrupted
                } catch (e: Exception) {
                    var ii = 1
                    ii += 1
                }
            }
            try {
                closeSocket(socket, out)
            } catch (e: Exception) {
                var ii = 1
                ii += 1
            }
        }

        fun closeSocket(socket: Socket, out: BufferedWriter?): Boolean {
            try {
                out!!.close()
                socket.close()
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                return true
            } catch (e: Exception) {
                var ii = 1
                ii += 1
            }
            return false
        }

        // Get data from the socket and process it
        // return false = break
        fun ProcessRequests(istream: InputStream?, out: BufferedWriter?): Boolean {
            var reply = "?"
            try {
                val buffer = ByteArray(1024) // if the buffer is too short, the while loop reads it in parts
                var dataReadTotal = ""
                while (istream!!.read(buffer) != -1) {  // leave at the end of the stream, this generally never happens
                    val readString = String(buffer)
                    dataReadTotal += readString
                    if (readString.contains("\u0003")) // leave at the end marked by 3
                    {
                        break
                    }
                }
                if (dataReadTotal.length > 0) { // process the data we just read
                    timeout = 0
                    if (dataReadTotal.contains(mRpcExternString.mRpcRequestBegin)) {    // dirty but fast parser
                        if (dataReadTotal.contains("<auth")) {
                            bAuthorized = false // the GUI thinks it's not connected
                        }
                        if (bAuthorized) {
                            reply = sendRequestAndReplyRaw(dataReadTotal) + boincEol
                            if (reply.length < 10)  // no reply = connection lost
                            {
                                connectToClient(true)
                            }
                        } else {
                            if (encryption) {
                                reply = authorizeAes(dataReadTotal)
                            } else {
                                reply = authorizeMd5(dataReadTotal)
                            }
                        }
                    }
                }
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                return false // interrupted
            } catch (e: SocketTimeoutException) {
                timeout += 1
                if (timeout > 5)
                {
                    sendToApplication("TIMEOUT")
                    bAuthorized = false
                    return false
                }
                return true
            }
            catch (e: Exception) {
                return false
            }
            try {
                out!!.newLine() // send the reply over the socket
                out.write(reply)
                out.flush()
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                return false // interrupted
            } catch (e: Exception) {
                return false
            }

            return true
        }

        // Encrypted
        private fun authorizeAes(dataReadTotal: String): String {
            var reply = notAuthorizedReply
            try {
                bAuthorized = false
                if (dataReadTotal.contains("<authe1")) {
                    reply = aes.authe1(mExternPasswrdAes)
                } else {
                    if (dataReadTotal.contains("<authe2")) {
                        var auth = "<unauthorized/>"
                        val parser = RpcExternAuthParser()
                        parser.parse(dataReadTotal)
                        if (aes.auth2(parser.encrypted, mExternPasswrdAes)) {
                            auth = "<authorized/>"
                            bAuthorized = true
                            authorizedIp = connectedIp
                            sendToApplication("CON_OK") // connected and authorized
                        } else {
                            bAuthorized = false
                            authorizedIp = ""
                            sendToApplication("CON_NOT_AUTH") // connected not authorized
                        }
                        reply = mRpcExternString.mRpcReplyBegin + "\n" + auth + mRpcExternString.mRpcReplyEnd + "\n" + boincEol
                    }
                    else
                    {
                        if (dataReadTotal.contains("<auth1")) {
                            bAuthorized = false
                            sendToApplication("CON_ASK_AES_GET_MD5") // expect encrypted get md5
                        }
                    }
                }
                return reply
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
            } catch (e: Exception) {
            }
            return reply
        }

        // MD5
        private fun authorizeMd5(dataReadTotal: String): String {
            var reply = notAuthorizedReply
            try {
                bAuthorized = false
                if (dataReadTotal.contains("<auth1")) {
                    reply = mAuthenticateMd5.auth1(externPasswrd)  // send the legacy authenticator
                } else
                {
                    if (dataReadTotal.contains("<auth2")) {
                        var auth = "<unauthorized/>"
                        val parser = RpcExternAuthParser()
                        parser.parse(dataReadTotal)
                        if (mAuthenticateMd5.auth2(parser.nonceHash)) { // check if the passwords match
                            auth = "<authorized/>"
                            bAuthorized = true
                            authorizedIp = connectedIp
                            sendToApplication("CON_OK") // connected and authorized
                        } else {
                            bAuthorized = false
                            authorizedIp = ""
                            sendToApplication("CON_NOT_AUTH") // connected not authorized
                        }
                        reply = mRpcExternString.mRpcReplyBegin + "\n" + auth + mRpcExternString.mRpcReplyEnd + "\n" + boincEol
                    }
                    else
                    {
                        if (dataReadTotal.contains("<authe1")) {
                            bAuthorized = false
                            sendToApplication("CON_ASK_MD5_GET_AES") // expect md5 get encrypted
                        }
                    }
                }
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
            } catch (e: Exception) {
            }
            return reply
        }
        // connect to the internal BOINC client
        private fun connectToClient(alwaysConnect : Boolean)
        {
            try {
                if (!alwaysConnect && isConnected) return
                val token = mRpcExtern.readAuthToken(mMonitor.boincWorkingDir + mMonitor.fileNameGuiAuthentication)
                Thread.sleep(2000) // wait for things to settle
                mRpcExtern.connectClient(mThis, mClientSocketAddress, token)
            }  catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
            } catch (e: Exception) {
            }
        }
    }

    // send broadcast to the SettingsFragment
    fun sendToApplication(dataString: String, ipString: String = "")
    {
        try {
            mMessage = dataString
            mMessageIp = ipString
            if (!mSendUpdates)
            {
                return
            }
            val intent = Intent()
            intent.action = "RPC_EXTERN_FROM_CONNECTION"
            intent.putExtra("data", dataString)
            intent.putExtra("ip", ipString)
            intent.flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES
            // make sure this is only send locally but the LocalBroadcast seems to fail
            mMonitor.sendBroadcast(intent)
//        LocalBroadcastManager.getInstance(BOINCActivity.appContext!!).sendBroadcast(intent)
        } catch (e: Exception) {
            var ii = 1  // debug
            ii +=1
        }
    }
}

