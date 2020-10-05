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
import java.io.*
import java.net.ServerSocket
import java.net.Socket

class RpcExternServer : RpcClient() {
    val mAuthenticateMd5 = RpcExternAuthorizeMd5()
    val mRpcExternString = RpcExternString()
    val mRpcExtern = RpcExtern()
    val mThis = this

    var mClientSocketAddress = ""
    var mBoicToken = ""

    var mbAuthorized = false
    var mServerRunning = false
    var mKeepRunning = true // false breaks down the threads
    var mClosedDown = false // signal closing down after keepRunning = false
    var mNotAllowedIP = false
    lateinit var mMonitor: Monitor
    var mSendUpdates = false
    var mTimeout = 0

    val DEFAULT_PORT = 31416
    var mServerPort = DEFAULT_PORT
    var mExternEnabled = false
    var mExternEncryption = true
    var mExternPasswrd :String = ""
    var mIpAllowedList = ArrayList<String>()
//    var mServerSocket: ServerSocket? = null
    var mThreadMakeCon: Thread? = null
    var mUpdate = false
    var mMessage: String = ""

    fun start(monitorIn: Monitor, socketAddress: String, boincToken: String, data: RpcSettingsData)
    {
        if (mServerRunning)
        {
            return  // prevent starting more than once
        }
        mClientSocketAddress = socketAddress
        mBoicToken = boincToken
        mbAuthorized = false
        mServerRunning = true
        try {
            mServerPort = data.externPort.toInt()
        } catch (e : Exception)
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

        if (mExternEnabled) {
            sendToApplication("START") // Start
            mThreadMakeCon = Thread(mainThreadLoop())
            mThreadMakeCon!!.start()
        }
        else
        {
            sendToApplication("IDLE")
        }
    }

    fun update(data: RpcSettingsData)
    {
        mbAuthorized = false
        mKeepRunning = false
        sendToApplication("CLOSING")
//        mClosedDown = false

        if (mUpdate)
        {
            return
        }
        mUpdate = true

        // wait for the thread to signal it has shut down
        if (mThreadMakeCon != null) {
            while (mThreadMakeCon!!.isAlive) {
                if (mThreadMakeCon == null) {
                    break
                }
                mThreadMakeCon!!.interrupt()    // signal thread to shutdown
                Thread.sleep(500)  // wait for things to shut down
            }
        }
        // Thread stopped

        mExternEnabled = data.externEnabled
        mExternEncryption = data.externEncryption
        mExternPasswrd = data.externPasswrd
        try {
            mServerPort = data.externPort.toInt()
        } catch (e : Exception)
        {
            mServerPort = DEFAULT_PORT
        }
        mIpAllowedList = data.ipAllowedList

        mKeepRunning = true
        mClosedDown = false
        if (mExternEnabled) {
            sendToApplication("START") // Start
            mThreadMakeCon = Thread(mainThreadLoop())
            mThreadMakeCon!!.start()
        }
        else
        {
            sendToApplication("IDLE")
        }
        mUpdate = false
    }

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

