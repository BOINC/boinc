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
import android.os.Build
import android.os.Bundle
import android.os.RemoteException
import android.util.Log
import androidx.core.content.edit
import androidx.lifecycle.lifecycleScope
import androidx.preference.*
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.HostInfo
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.setAppTheme
import kotlinx.coroutines.async
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch

class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {
    override fun onResume() {
        super.onResume()
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        val prefs = BOINCActivity.monitor!!.prefs
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)

        findPreference<EditTextPreference>("deviceName")?.summary =
                sharedPreferences.getString("deviceName", BOINCActivity.monitor!!.hostInfo.domainName)

        val theme = sharedPreferences.getString("theme", "default")!!
        findPreference<ListPreference>("theme")?.summary = getThemeString(theme)

        val dailyTransferLimit = sharedPreferences.getString("dailyTransferLimit",
                prefs.dailyTransferLimitMB.toString())
        findPreference<EditTextPreference>("dailyTransferLimit")?.summary = dailyTransferLimit

        findPreference<EditTextPreference>("diskMinFreeMB")?.summary =
                sharedPreferences.getString("diskMinFreeMB", "107")
        findPreference<EditTextPreference>("diskInterval")?.summary =
                sharedPreferences.getString("diskInterval", "60")

        findPreference<EditTextPreference>("workBufMinDays")?.summary =
                sharedPreferences.getString("workBufMinDays", "0.1")
        findPreference<EditTextPreference>("workBufAdditionalDays")?.summary =
                sharedPreferences.getString("workBufAdditionalDays", "0.5")
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences.unregisterOnSharedPreferenceChangeListener(this)
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        val hostInfo = BOINCActivity.monitor!!.hostInfo // Get the hostinfo from client via RPC
        val prefs = BOINCActivity.monitor!!.prefs

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)
        if ("usedCpuCores" !in sharedPreferences) {
            sharedPreferences.edit { putInt("usedCpuCores", pctCpuCoresToNumber(hostInfo, prefs.maxNoOfCPUsPct)) }
        }

        val stationaryDeviceMode = BOINCActivity.monitor!!.stationaryDeviceMode
        val stationaryDeviceSuspected = BOINCActivity.monitor!!.isStationaryDeviceSuspected

        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        setPreferencesFromResource(R.xml.root_preferences, rootKey)

        if ("deviceName" !in sharedPreferences) {
            findPreference<EditTextPreference>("deviceName")?.text =
                    BOINCActivity.monitor!!.hostInfo.domainName
        }

        if (!stationaryDeviceSuspected) {
            findPreference<CheckBoxPreference>("stationaryDeviceMode")?.isVisible = false
        }
        if (stationaryDeviceMode) {
            findPreference<CheckBoxPreference>("suspendWhenScreenOn")?.isVisible = false
        }

        val usedCpuCores = findPreference<SeekBarPreference>("usedCpuCores")
        if (hostInfo.noOfCPUs <= 1) {
            usedCpuCores?.isVisible = false
        } else {
            usedCpuCores?.max = hostInfo.noOfCPUs
        }
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
        val prefs = BOINCActivity.monitor!!.prefs
        val hostInfo = BOINCActivity.monitor!!.hostInfo

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
            "deviceName" -> {
                val domainName = sharedPreferences.getString(key, "")
                findPreference<EditTextPreference>(key)?.summary = domainName
                BOINCActivity.monitor!!.setDomainName(domainName)
            }
            "theme" -> {
                val theme = sharedPreferences.getString(key, "default")!!
                findPreference<ListPreference>(key)?.summary = getThemeString(theme)
                setAppTheme(theme)
            }

            // Network
            "networkWiFiOnly" -> {
                prefs.networkWiFiOnly = sharedPreferences.getBoolean(key, true)

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "dailyTransferLimit" -> {
                val dailyTransferLimit = sharedPreferences.getString(key, prefs.dailyTransferLimitMB.toString())
                findPreference<EditTextPreference>(key)?.summary = dailyTransferLimit
                prefs.dailyTransferLimitMB = dailyTransferLimit?.toDouble() ?: 1.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Power
            "powerSources" -> {
                val powerSources = sharedPreferences.getStringSet(key,
                        resources.getStringArray(R.array.power_source_default).toSet()) ?: emptySet()
                Log.d(Logging.TAG, "powerSources: $powerSources")
                BOINCActivity.monitor!!.powerSourceAc = "wall" in powerSources
                BOINCActivity.monitor!!.powerSourceUsb = "usb" in powerSources
                BOINCActivity.monitor!!.powerSourceWireless = "wireless" in powerSources
                prefs.runOnBatteryPower = "battery" in powerSources

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "stationaryDeviceMode" -> {
                BOINCActivity.monitor!!.stationaryDeviceMode = sharedPreferences.getBoolean(key,
                        resources.getBoolean(R.bool.prefs_stationary_device_mode))
            }
            "maxBatteryTemp" -> {
                val maxBatteryTemp = sharedPreferences.getString(key, "40")
                prefs.batteryMaxTemperature = maxBatteryTemp?.toDouble() ?: 40.0
                findPreference<EditTextPreference>(key)?.summary = maxBatteryTemp

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "minBatteryLevel" -> {
                prefs.batteryChargeMinPct = sharedPreferences.getInt(key, 90).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // CPU
            "usedCpuCores" -> {
                val usedCpuCores = sharedPreferences.getInt(key, pctCpuCoresToNumber(hostInfo,
                        prefs.maxNoOfCPUsPct))
                prefs.maxNoOfCPUsPct = numberCpuCoresToPct(hostInfo, usedCpuCores)

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "cpuUsageLimit" -> {
                prefs.cpuUsageLimit = sharedPreferences.getInt(key, 100).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "suspendCpuUsage" -> {
                prefs.suspendCpuUsage = sharedPreferences.getInt(key, 50).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Storage
            "diskMaxUsedPct" -> {
                prefs.diskMaxUsedPct = sharedPreferences.getInt(key, 90).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "diskMinFreeMB" -> {
                val diskMinFreeMB = sharedPreferences.getString(key, "107")
                prefs.diskMinFreeMB = diskMinFreeMB?.toDouble() ?: 107.0
                findPreference<EditTextPreference>(key)?.summary = diskMinFreeMB

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "diskInterval" -> {
                val diskInterval = sharedPreferences.getString(key, "60")
                prefs.diskInterval = diskInterval?.toDouble() ?: 60.0
                findPreference<EditTextPreference>(key)?.summary = diskInterval

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Memory
            "maxRamUsedIdle" -> {
                prefs.ramMaxUsedIdleFrac = sharedPreferences.getInt(key, 90).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Other
            "workBufMinDays" -> {
                val workBufMinDays = sharedPreferences.getString(key, "0.1")
                prefs.workBufMinDays = workBufMinDays?.toDouble() ?: 0.0
                findPreference<EditTextPreference>(key)?.summary = workBufMinDays

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "workBufAdditionalDays" -> {
                val workBufAdditionalDays = sharedPreferences.getString(key, "0.5")
                prefs.workBufAdditionalDays = workBufAdditionalDays?.toDouble() ?: 0.0
                findPreference<EditTextPreference>(key)?.summary = workBufAdditionalDays

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Debug
            "clientLogFlags" -> {
                lifecycleScope.launch {
                    val flags = sharedPreferences.getStringSet(key, emptySet()) ?: emptySet()
                    BOINCActivity.monitor!!.setCcConfig(formatOptionsToCcConfig(flags))
                }
            }
            "logLevel" -> {
                BOINCActivity.monitor!!.logLevel = sharedPreferences.getInt(key,
                        resources.getInteger(R.integer.prefs_default_loglevel))
            }
        }
    }

    private fun getThemeString(string: String): String {
        return when (string) {
            "light" -> getString(R.string.prefs_theme_light)
            "dark" -> getString(R.string.prefs_theme_dark)
            else -> {
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
                    getString(R.string.prefs_theme_battery_saver)
                } else {
                    getString(R.string.prefs_theme_system)
                }
            }
        }
    }

    private fun pctCpuCoresToNumber(hostInfo: HostInfo, pct: Double) =
            1.0.coerceAtLeast(hostInfo.noOfCPUs.toDouble() * (pct / 100.0)).toInt()

    private fun numberCpuCoresToPct(hostInfo: HostInfo, ncpus: Int) = ncpus / hostInfo.noOfCPUs.toDouble() * 100

    private fun formatOptionsToCcConfig(options: Set<String>): String {
        val builder = StringBuilder()
        builder.append("<cc_config>\n <log_flags>\n")
        for (option in options) {
            builder.append("  <").append(option).append("/>\n")
        }
        builder.append(" </log_flags>\n <options>\n </options>\n</cc_config>")
        return builder.toString()
    }

    private suspend fun writeClientPrefs(prefs: GlobalPreferences) = coroutineScope {
        val success = async {
            return@async try {
                BOINCActivity.monitor!!.setGlobalPreferences(prefs)
            } catch (e: RemoteException) {
                false
            }
        }

        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "writeClientPrefs() async call returned: ${success.await()}")
        }
    }
}
