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

import android.util.Log
import edu.berkeley.boinc.rpcExtern.RpcExternServer
import edu.berkeley.boinc.utils.Logging
import org.apache.commons.lang3.StringUtils
import java.io.*

class RpcExtern {

    /**
     * Establishes connection to client and handles initial authentication
     *
     * @return Boolean success
     */
    fun connectClient(rpcClient: RpcExternServer, clientSocketAddress: String, boincToken: String): Boolean {
        var success = rpcClient.open(clientSocketAddress)
        if (!success) {
            if (Logging.ERROR) Log.e(Logging.TAG, "Connection failed!")
            return false
        }

        //authorize
        success = rpcClient.authorize(boincToken)
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
            File(authFilePath).bufferedReader().use { br -> authKey = br.readLine() }
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
            Log.d(Logging.TAG, "Authentication key acquired. length: " + (authKey?.length ?: 0))
        }
        return authKey
    }
}
