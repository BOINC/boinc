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
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences.unregisterOnSharedPreferenceChangeListener(this)
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        val hostInfo = BOINCActivity.monitor!!.hostInfo // Get the hostinfo from client via RPC
        val prefs = BOINCActivity.monitor!!.prefs

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)
        if (!sharedPreferences.contains("usedCpuCores")) {
            sharedPreferences.edit { putInt("usedCpuCores", pctCpuCoresToNumber(hostInfo, prefs.maxNoOfCPUsPct)) }
        }

        val stationaryDeviceMode = BOINCActivity.monitor!!.stationaryDeviceMode
        val stationaryDeviceSuspected = BOINCActivity.monitor!!.isStationaryDeviceSuspected

        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        setPreferencesFromResource(R.xml.root_preferences, rootKey)

        findPreference<EditTextPreference>("deviceName")?.text = BOINCActivity.monitor!!.hostInfo.domainName

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
            "deviceName" -> BOINCActivity.monitor!!.setDomainName(sharedPreferences.getString(key, ""))
            "theme" -> setAppTheme(sharedPreferences.getString(key, "default")!!)

            // Network
            "networkWiFiOnly" -> {
                prefs.networkWiFiOnly = sharedPreferences.getBoolean(key, true)

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "dailyTransferLimit" -> {
                prefs.dailyTransferLimitMB = sharedPreferences.getString(key,
                        prefs.dailyTransferLimitMB.toString())?.toDouble()
                        ?: 1.0

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

            // CPU
            "usedCpuCores" -> {
                val usedCpuCores = sharedPreferences.getInt(key, pctCpuCoresToNumber(hostInfo, prefs.maxNoOfCPUsPct))
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
                prefs.diskMinFreeMB = sharedPreferences.getString(key, "107")?.toDouble() ?: 0.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "diskInterval" -> {
                prefs.diskInterval = sharedPreferences.getString(key, "60")?.toDouble() ?: 0.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Memory
            "maxRamUsedIdle" -> {
                prefs.ramMaxUsedIdleFrac = sharedPreferences.getInt(key, 90).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Other
            "workBufMinDays" -> {
                prefs.workBufMinDays = sharedPreferences.getString(key, "0.1")?.toDouble() ?: 0.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "workBufAdditionalDays" -> {
                prefs.workBufAdditionalDays = sharedPreferences.getString(key, "0.5")?.toDouble() ?: 0.0

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
