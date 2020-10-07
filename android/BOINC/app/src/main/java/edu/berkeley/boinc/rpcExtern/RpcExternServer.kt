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

import android.content.Intent
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.rpc.RpcClient
import java.net.InetAddress.*
import java.io.*
import java.net.InetAddress
import java.net.ServerSocket
import java.net.Socket
import java.nio.ByteBuffer.*
import java.nio.ByteOrder.*

class RpcExternServer : RpcClient() {
    val mAuthenticateMd5 = RpcExternAuthorizeMd5()
    val mRpcExternString = RpcExternString()
    val mRpcExtern = RpcExtern()
    val mThis = this
    var mWiFiIpInt = 0
    var mThreadID : Long = -1

    var mClientSocketAddress = ""
    var mBoicToken = ""

    var mServerRunning = false
    lateinit var mMonitor: Monitor
    var mSendUpdates = false

    val DEFAULT_PORT = 31416
    var mServerPort = DEFAULT_PORT
    var mExternEnabled = false
    var mExternEncryption = true
    var mExternPasswrd :String = ""
    var mIpAllowedList = ArrayList<String>()
//    var mServerSocket: ServerSocket? = null
    var mThreadMakeCon: Thread? = null
    var mMessage: String = ""


    fun start(wiFiIp: Int, monitorIn: Monitor, socketAddress: String, boincToken: String, data: RpcSettingsData)
    {
        if (mServerRunning)
        {
            return  // prevent starting more than once
        }
        mWiFiIpInt = wiFiIp
        mClientSocketAddress = socketAddress
        mBoicToken = boincToken
        mServerRunning = true
        try {
            mServerPort = data.externPort.toInt()
        } catch (e: Exception)
        {
            mServerPort = DEFAULT_PORT
        }
        try {
            mMonitor = monitorIn
            mExternEnabled = data.externEnabled
            mExternEncryption = data.externEncryption
            mExternPasswrd = data.externPasswrd
            mIpAllowedList = data.ipAllowedList
        } catch (e: Exception){
            mIpAllowedList.add("none")
        }

        if (mExternEnabled && mWiFiIpInt > 0 ) {
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
            mExternEnabled = data.externEnabled
            mExternEncryption = data.externEncryption
            mExternPasswrd = data.externPasswrd
            try {
                mServerPort = data.externPort.toInt()
            } catch (e: Exception) {
                mServerPort = DEFAULT_PORT
            }
            mIpAllowedList = data.ipAllowedList
        }

        // Start the connection thread if enabled and there is WiFi
        if (mExternEnabled && mWiFiIpInt > 0) {
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
                sendToApplication(mMessage)
            }
            "STOP_UPDATE" -> {
                mSendUpdates = false
                sendToApplication(mMessage)
            }
        }
    }

    // Connection Thread
    private val inputs: DataInputStream? = null
    inner class mainThreadLoop : Runnable {
        var serverSocket: ServerSocket? = null
        var closeDownInterrupt = false
        var serverPort = mServerPort
        var bAuthorized = false
        var authorizedIp = ""
        var connectedIp = ""
        lateinit var wiFiIp : InetAddress
        override fun run() {
            val  wiFiStr = getByAddress(allocate(4).order(LITTLE_ENDIAN).putInt(mWiFiIpInt).array()).hostAddress
            wiFiIp = getByName(wiFiStr)

            while (!closeDownInterrupt) {
                try {
                    connectToClient()  // make sure we are connected to the internal BOINC client
                   // while (!mExternEnabled && !closeDownInterrupt) {
                   //     sendToApplication("IDLE")
                   //     Thread.sleep(2000)  // do nothing
                   // }

                    if (serverSocket != null) { // close the socket to free it for connecting
                        serverSocket!!.close()
                    }
                    serverSocket = ServerSocket(serverPort, 0, wiFiIp)  // backlog default
                    serverSocket!!.soTimeout = 10000 // 10 second timeout
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
                if (Thread.currentThread().isInterrupted)
                {
                    // InterruptedException doesn't catch in .accept but goes to InterruptedIOException
                    closeDownInterrupt = true
                    return // interrupted
                }
                try {
                    val socket = serverSocket!!.accept()
                    if (socket != null) {
                        handleConnection(socket) // we have a connection, handle the handshake
 //                       socket.close()
                    }
                } catch (e: InterruptedException) { // must be the first in catch
                    closeDownInterrupt = true
                    return // interrupted
                }
                catch (i: InterruptedIOException) {
                    // ends here on an external interrupt
                    sendToApplication("TIMEOUT")
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
                if (mIpAllowedList.isNotEmpty()) {  // check the ip allow list
                    var ip = connectedIp
                    var bFound = false
                    ip = ip.replace("/", "")
                    ip = ip.substringBefore(':')
                    for (item in mIpAllowedList) {
                        if (ip.contains(item)) {
                            bFound = true
                            break
                        }
                    }
                    if (!bFound) {  // if no match just close the socket and don't reply
                        closeSocket(socket, out)
                        sendToApplication("IPNOT") //IP not allowed
                        return
                    }
                }
                if (bAuthorized) {
                    sendToApplication("CONOK") // connected and authorized
                } else {
                    sendToApplication("CONNOT") // connected not authorized
                }
                istream = socket.getInputStream()
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                closeSocket(socket, out)
                return  // interrupted
            } catch (e: Exception) {
            }
            try {
                out = BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
            } catch (e: Exception) {
            }
            // process the connection
            while (status && !closeDownInterrupt) {
                // check if this is the same IP as the authorized one
                if (bAuthorized)
                {
                    if (!connectedIp.equals(authorizedIp))
                    {
                        bAuthorized = false // authorizing ip mismatch
                        sendToApplication("CONIPMATCH")
                        closeSocket(socket, out)
                        return
                    }
                }

                status = ProcessRequests(istream, out)
                if (Thread.currentThread().isInterrupted)
                {
                    // Catch doesn't always trigger InterruptedException
                    closeSocket(socket, out)
                    return
                }
            }
            closeSocket(socket, out)
        }

        fun closeSocket(socket: Socket, out: BufferedWriter?) : Boolean
        {
            try {
                out!!.close()
                socket.close()
            } catch (e: InterruptedException) { // must be the first in catch
                closeDownInterrupt = true
                return true
            } catch (e: Exception) {
            }
            return false
        }

        // Get data from the socket and process it
        fun ProcessRequests(istream: InputStream?, out: BufferedWriter?): Boolean {
            try {
                val eol = '\u0003'
                var reply = "<?>" // Don't tell who we are
//                var read = 0
                val buffer = ByteArray(1024) // if the buffer is too short, the while loop reads it in parts
                var dataReadTotal = ""
 //               while (istream!!.read(buffer).also {read = it} != -1) {
                while (istream!!.read(buffer) != -1) {  // leave at the end of the stream, this generally never happens
                    val readString = String(buffer)
                    dataReadTotal += readString
                    if (readString.contains("\u0003")) // leave at the end marked by 3
                    {
                        break
                    }
                }
                if (dataReadTotal.length > 0) { // process the data we just read
                    var bAuth1 = false
                    if (dataReadTotal.contains(mRpcExternString.mRpcRequestBegin)) {    // dirty but fast parser
                        if (dataReadTotal.contains("<auth1")) {
                            bAuthorized = false
                            reply = mAuthenticateMd5.auth1(mExternPasswrd)  // send the legacy authenticator
                            bAuth1 = true
                        }
                        if (dataReadTotal.contains("<auth2")) {
                            var auth = "<unauthorized/>"
                            val parser = RpcExternAuthParser()
                            parser.parse(dataReadTotal)
                            if (mAuthenticateMd5.auth2(parser.nonceHash)) { // check if the passwords match
                                auth = "<authorized/>"
                                bAuthorized = true
                                authorizedIp = connectedIp
                                sendToApplication("CONOK") // connected and authorized
                            } else {
                                bAuthorized = false
                                authorizedIp = ""
                                sendToApplication("CONNOT") // connected not authorized
                            }
                            reply = mRpcExternString.mRpcReplyBegin + "\n" + auth + mRpcExternString.mRpcReplyEnd + "\n\u0003"
                        } else {
                            if (bAuthorized) {
                                if (isConnected) {  // we are connected and the passwords match, send the request to the BOINC client
                                    reply = sendRequestAndReplyRaw(dataReadTotal)
                                    reply += eol    // the BOINC client reply
                                }
                                else{
                                    connectToClient()   // this should never happen but just in case
                                }
                            } else {
                                if (!bAuth1) {
                                    reply = mRpcExternString.mRpcReplyBegin + "\n" + "\"<unauthorized/>\"" + mRpcExternString.mRpcReplyEnd + "\n\u0003"
                                }
                            }
                        }
                    }
                }
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
    }

    // send broadcast to the SettingsFragment
    fun sendToApplication(dataString: String)
    {
        try {
            mMessage = dataString
            if (!mSendUpdates)
            {
                return
            }
            val intent = Intent()
            intent.action = "RPC_EXTERN_FROM_CONNECTION"
            intent.putExtra("data", dataString)
            intent.flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES
            // make sure this is only send locally but the LocalBroadcast seems to fail
            mMonitor.sendBroadcast(intent)
//        LocalBroadcastManager.getInstance(BOINCActivity.appContext!!).sendBroadcast(intent)
        } catch (e: Exception) {
            var ii = 1  // debug
            ii +=1
        }
    }

    // connect to the internal BOINC client
    fun connectToClient()
    {
        while (!isConnected)  // check if connected to client
        {
            Thread.sleep(2000) // wait for things to settle
            mRpcExtern.connectClient(mThis, mClientSocketAddress, mBoicToken)
        }
    }
}

