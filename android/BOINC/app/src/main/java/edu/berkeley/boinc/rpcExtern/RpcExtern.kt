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
import android.util.Log
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpcExternSettings.RpcExternServer
import edu.berkeley.boinc.utils.Logging
import org.apache.commons.lang3.StringUtils
import java.io.*

class RpcExtern {
    var ThreadConnect: Thread? = null
    var rpcClient = RpcClient()
    var rpcExternServer = RpcExternServer()
    var connected = false
    lateinit var monitor : Monitor
    lateinit var rpcSettingsData : RpcSettingsData
    private lateinit var clientSocketAddress : String
    private lateinit var boicToken : String


    // The BOINC client started, so we should be able to connect
    fun start(monitorIn : Monitor, socketAddress: String, token: String, data : RpcSettingsData) : Boolean {
        clientSocketAddress = socketAddress
        boicToken = token
        rpcSettingsData = data
        monitor = monitorIn
        ThreadConnect = Thread(connect())
        ThreadConnect!!.start()
        return true
    }

    fun update(data : RpcSettingsData)
    {
        rpcExternServer.update(data)
    }

    fun command(data : String)
    {
        rpcExternServer.command(data)
    }

    inner class connect : Runnable {
        override fun run() {
            val keeptrying = true
            while (keeptrying)
            {
                Log.d(Logging.TAG, "RpcExtern Connect to Client")
                connected = connectClient()
                if (connected) {
                    Log.d(Logging.TAG, "RpcExtern Connected to Client")
                    rpcExternServer.server(monitor, rpcClient, rpcSettingsData)
                    return
                }
                Thread.sleep(5000)  // try again after xx seconds
            }
        }
    }

    /**
     * Establishes connection to client and handles initial authentication
     *
     * @return Boolean success
     */
    private fun connectClient(): Boolean {
        var success = rpcClient.open(clientSocketAddress)
        if (!success) {
            if (Logging.ERROR) Log.e(Logging.TAG, "Connection failed!")
            return false
        }

        //authorize
        success = rpcClient.authorize(boicToken)
        if (!success && Logging.ERROR) {
            Log.e(Logging.TAG, "Authorization failed!")
        }
        return success
    }

    /**
     * Reads authentication token for GUI RPC authentication from file
     *
     * @param authFilePath absolute path to file containing GUI RPC authentication
     * @return GUI RPC authentication code
     */
    fun readAuthToken(authFilePath: String): String {   //TODO eFMer duplicate ClientImplementation.java
        var authKey = ""
        try {
            BufferedReader(FileReader(File(authFilePath))).use { br -> authKey = br.readLine() }
        } catch (fnfe: FileNotFoundException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "Auth file not found: ", fnfe)
            }
        } catch (ioe: IOException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "IOException: ", ioe)
            }
        }
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "Authentication key acquired. length: " + StringUtils.length(authKey))
        }
        return authKey
    }
}


