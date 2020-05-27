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
package edu.berkeley.boinc

import android.content.SharedPreferences
import android.os.Bundle
import androidx.preference.*
import edu.berkeley.boinc.utils.getDefaultTheme
import edu.berkeley.boinc.utils.setAppTheme

class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {
    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        val context = preferenceManager.context
        val screen = preferenceManager.createPreferenceScreen(context)

        val autostart = CheckBoxPreference(context)
        autostart.title = context.getString(R.string.prefs_autostart_header)
        autostart.key = "autostart"
        autostart.isIconSpaceReserved = false
        autostart.setDefaultValue(context.resources.getBoolean(R.bool.prefs_default_autostart))

        val showNotification = CheckBoxPreference(context)
        showNotification.title = context.getString(R.string.prefs_show_notification_notices_header)
        showNotification.key = "showNotification"
        showNotification.isIconSpaceReserved = false
        showNotification.setDefaultValue(context.resources.getBoolean(R.bool.prefs_default_notification_notices))

        val showAdvanced = CheckBoxPreference(context)
        showAdvanced.title = context.getString(R.string.prefs_show_advanced_header)
        showAdvanced.key = "showAdvanced"
        showAdvanced.isIconSpaceReserved = false
        showAdvanced.setDefaultValue(context.resources.getBoolean(R.bool.prefs_default_advanced))

        val general = PreferenceCategory(context)
        general.title = context.getString(R.string.prefs_category_general)
        general.isIconSpaceReserved = false
        screen.addPreference(general)
        general.addPreference(autostart)
        general.addPreference(showNotification)
        general.addPreference(showAdvanced)

        if (!BOINCActivity.monitor!!.stationaryDeviceMode) {
            val suspendWhenScreenOn = CheckBoxPreference(context)
            suspendWhenScreenOn.title = context.getString(R.string.prefs_suspend_when_screen_on)
            suspendWhenScreenOn.key = "suspendWhenScreenOn"
            suspendWhenScreenOn.isIconSpaceReserved = false
            suspendWhenScreenOn.setDefaultValue(context.resources.getBoolean(R.bool.prefs_suspend_when_screen_on))

            general.addPreference(suspendWhenScreenOn)
        }

        val deviceName = EditTextPreference(context)
        deviceName.title = context.getString(R.string.prefs_general_device_name_header)
        deviceName.isIconSpaceReserved = false
        deviceName.key = "deviceName"
        deviceName.text = BOINCActivity.monitor!!.hostInfo.domainName ?: ""
        general.addPreference(deviceName)

        val theme = ListPreference(context)
        theme.title = getString(R.string.prefs_theme_header)
        theme.isIconSpaceReserved = false
        theme.key = "theme"
        theme.setEntries(R.array.theme_entries)
        theme.setEntryValues(R.array.theme_values)
        theme.setDefaultValue(getDefaultTheme())
        general.addPreference(theme)

        val network = PreferenceCategory(context)
        network.title = context.getString(R.string.prefs_category_network)
        network.isIconSpaceReserved = false
        screen.addPreference(network)

        val power = PreferenceCategory(context)
        power.title = context.getString(R.string.prefs_category_power)
        power.isIconSpaceReserved = false
        screen.addPreference(power)

        val cpu = PreferenceCategory(context)
        cpu.title = context.getString(R.string.prefs_category_cpu)
        cpu.isIconSpaceReserved = false
        screen.addPreference(cpu)

        val storage = PreferenceCategory(context)
        storage.title = context.getString(R.string.prefs_category_storage)
        storage.isIconSpaceReserved = false
        screen.addPreference(storage)

        val memory = PreferenceCategory(context)
        memory.title = context.getString(R.string.prefs_category_memory)
        memory.isIconSpaceReserved = false
        screen.addPreference(memory)

        val other = PreferenceCategory(context)
        other.title = context.getString(R.string.prefs_category_other)
        other.isIconSpaceReserved = false
        screen.addPreference(other)

        val debug = PreferenceCategory(context)
        debug.title = context.getString(R.string.prefs_category_debug)
        debug.isIconSpaceReserved = false
        screen.addPreference(debug)

        preferenceScreen = screen
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
        when (key) {
            // General
            "autostart" -> {
                BOINCActivity.monitor!!.autostart = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_default_autostart))
            }
            "showNotification" -> {
                BOINCActivity.monitor!!.showNotificationForNotices =
                        sharedPreferences.getBoolean(key, resources.getBoolean(R.bool.prefs_default_notification_notices))
            }
            "showAdvanced" -> {
                BOINCActivity.monitor!!.showAdvanced = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_default_advanced))
            }
            "suspendWhenScreenOn" -> {
                BOINCActivity.monitor!!.suspendWhenScreenOn = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_suspend_when_screen_on))
            }
            "deviceName" -> BOINCActivity.monitor!!.setDomainName(sharedPreferences.getString(key, ""))
            "theme" -> setAppTheme(sharedPreferences.getString(key, getDefaultTheme())!!)

            // Network

            // Power
            "powerSourceAc" -> {
                BOINCActivity.monitor!!.powerSourceAc = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_power_source_ac))
            }
            "powerSourceUsb" -> {
                BOINCActivity.monitor!!.powerSourceUsb = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_power_source_usb))
            }
            "powerSourceWireless" -> {
                BOINCActivity.monitor!!.powerSourceWireless = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_power_source_wireless))
            }
            "stationaryDeviceMode" -> {
                BOINCActivity.monitor!!.stationaryDeviceMode = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_stationary_device_mode))
            }

            // CPU

            // Storage

            // Memory

            // Other

            // Debug
            "logLevel" -> {
                BOINCActivity.monitor!!.logLevel = sharedPreferences.getInt(key,
                        resources.getInteger(R.integer.prefs_default_loglevel))
            }
        }
    }
}
