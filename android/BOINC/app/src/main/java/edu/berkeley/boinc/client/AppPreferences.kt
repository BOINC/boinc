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
package edu.berkeley.boinc.client

import android.content.Context
import android.content.SharedPreferences
import android.util.Log
import androidx.core.content.edit
import edu.berkeley.boinc.R
import edu.berkeley.boinc.utils.Logging

class AppPreferences {
    private var prefs: SharedPreferences? = null

    var autostart = false
        set(value) {
            putBooleanToPrefs("autostart", value)
            field = value
        }
    var showNotificationForNotices = false
        set(value) {
            putBooleanToPrefs("showNotification", value)
            field = value
        }
    var showAdvanced = false
        set(value) {
            putBooleanToPrefs("showAdvanced", value)
            field = value
        }
    var logLevel = 0
        set(value) {
            // Commit a new value synchronously
            prefs?.edit(commit = true) { putInt("logLevel", value) }
            field = value
            Logging.setLogLevel(value)
        }
    var powerSourceAc = false
        set(value) {
            putBooleanToPrefs("powerSourceAc", value)
            field = value
        }
    var powerSourceUsb = false
        set(value) {
            putBooleanToPrefs("powerSourceUsb", value)
            field = value
        }
    var powerSourceWireless = false
        set(value) {
            putBooleanToPrefs("powerSourceWireless", value)
            field = value
        }
    var stationaryDeviceMode = false
        set(value) {
            putBooleanToPrefs("stationaryDeviceMode", value)
            field = value
        }
    var suspendWhenScreenOn = false
        set(value) {
            putBooleanToPrefs("suspendWhenScreenOn", value)
            field = value
        }

    private fun putBooleanToPrefs(key: String, value: Boolean) {
        prefs?.edit(commit = true) { putBoolean(key, value) }
    }

    fun readPrefs(ctx: Context) {
        if (prefs == null) {
            prefs = ctx.getSharedPreferences("PREFS", 0)
        }
        //second parameter of reading function is the initial value after installation.
        autostart = prefs!!.getBoolean("autostart",
                ctx.resources.getBoolean(R.bool.prefs_default_autostart))
        showNotificationForNotices = prefs!!.getBoolean("showNotification",
                ctx.resources.getBoolean(R.bool.prefs_default_notification_notices))
        showAdvanced = prefs!!.getBoolean("showAdvanced",
                ctx.resources.getBoolean(R.bool.prefs_default_advanced))
        logLevel = prefs!!.getInt("logLevel",
                ctx.resources.getInteger(R.integer.prefs_default_loglevel))
        Logging.setLogLevel(logLevel)
        powerSourceAc = prefs!!.getBoolean("powerSourceAc",
                ctx.resources.getBoolean(R.bool.prefs_power_source_ac))
        powerSourceUsb = prefs!!.getBoolean("powerSourceUsb",
                ctx.resources.getBoolean(R.bool.prefs_power_source_usb))
        powerSourceWireless = prefs!!.getBoolean("powerSourceWireless",
                ctx.resources.getBoolean(R.bool.prefs_power_source_wireless))
        stationaryDeviceMode = prefs!!.getBoolean("stationaryDeviceMode",
                ctx.resources.getBoolean(R.bool.prefs_stationary_device_mode))
        suspendWhenScreenOn = prefs!!.getBoolean("suspendWhenScreenOn",
                ctx.resources.getBoolean(R.bool.prefs_suspend_when_screen_on))
        if (Logging.DEBUG) {
            Log.d(Logging.TAG,
                    "appPrefs read successful." + autostart + showNotificationForNotices +
                            showAdvanced + logLevel + powerSourceAc + powerSourceUsb + powerSourceWireless)
        }
    }
}
