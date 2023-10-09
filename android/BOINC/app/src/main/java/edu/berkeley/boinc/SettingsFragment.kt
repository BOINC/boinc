/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import android.content.Intent
import android.content.SharedPreferences
import android.os.Bundle
import android.provider.Settings
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.biometric.BiometricManager
import androidx.biometric.BiometricManager.Authenticators.BIOMETRIC_STRONG
import androidx.biometric.BiometricManager.Authenticators.BIOMETRIC_WEAK
import androidx.biometric.BiometricManager.Authenticators.DEVICE_CREDENTIAL
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import androidx.core.content.edit
import androidx.preference.CheckBoxPreference
import androidx.preference.EditTextPreference
import androidx.preference.Preference
import androidx.preference.PreferenceCategory
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.PreferenceManager
import androidx.preference.SeekBarPreference
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.HostInfo
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.setAppTheme
import java.io.File
import java.util.concurrent.Executor
import java.util.concurrent.ThreadLocalRandom
import kotlin.streams.asSequence


class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {
    private val hostInfo = BOINCActivity.monitor!!.hostInfo // Get the hostinfo from client via RPC
    private val prefs = BOINCActivity.monitor!!.prefs
    private val charPool : List<Char> = ('a'..'z') + ('A'..'Z') + ('0'..'9')
    private val passwordLength = 32
    private var authKey = ""
    private lateinit var biometricPrompt: BiometricPrompt
    private lateinit var authenticationPopupView: View
    private lateinit var authenticationPopupEditText: EditText

