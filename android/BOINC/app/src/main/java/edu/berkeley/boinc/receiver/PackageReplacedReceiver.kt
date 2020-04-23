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
package edu.berkeley.boinc.receiver

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.utils.Logging

class PackageReplacedReceiver : BroadcastReceiver() {
    /*
     * Receiver for android.intent.action.PACKAGE_REPLACED
     * This intent is protected and can only be triggered by the system
     * To test this code run ADB with the following command:
     * adb install -r yourapp.apk
     */
    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action == Intent.ACTION_PACKAGE_REPLACED) {
            if (intent.dataString.toString().contains("edu.berkeley.boinc")) {
                if (Logging.ERROR) {
                    Log.d(Logging.TAG, "PackageReplacedReceiver: starting service...")
                }
                context.startService(Intent(context, Monitor::class.java))
            } else if (Logging.DEBUG) {
                Log.d(Logging.TAG, "PackageReplacedReceiver: other package: " + intent.dataString)
            }
        }
    }
}