    private val inputs: DataInputStream? = null
    inner class mainThreadLoop : Runnable {
        var serverSocket: ServerSocket? = null
        var closeDownInterrupt = false
        override fun run() {
            try {
                while (!closeDownInterrupt) {
                    while (!mKeepRunning && !closeDownInterrupt) {
                        // parking loop to wait for closedown
                        Thread.sleep(2000)  // do nothing
                    }

                    while (!isConnected())  // check if connected to client
                    {
                        Thread.sleep(2000) // wait for client to settle
                        mRpcExtern.connectClient(mThis, mClientSocketAddress, mBoicToken)
                    }

                    while (!mExternEnabled && mKeepRunning && !closeDownInterrupt) {
                        sendToApplication("IDLE")
                        Thread.sleep(2000)  // do nothing
                    }

                    try {
                        if (serverSocket == null) {
                            serverSocket = ServerSocket(mServerPort)
                        }
                        serverSocket!!.soTimeout = 10000 // 10 second timeout
                    } catch (iioe: InterruptedIOException) {
                        var ii = 1 // debug break point
                        ii += 1
                    } catch (e: InterruptedException) {
                        mClosedDown = true
                        return  // interrupted
                    } catch (e: Exception) {
                        var ii = 1 // debug break point
                        ii += 1
                    }
                    ConnectAndRead()
                }
            } catch (e: InterruptedException) {
                serverSocket!!.close()
                return // interrupted
            } catch (e: Exception) {
                var ii = 1 // debug break point
                ii += 1
            }
            serverSocket!!.close()  // interrupted
        }

        private fun ConnectAndRead() {
            while (mKeepRunning && !closeDownInterrupt) {
                try {
                    val socket = serverSocket!!.accept()
                    if (socket != null) {
                        handleConnection(socket)
 //                       socket.close()
                    }
                } catch (i: InterruptedIOException) {
                    mTimeout += 1
                    if (mTimeout > 2) {
                        sendToApplication("TIMEOUT") //timout
                    }
                } catch (e: InterruptedException) {
                    closeDownInterrupt = true
                    return // interrupted
                } catch (e: Exception) {
                }
            }
        }

        var istream: InputStream? = null
        var out: BufferedWriter? = null
        fun handleConnection(socket: Socket) {
            var status = true
            Thread.sleep(500)  // don't allow flooding
            mTimeout = 0
            try {
                if (mIpAllowedList.isNotEmpty()) {
                    var bFound = false
                    var ip = socket.remoteSocketAddress.toString()
                    ip = ip.replace("/", "")
                    ip = ip.substringBefore(':')
                    for (item in mIpAllowedList) {
                        if (ip.contains(item)) {
                            bFound = true
                            break
                        }
                    }
                    if (!bFound) {
                        // make a hole
                        socket.close()
                        mNotAllowedIP = true
                        sendToApplication("IPNOT") //IP not allowed
                        return
                    }
                }
                if (mbAuthorized) {
                    sendToApplication("CONOK") // connected and authorized
                } else {
                    sendToApplication("CONNOT") // connected not authorized
                }

                mNotAllowedIP = false
                istream = socket.getInputStream()
            } catch (e: Exception) {
            }
            try {
                out = BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
            } catch (e: Exception) {
            }
            while (status && mKeepRunning && !closeDownInterrupt) {
                status = ProcessRequests(istream, out)
            }
            try {
                out!!.close()
                socket.close()
                return
            } catch (e: InterruptedException) {
                closeDownInterrupt = true
                return  // interrupted
            } catch  (e: Exception) {
            }
        }

        fun ProcessRequests(istream: InputStream?, out: BufferedWriter?): Boolean {
            try {
                val eol = '\u0003'
                var reply = "<?>"   // Don't tell who we are
//                var read = 0
                val buffer = ByteArray(1024) // if the buffer is too short, the while loop reads it in parts
                var dataReadTotal = ""
 //               while (istream!!.read(buffer).also {read = it} != -1) {
                while (istream!!.read(buffer) != -1) {
                    val readString = String(buffer)
                    dataReadTotal += readString
                    if (readString.contains("\u0003")) // make sure to leave at the end marked by 3
                    {
                        break
                    }
                }
                if (dataReadTotal.length > 0) {
                    var bAuth1 = false
                    if (dataReadTotal.contains(mRpcExternString.mRpcRequestBegin)) {
                        if (dataReadTotal.contains("<auth1")) {
                            mbAuthorized = false
                            reply = mAuthenticateMd5.auth1(mExternPasswrd)
                            bAuth1 = true
                        }
                        if (dataReadTotal.contains("<auth2")) {
                            var auth = "<unauthorized/>"
                            var parser = RpcExternAuthParser()
                            parser.parse(dataReadTotal)
                            if (mAuthenticateMd5.auth2(parser.nonceHash)) {
                                auth = "<authorized/>"
                                mbAuthorized = true
                                sendToApplication("CONOK") // connected and authorized
                            } else {
                                mbAuthorized = false
                                sendToApplication("CONNOT") // connected not authorized
                            }
                            reply = mRpcExternString.mRpcReplyBegin + "\n" + auth + mRpcExternString.mRpcReplyEnd + "\n\u0003"
                        } else {
                            if (mbAuthorized) {
                                if (isConnected()) {
                                    reply = sendRequestAndReplyRaw(dataReadTotal)
                                    reply += eol
                                }
                            } else {
                                if (!bAuth1) {
                                    reply = mRpcExternString.mRpcReplyBegin + "\n" + "\"<unauthorized/>\"" + mRpcExternString.mRpcReplyEnd + "\n\u0003"
                                }
                            }
                        }
                    }
                }
                out!!.newLine()
                out.write(reply)
                out.flush()
            } catch (e: InterruptedException) {
                closeDownInterrupt = true
                return false // interrupted
            } catch (e: Exception) {
                return false
            }
            return true
        }
    }

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
            var ii = 1
            ii +=1
        }
    }
}

