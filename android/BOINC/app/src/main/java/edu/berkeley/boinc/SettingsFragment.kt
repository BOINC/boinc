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
import java.io.File
import java.util.concurrent.ThreadLocalRandom
import kotlin.streams.asSequence

class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {
    private val hostInfo = BOINCActivity.monitor!!.hostInfo // Get the hostinfo from client via RPC
    private val prefs = BOINCActivity.monitor!!.prefs
    private val charPool : List<Char> = ('a'..'z') + ('A'..'Z') + ('0'..'9')
    private val STRING_LENGTH = 32;


    override fun onResume() {
        super.onResume()
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences.unregisterOnSharedPreferenceChangeListener(this)
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)
        if ("usedCpuCores" !in sharedPreferences) {
            sharedPreferences.edit { putInt("usedCpuCores", pctCpuCoresToNumber(hostInfo, prefs.maxNoOfCPUsPct)) }
        }
        if ("deviceName" !in sharedPreferences) {
            sharedPreferences.edit { putString("deviceName", hostInfo.domainName) }
        }

        var autKey = readAutFileContent()
        if (autKey.isEmpty()) {
            autKey = generateRandomString()
            writeAutFileContent(autKey)
        }
        if ("authenticationKey" !in sharedPreferences) {
            sharedPreferences.edit { putString("authenticationKey", autKey) }
        }

        val stationaryDeviceMode = BOINCActivity.monitor!!.stationaryDeviceMode
        val stationaryDeviceSuspected = BOINCActivity.monitor!!.isStationaryDeviceSuspected

        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        setPreferencesFromResource(R.xml.root_preferences, rootKey)

        setAdvancedPreferencesVisibility()

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

        val preference = findPreference<EditTextPreference>("authenticationKey")!!
        preference.setSummaryProvider {
            val autKey = sharedPreferences.getString("authenticationKey", "")
            setAsterisks(autKey!!.length)
        }
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
        when (key) {
            // General
            "autostart" -> BOINCActivity.monitor!!.autostart = sharedPreferences.getBoolean(key, true)
            "showNotifications" -> BOINCActivity.monitor!!.showNotificationForNotices = sharedPreferences.getBoolean(key, true)
            "showAdvanced" -> {
                BOINCActivity.monitor!!.showAdvanced = sharedPreferences.getBoolean(key, false)
                setAdvancedPreferencesVisibility()
            }
            "suspendWhenScreenOn" -> BOINCActivity.monitor!!.suspendWhenScreenOn = sharedPreferences.getBoolean(key, true)
            "deviceName" -> BOINCActivity.monitor!!.setDomainName(sharedPreferences.getString(key, ""))
            "theme" -> setAppTheme(sharedPreferences.getString(key, "light")!!)

            // Network
            "networkWiFiOnly" -> {
                prefs.networkWiFiOnly = sharedPreferences.getBoolean(key, true)

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "dailyTransferLimitMB" -> {
                val dailyTransferLimitMB = sharedPreferences.getString(key, prefs.dailyTransferLimitMB.toString())
                prefs.dailyTransferLimitMB = dailyTransferLimitMB?.toDouble() ?: 0.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "dailyTransferPeriodDays" -> {
                val dailyTransferPeriodDays = sharedPreferences.getString(key, prefs.dailyTransferPeriodDays.toString())
                prefs.dailyTransferPeriodDays = dailyTransferPeriodDays?.toInt() ?: 0

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
            "stationaryDeviceMode" -> BOINCActivity.monitor!!.stationaryDeviceMode = sharedPreferences.getBoolean(key, false)
            "maxBatteryTemp" -> {
                prefs.batteryMaxTemperature = sharedPreferences.getString(key, "40")?.toDouble() ?: 40.0

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
            "diskMinFreeGB" -> {
                prefs.diskMinFreeGB = sharedPreferences.getString(key, "0.1")?.toDouble() ?: 0.1

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "diskInterval" -> {
                prefs.diskInterval = sharedPreferences.getString(key, "60")?.toDouble() ?: 60.0

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Memory
            "maxRamUsedIdle" -> {
                prefs.ramMaxUsedIdleFrac = sharedPreferences.getInt(key, 50).toDouble()

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            // Other
            "workBufMinDays" -> {
                prefs.workBufMinDays = sharedPreferences.getString(key, "0.1")?.toDouble() ?: 0.1

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }
            "workBufAdditionalDays" -> {
                prefs.workBufAdditionalDays = sharedPreferences.getString(key, "0.5")?.toDouble() ?: 0.5

                lifecycleScope.launch { writeClientPrefs(prefs) }
            }

            "authenticationKey" -> {
                val autPath = BOINCActivity.monitor!!.authFilePath
                var autKey = sharedPreferences.getString(key, "")!!
                if (autKey != "") {
                    File(autPath).writeText(autKey)
                }
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

    private fun generateRandomString() : String {
        return ThreadLocalRandom.current()
                .ints(STRING_LENGTH.toLong(), 0, charPool.size)
                .asSequence()
                .map(charPool::get)
                .joinToString("")
    }

    private fun formatOptionsToCcConfig(options: Set<String>): String {
        val builder = StringBuilder()
        builder.append("<cc_config>\n <log_flags>\n")
        for (option in options) {
            builder.append("  <").append(option).append("/>\n")
        }
        builder.append(" </log_flags>\n <options>\n </options>\n</cc_config>")
        return builder.toString()
    }

    private fun setAdvancedPreferencesVisibility() {
        val showAdvanced = BOINCActivity.monitor!!.showAdvanced

        findPreference<EditTextPreference>("dailyTransferLimitMB")?.isVisible = showAdvanced
        findPreference<EditTextPreference>("dailyTransferPeriodDays")?.isVisible = showAdvanced

        findPreference<PreferenceCategory>("cpu")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("storage")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("memory")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("other")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("debug")?.isVisible = showAdvanced
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

    // Return the password in asterisks
    private fun setAsterisks(length: Int): String {
        val sb = java.lang.StringBuilder()
        for (s in 0 until length) {
            sb.append("*") }
        return sb.toString()
    }

    private fun readAutFileContent(): String {
        val autPath = BOINCActivity.monitor!!.authFilePath
        val autFile = File(autPath)
        var autKey = ""
        if (autFile.exists()) {
            autKey = autFile.bufferedReader().readLine()
        }
        return autKey
    }

    private fun writeAutFileContent(value: String) {
        val autPath = BOINCActivity.monitor!!.authFilePath
        val autFile = File(autPath)
        autFile.writeText(value)
    }    
}