    override fun onResume() {
        super.onResume()
        preferenceManager.sharedPreferences!!.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences!!.unregisterOnSharedPreferenceChangeListener(this)
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(requireContext())
        if ("usedCpuCores" !in sharedPreferences) {
            sharedPreferences.edit { putInt("usedCpuCores", pctCpuCoresToNumber(hostInfo, prefs.maxNoOfCPUsPct)) }
        }
        if ("deviceName" !in sharedPreferences) {
            sharedPreferences.edit { putString("deviceName", hostInfo.domainName) }
        }

        if(authKey.isEmpty()) {
            authKey = readAuthFileContent()
            if (authKey.isEmpty()) {
                authKey = generateRandomString(passwordLength)
                writeAuthFileContent(authKey)
            }
            sharedPreferences.edit { putString("authenticationKey", authKey) }
        }

        val stationaryDeviceMode = BOINCActivity.monitor!!.stationaryDeviceMode
        val stationaryDeviceSuspected = BOINCActivity.monitor!!.isStationaryDeviceSuspected

        preferenceManager.sharedPreferences!!.registerOnSharedPreferenceChangeListener(this)

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

        authenticationPopupView = LayoutInflater.from(context).inflate(R.layout.authenticationkey_preference_dialog, null)
        authenticationPopupEditText = authenticationPopupView.findViewById(R.id.authentication_key_input)

        val preference = findPreference<Preference>("authenticationKey")!!

        val executor: Executor = ContextCompat.getMainExecutor(requireContext())

        biometricPrompt = BiometricPrompt(this, executor, object : BiometricPrompt.AuthenticationCallback() {
            override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                super.onAuthenticationSucceeded(result)
                preference.isEnabled = true
                authenticationPopup(sharedPreferences)
            }

            override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                super.onAuthenticationError(errorCode, errString)
                Toast.makeText(context, "Authentication Error", Toast.LENGTH_SHORT).show()
            }
        })

        preference.summaryProvider = Preference.SummaryProvider<Preference> {
            getString(R.string.prefs_remote_boinc_relaunched) + '\n' +
                    setAsterisks(authKey.length)
        }

        val biometricManager = BiometricManager.from(this.requireContext())

        preference.setOnPreferenceClickListener {
            when (biometricManager.canAuthenticate(BIOMETRIC_STRONG or BIOMETRIC_WEAK or DEVICE_CREDENTIAL)) {
                BiometricManager.BIOMETRIC_SUCCESS -> {
                    val promptInfo = BiometricPrompt.PromptInfo.Builder()
                        .setTitle("Authenticate")
                        .setSubtitle("Use biometric authentication to reveal or edit the authentication key.")
                        .setAllowedAuthenticators(DEVICE_CREDENTIAL or BIOMETRIC_WEAK or BIOMETRIC_STRONG)
                        .build()
                    biometricPrompt.authenticate(promptInfo)
                }
                BiometricManager.BIOMETRIC_ERROR_NONE_ENROLLED -> {
                    val enrollIntent = Intent(Settings.ACTION_BIOMETRIC_ENROLL).apply {
                        putExtra(Settings.EXTRA_BIOMETRIC_AUTHENTICATORS_ALLOWED,
                            BIOMETRIC_WEAK or DEVICE_CREDENTIAL)
                    }
                    startActivityForResult(enrollIntent, 0)
                }
                else -> authenticationPopup(sharedPreferences)
            }
            true
        }
    }

    private fun authenticationPopup(sharedPreferences: SharedPreferences) {
        val builder = AlertDialog.Builder(requireContext())

        if (authenticationPopupView.parent != null) {
            (authenticationPopupView.parent as ViewGroup).removeView(authenticationPopupView)
        }

        builder.setView(authenticationPopupView)

        val currentAuthKey = sharedPreferences.getString("authenticationKey", "")!!
        authenticationPopupEditText.setText(currentAuthKey)


        builder.setPositiveButton("OK") { _, _ ->
            val enteredText = authenticationPopupEditText.text.toString()
            sharedPreferences.edit { putString("authenticationKey", enteredText) }
        }

        builder.setNegativeButton("Cancel") { dialog, _ ->
            dialog.dismiss()
        }

        builder.create().show()
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
            "deviceName" -> BOINCActivity.monitor!!.setDomainName(sharedPreferences.getString(key, "") ?: "")
            "theme" -> setAppTheme(sharedPreferences.getString(key, "light")!!)

            // Network
            "networkWiFiOnly" -> {
                prefs.networkWiFiOnly = sharedPreferences.getBoolean(key, true)
                writeClientPrefs(prefs)
            }
            "dailyTransferLimitMB" -> {
                val dailyTransferLimitMB = sharedPreferences.getString(key, prefs.dailyTransferLimitMB.toString())
                prefs.dailyTransferLimitMB = dailyTransferLimitMB?.toDouble() ?: 0.0
                writeClientPrefs(prefs)
            }
            "dailyTransferPeriodDays" -> {
                val dailyTransferPeriodDays = sharedPreferences.getString(key, prefs.dailyTransferPeriodDays.toString())
                prefs.dailyTransferPeriodDays = dailyTransferPeriodDays?.toInt() ?: 0
                writeClientPrefs(prefs)
            }

            // Power
            "powerSources" -> {
                val powerSources = sharedPreferences.getStringSet(key,
                        resources.getStringArray(R.array.power_source_default).toSet()) ?: emptySet()
                Logging.logDebug(Logging.Category.SETTINGS, "powerSources: $powerSources")
                BOINCActivity.monitor!!.powerSourceAc = "wall" in powerSources
                BOINCActivity.monitor!!.powerSourceUsb = "usb" in powerSources
                BOINCActivity.monitor!!.powerSourceWireless = "wireless" in powerSources
                prefs.runOnBatteryPower = "battery" in powerSources
                writeClientPrefs(prefs)
            }
            "stationaryDeviceMode" -> BOINCActivity.monitor!!.stationaryDeviceMode = sharedPreferences.getBoolean(key, false)
            "maxBatteryTemp" -> {
                prefs.batteryMaxTemperature = sharedPreferences.getString(key, "40")?.toDouble() ?: 40.0
                writeClientPrefs(prefs)
            }
            "minBatteryLevel" -> {
                prefs.batteryChargeMinPct = sharedPreferences.getInt(key, 90).toDouble()
                writeClientPrefs(prefs)
            }

            // CPU
            "usedCpuCores" -> {
                val usedCpuCores = sharedPreferences.getInt(key, pctCpuCoresToNumber(hostInfo,
                        prefs.maxNoOfCPUsPct))
                prefs.maxNoOfCPUsPct = numberCpuCoresToPct(hostInfo, usedCpuCores)
                writeClientPrefs(prefs)
            }
            "cpuUsageLimit" -> {
                prefs.cpuUsageLimit = sharedPreferences.getInt(key, 100).toDouble()
                writeClientPrefs(prefs)
            }
            "suspendCpuUsage" -> {
                prefs.suspendCpuUsage = sharedPreferences.getInt(key, 50).toDouble()
                writeClientPrefs(prefs)
            }

            // Storage
            "diskMaxUsedPct" -> {
                prefs.diskMaxUsedPct = sharedPreferences.getInt(key, 90).toDouble()
                writeClientPrefs(prefs)
            }
            "diskMinFreeGB" -> {
                prefs.diskMinFreeGB = sharedPreferences.getString(key, "0.1")?.toDouble() ?: 0.1
                writeClientPrefs(prefs)
            }
            "diskInterval" -> {
                prefs.diskInterval = sharedPreferences.getString(key, "60")?.toDouble() ?: 60.0
                writeClientPrefs(prefs)
            }

            // Memory
            "maxRamUsedIdle" -> {
                prefs.ramMaxUsedIdleFrac = sharedPreferences.getInt(key, 50).toDouble()
                writeClientPrefs(prefs)
            }

            // Other
            "workBufMinDays" -> {
                prefs.workBufMinDays = sharedPreferences.getString(key, "0.1")?.toDouble() ?: 0.1
                writeClientPrefs(prefs)
            }
            "workBufAdditionalDays" -> {
                prefs.workBufAdditionalDays = sharedPreferences.getString(key, "0.5")?.toDouble() ?: 0.5
                writeClientPrefs(prefs)
            }

            // Remote
            "authenticationKey" -> {
                val currentAuthKey = sharedPreferences.getString(key, "")!!
                if (currentAuthKey.isEmpty()) {
                    sharedPreferences.edit { putString(key, authKey) }
                    Toast.makeText(activity, R.string.prefs_remote_empty_password, Toast.LENGTH_SHORT).show()
                } else {
                    authKey = currentAuthKey
                    writeAuthFileContent(authKey)
                    quitClient()
                }
            }

            "remoteEnable" -> {
                val isRemote = sharedPreferences.getBoolean(key, false)
                BOINCActivity.monitor!!.isRemote = isRemote
                findPreference<Preference>("authenticationKey")?.isVisible = isRemote
                quitClient()
            }

            // Debug
            "clientLogFlags" -> {
                val flags = sharedPreferences.getStringSet(key, emptySet()) ?: emptySet()
                BOINCActivity.monitor!!.setCcConfigAsync(formatOptionsToCcConfig(flags))
            }
            "guiLogCategories" -> {
                val categories = (sharedPreferences.getStringSet(key, emptySet()) ?: emptySet()).toList()
                BOINCActivity.monitor!!.logCategories = categories
                Logging.setLogCategories(categories)
            }
            "logLevel" -> {
                val logLevel = sharedPreferences.getInt(key,
                    resources.getInteger(R.integer.prefs_default_loglevel))
                BOINCActivity.monitor!!.logLevel = logLevel
                Logging.setLogLevel(logLevel)
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

    private fun setAdvancedPreferencesVisibility() {
        val showAdvanced = BOINCActivity.monitor!!.showAdvanced

        findPreference<EditTextPreference>("dailyTransferLimitMB")?.isVisible = showAdvanced
        findPreference<EditTextPreference>("dailyTransferPeriodDays")?.isVisible = showAdvanced

        findPreference<PreferenceCategory>("cpu")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("storage")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("memory")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("other")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("debug")?.isVisible = showAdvanced
        findPreference<PreferenceCategory>("remote")?.isVisible = showAdvanced
        val isRemote = findPreference<CheckBoxPreference>("remoteEnable")?.isChecked
        findPreference<Preference>("authenticationKey")?.isVisible = showAdvanced && isRemote == true
    }

    private fun writeClientPrefs(prefs: GlobalPreferences) {
        BOINCActivity.monitor!!.setGlobalPreferencesAsync(prefs) {
            success: Boolean ->
                Logging.logDebug(Logging.Category.SETTINGS, "writeClientPrefs() async call returned: $success")
        }
    }

    private fun generateRandomString(length: Int) : String {
        return ThreadLocalRandom.current()
                .ints(length.toLong(), 0, charPool.size)
                .asSequence()
                .map(charPool::get)
                .joinToString("")
    }

    // Return the password in asterisks
    private fun setAsterisks(length: Int): String {
        return "*".repeat(length)
    }

    private fun readAuthFileContent(): String {
        val authFile = File(BOINCActivity.monitor!!.authFilePath)
        return if (authFile.exists()) authFile.bufferedReader().readLine() else ""
    }

    private fun writeAuthFileContent(value: String) {
        File(BOINCActivity.monitor!!.authFilePath).writeText(value)
    }

    private fun quitClient() {
        BOINCActivity.monitor!!.quitClientAsync { result: Boolean ->
            Logging.logDebug(Logging.Category.SETTINGS, "SettingActivity: quitClient returned: $result")
        }
    }
}
