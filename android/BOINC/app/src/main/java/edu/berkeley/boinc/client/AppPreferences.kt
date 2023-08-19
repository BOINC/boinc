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
import androidx.preference.PreferenceManager
import edu.berkeley.boinc.R
import edu.berkeley.boinc.utils.Logging
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class AppPreferences @Inject constructor(val context: Context) {
    private val prefs = PreferenceManager.getDefaultSharedPreferences(context)

    var autostart = prefs.getBoolean("autostart", context.resources.getBoolean(R.bool.prefs_default_autostart))
    var showNotificationForNotices = prefs.getBoolean("showNotifications",
            context.resources.getBoolean(R.bool.prefs_default_notification_notices))
    var showNotificationDuringSuspend = prefs.getBoolean("showNotificationsDuringSuspend",
            context.resources.getBoolean(R.bool.prefs_default_notification_suspended))
    var showAdvanced = prefs.getBoolean("showAdvanced", context.resources.getBoolean(R.bool.prefs_default_advanced))
    var isRemote     = prefs.getBoolean("remoteEnable", context.resources.getBoolean(R.bool.prefs_default_remote))
    var logLevel = prefs.getInt("logLevel", context.resources.getInteger(R.integer.prefs_default_loglevel))
        set(value) {
            field = value
            Logging.setLogLevel(value)
        }
    var logCategories = prefs.getStringSet("guiLogCategories", context.resources.getStringArray(R.array.prefs_gui_log_categories).toSet())!!.toList()
        set(value) {
            field = value
            Logging.setLogCategories(value)
        }
    var powerSourceAc = prefs.getBoolean("powerSourceAc", context.resources.getBoolean(R.bool.prefs_power_source_ac))
    var powerSourceUsb = prefs.getBoolean("powerSourceUsb", context.resources.getBoolean(R.bool.prefs_power_source_usb))
    var powerSourceWireless = prefs.getBoolean("powerSourceWireless",
            context.resources.getBoolean(R.bool.prefs_power_source_wireless))
    var stationaryDeviceMode = prefs.getBoolean("stationaryDeviceMode",
            context.resources.getBoolean(R.bool.prefs_stationary_device_mode))
    var suspendWhenScreenOn = prefs.getBoolean("suspendWhenScreenOn",
            context.resources.getBoolean(R.bool.prefs_suspend_when_screen_on))

    init {
        Logging.setLogLevel(logLevel)

        Logging.logDebug(Logging.Category.SETTINGS,
            "appPrefs read successful: autostart: [$autostart] showNotificationForNotices: [$showNotificationForNotices] " +
            "showNotificationDuringSuspend: [$showNotificationDuringSuspend] " +
            "showAdvanced: [$showAdvanced] logLevel: [$logLevel] powerSourceAc: [$powerSourceAc] powerSourceUsb: [$powerSourceUsb] " +
            "powerSourceWireless: [$powerSourceWireless] stationaryDeviceMode: [$stationaryDeviceMode] suspendWhenScreenOn: [$suspendWhenScreenOn]")
    }
}
