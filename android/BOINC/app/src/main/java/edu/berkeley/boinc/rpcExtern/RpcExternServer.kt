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

package edu.berkeley.boinc.rpcExternSettings

import android.content.Intent
import edu.berkeley.boinc.R
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpcExtern.RpcSettingsData
import java.io.*
import java.net.*
import java.nio.ByteBuffer
import java.nio.ByteOrder

class RpcExternServer {
    var mServerRunning = false
    var mKeepRunning = true // false breaks down the threads
    var mClosedDown = false // signal closing down after keepRunning = false
    var mNotAllowedIP = false
    lateinit var mMonitor: Monitor
    var mSendUpdates = false
    var mTimeout = 0

    var mServerPort = 31416 // BOINC default port
    var mExternEnabled = false
    var mExternEncryption = true
    var mExternPasswrd :String = ""
    var mIpAllowedList = ArrayList<String>()

    private lateinit var mRpcClient: RpcClient

    var mServerSocket: ServerSocket? = null
    var mSocket: Socket? = null
    var mThreadMakeCon: Thread? = null
    var mMessage: String = ""

    fun server(monitorIn : Monitor, rpc : RpcClient, data : RpcSettingsData) {
        if (mServerRunning)
        {
            return  // prevent starting more than once
        }
        mServerRunning = true
        mRpcClient = rpc
        try {
            mMonitor = monitorIn
            mServerPort = data.externPort.toInt()
            mExternEnabled = data.externEnabled
            mExternEncryption = data.externEncryption
            mExternPasswrd = data.externPasswrd
            mIpAllowedList = data.ipAllowedList
        } catch (e:Exception){
            mIpAllowedList.add("none")
        }
        sendToApplication("ST") // Start
        mThreadMakeCon = Thread(connectWithServerSocket())
        mThreadMakeCon!!.start()
    }

    fun update(data : RpcSettingsData)
    {
        mKeepRunning = false
        sendToApplication("RC") // Reconnect
        var timeOut = 10
        try {
            mSocket!!.close()
        } catch (e : Exception) {
        }
        while (!mClosedDown)
        {
            Thread.sleep(500)  // wait for things to shut down
            timeOut -=1
            if (timeOut < 0)
            {
                break
            }
        }

        mExternEnabled = data.externEnabled
        mExternEncryption = data.externEncryption
        mExternPasswrd = data.externPasswrd
        mServerPort = data.externPort.toInt()
        mIpAllowedList = data.ipAllowedList

        mKeepRunning = true
        mClosedDown = false
        sendToApplication("ST") // Start
        mThreadMakeCon = Thread(connectWithServerSocket())
        mThreadMakeCon!!.start()

        /*
        ipAllowedList = data.ipAllowedList
         */
    }

    fun command(data : String)
    {
        when (data)
        {
            "START_UPDATE" ->
            {
                mSendUpdates = true
                sendToApplication(mMessage)
            }
            "STOP_UPDATE" ->
            {
                mSendUpdates = false
                sendToApplication(mMessage)
            }
        }
    }

    private val inputs: DataInputStream? = null
    inner class connectWithServerSocket : Runnable {
        override fun run() {
            while (!mExternEnabled && mKeepRunning)
            {
                sendToApplication("ID")
                Thread.sleep(2000)  // do nothing
            }
            if (!mKeepRunning)
            {
                mClosedDown = true
                return
            }

            try {
                if (mSocket == null) {
                    mServerSocket = ServerSocket(mServerPort)
                }
                mServerSocket!!.soTimeout = 10000 // 10 second timeout
            } catch (iioe: InterruptedIOException) {
                // Timeout
            } catch (e: Exception) {
            }
            ConnectAndRead()
        }

        private fun ConnectAndRead() {
            while (mKeepRunning) {
                try {
                    Thread(ConnectionThread(mServerSocket!!.accept())).start()
                    mTimeout += 1
                    if (mTimeout > 10)
                    {
                        sendToApplication("TO") //timout
                    }
                } catch (i: InterruptedIOException) {
                } catch (e: Exception) {
                }
            }
        }
    }

    private inner class ConnectionThread(private val socket: Socket) : Runnable {
        var istream: InputStream? = null
        var out: BufferedWriter? = null
        override fun run() {
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
 //                       if (!notAllowedIP) {
                            mNotAllowedIP = true
                            sendToApplication("IP") //IP not allowed
  //                      }
                        return
                    }
                }
                //               }
//                if (notAllowedIP) {
                mNotAllowedIP = false
                sendToApplication("CN") // connected
//                }
                istream = socket.getInputStream()
            } catch (e: Exception) {
            }
            try {
                out = BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
            } catch (e: Exception) {
            }
            while (status && mKeepRunning) {
                status = ProcessRequests(istream, out)
            }
            try {
                out!!.close()
                socket.close()
                return
            } catch (e: Exception) {
            }
        }
    }

    fun ProcessRequests(istream: InputStream?, out: BufferedWriter?): Boolean {
        try {
            val eol = '\u0003'
//            commandList.clear()
            var reply = "<?>"   // Don't tell who we are
            var bAuthHandled = false
            var read = 0
            val buffer = ByteArray(1024) // is the buffer is too short, the while loop reads it in parts
            var dataReadTotal = ""
            while (istream!!.read(buffer).also {read = it } != -1) {
                val readString = String(buffer)
                dataReadTotal += readString
                if (readString.contains("\u0003")) // make sure to leave at the end marked by 3
                {
                    break
                }
            }
            if (dataReadTotal.length > 0) {
//                commandList.add(dataReadTotal)
                if (dataReadTotal.contains("<boinc_gui_rpc_request>")) {
                    if (dataReadTotal.contains("<auth1")) {
                        reply = "<boinc_gui_rpc_reply>\n<nonce>0</nonce>\n</boinc_gui_rpc_reply>\n" + eol // dummy
                        bAuthHandled = true
                    }
                    if (dataReadTotal.contains("<auth2")) {
                        reply = "<boinc_gui_rpc_reply>\n<authorized/></boinc_gui_rpc_reply>\n" + eol // dummy
                        bAuthHandled = true
                    }
                    if (bAuthHandled == false) {
                        if (mRpcClient.isConnected()) {
                            reply =  mRpcClient.sendRequestAndReplyRaw(dataReadTotal)
                            reply += eol
                        }
                        else
                        {
                            reply = "<boinc_gui_rpc_reply>\n<error>No BOINC Client present</error></boinc_gui_rpc_reply>\n" + eol
                        }
                    }
                }
            }
//            blockWriter(out,reply)
            out!!.newLine()
            out.write(reply)
            out.flush()
        } catch (e: Exception) {
            return false
        }
        return true
    }
/*
    // testing only
    private fun blockWriter(out : BufferedWriter?, reply: String)
    {
        var start = 0
        var sub : String
        var lenReply = reply.length
        try {
            while (start < lenReply) {
                var len = start + 8192
                if (len > lenReply)
                {
                    len = lenReply
                }
                sub = reply.substring(start, len)
                out!!.write(sub)
                out.flush()
                start += 8192
            }
        } catch (e: IndexOutOfBoundsException )
        {
            if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer Index out of bound")
        } catch (e: IOException) {
            if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer IO exception")
        }
    }
 */

    fun sendToApplication(dataString : String)
    {
        try {
            mMessage = dataString
            if (!mSendUpdates)
            {
                return
            }
            val intent = Intent()
            intent.action = "RPC_EXTERN"
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

