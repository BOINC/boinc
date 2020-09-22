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

import android.content.Context
import android.net.wifi.WifiManager
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpcExtern.RpcExtern
import edu.berkeley.boinc.utils.Logging
import java.io.*
import java.net.*
import java.nio.ByteBuffer
import java.nio.ByteOrder


class RpcExternSettings {
    val SERVER_PORT = 31418 // BOINC default port

    lateinit var boincWorkingDir: String
    lateinit var fileNameGuiAuthentication: String
    lateinit var clientSocketAddress: String
    var context: Context? = null
    var commandList: MutableList<String> = ArrayList()
    var ipAllowedList = listOf<String>()

    private lateinit var rpcClient: RpcClient

    var rpcExtern = RpcExtern()

    fun test(contextIn: Context): Boolean {
        context = contextIn
        boincWorkingDir = context!!.getString(R.string.client_path)
        fileNameGuiAuthentication = context!!.getString(R.string.auth_file_name)
        clientSocketAddress = context!!.getString(R.string.client_socket_address)
        var connect: Boolean = false
        try {
            val token = rpcExtern.readAuthToken(boincWorkingDir + fileNameGuiAuthentication)
            connect = rpcExtern.start(clientSocketAddress, token)
        } catch (e: IOException) {
            if (Logging.DEBUG) Log.e(Logging.TAG, "Start RpcExtern something went wrong")
            return false
        }
        if (!connect) {
            return false
        }
        rpcClient = rpcExtern.GetRpcClient()
        val reply: String = rpcClient.sendRequestAndReply("<get_cc_status/>\n")
        Log.d(Logging.TAG, "RpcExtern Connected to Client 2" + reply)

        server(context!!)

        return true
    }

    var serverSocket: ServerSocket? = null
    var socket: Socket? = null
    var ThreadMakeCon: Thread? = null
    var message: String? = null
    var serverIp = ""

    fun server(contextThis: Context) {
        // test
        ipAllowedList += "192.168.10.20" // allowed
        // test
        context = contextThis
        try {
            serverIp = localIpAddress()
        } catch (e: UnknownHostException) {
            e.printStackTrace()
        }
        ThreadMakeCon = Thread(connectWithServerSocket())
        ThreadMakeCon!!.start()
    }

    fun localIpAddress(): String {
        val wifiManager: WifiManager = context!!.applicationContext.getSystemService(AppCompatActivity.WIFI_SERVICE) as WifiManager
        val wifiInfo = wifiManager.connectionInfo
        val ipInt = wifiInfo.ipAddress
        return InetAddress.getByAddress(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(ipInt).array()).hostAddress
    }

    private val inputs: DataInputStream? = null

    inner class connectWithServerSocket : Runnable {
        override fun run() {
            try {
                if (socket == null) {
                    serverSocket = ServerSocket(SERVER_PORT)
                }
                serverSocket!!.soTimeout = 10000 // 10 second timeout
                ConnectAndRead()
            } catch (iioe: InterruptedIOException) {
                // Timeout
                ConnectAndRead()
            } catch (e: SocketException) {
                // Port already in use
                e.printStackTrace()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }

        private fun ConnectAndRead() {
            try {
                while (true) {
                    Thread(ConnectionThread(serverSocket!!.accept())).start()
                }
            } catch (iioe: InterruptedIOException) {
                ConnectAndRead()
            } catch (e: IOException) {
                ConnectAndRead()
            }
        }
    }

    private inner class ConnectionThread(private val socket: Socket) : Runnable {
        var istream: InputStream? = null
        var out: BufferedWriter? = null
        override fun run() {
            var status = true
            Thread.sleep(500)  // don't allow flooding
            try {
                if (ipAllowedList.isNotEmpty()) {
                    var bFound = false
                    var ip = socket.remoteSocketAddress.toString()
                    ip = ip.replace("/", "")
                    ip = ip.substringBefore(':')
                    for (item in ipAllowedList) {
                        if (ip.contains(item)) {
                            bFound = true
                            break
                        }
                    }
                    if (!bFound) {
                        // make a hole
                        out!!.close()
                        socket.close()
                        return
                    }
                }
                //               }

                istream = socket.getInputStream()
            } catch (e: IOException) {
                e.printStackTrace()
            }
            try {
                out = BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
            } catch (e: IOException) {
                e.printStackTrace()
            }
            while (status) {
                status = ProcessRequests(socket, istream, out)
            }
            try {
                out!!.close()
                socket.close()
                return
            } catch (e: IOException) {
                e.printStackTrace()
            }
            out!!.close()
        }
    }

    fun ProcessRequests(socket: Socket?, istream: InputStream?, out: BufferedWriter?): Boolean {
        try {
            val eol = '\u0003'
            commandList.clear()
            var reply = "<?>"   // Don't tell who we are
            var bAuthHandled = false
            var read = 0
            val buffer = ByteArray(1024) // is the buffer is too short, the while loop reads it in parts
            var dataReadTotal = ""
            while (istream!!.read(buffer).also { read = it } != -1) {
                val readString = String(buffer)
                dataReadTotal += readString
                if (readString.contains("\u0003")) // make sure to leave at the end marked by 3
                {
                    break
                }
            }
            if (dataReadTotal.length > 0) {
                commandList.add(dataReadTotal)
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
                        if (rpcClient.isConnected()) {
                            reply =  rpcClient.sendRequestAndReplyRaw(dataReadTotal)
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
            out!!.write(reply)
            out!!.flush()
        } catch (e: IOException) {
            e.printStackTrace()
            return false
        }
        return true
    }

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

/*
    fun localIpAddress(): String
    {
        try {
            val wifiManager: WifiManager
            wifiManager = context!!.getSystemService(Context.WIFI_SERVICE) as WifiManager
            val dhcpinfo = wifiManager.dhcpInfo
            val wifiInfo = wifiManager.connectionInfo
            val ipInt = wifiInfo.ipAddress
            return InetAddress.getByAddress(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(ipInt).array()).hostAddress
        }
        catch (e: UnknownHostException)
        {
            return "";
        }
    }

    private var output: BufferedOutputStream? = null
    private var input: BufferedInputStream? = null
    var msg1: String? = null
    var msg2: String? = null
    var bytesRead = 0
    var connected = false

    inner class connectWithServer : Runnable {
        override fun run() {
            var msg: String
            val charsetName = "ASCII"
            val testOut = "Boinc test Message"
            try {
                if (serverSocket == null) {
                    serverSocket = ServerSocket(serverPort)
                }
                serverSocket!!.setSoTimeout(10000) // 10 sec only for the read function
                val byteArrray = testOut.toByteArray(charset(charsetName))
                socket = serverSocket!!.accept()
                output = BufferedOutputStream(socket!!.getOutputStream())
                input = BufferedInputStream(socket!!.getInputStream()) // read bytes
                bytesRead = 0
                connected = true

                //               output.write(byteArrray);
                //               output.flush();
                while (bytesRead <= 0 && connected) {
                    bytesRead = input!!.read()
                    //                  output.write(byteArrray);
                    //                  output.flush();
                    // connected = socket.isConnected() && !socket.isClosed(); doesn't work
                }

                //    output.write(testOut);
                //    output.flush();
                val i = 1
            } catch (e: IOException) {
                if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer IOException")
            } catch (e: SecurityException) {
                if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer SecurityException")
            } catch (e: IllegalArgumentException) {
                if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer IllegalArgumentException")
            } catch (e: SocketTimeoutException) {
                if (Logging.DEBUG) Log.e(Logging.TAG, "RpcExternServer Timeout")
            }
            startServerThread();
        }
    }
 */
}

