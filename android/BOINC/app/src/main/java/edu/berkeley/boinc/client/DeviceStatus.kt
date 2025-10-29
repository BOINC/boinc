/*
* This file is part of BOINC.
* https://boinc.berkeley.edu
* Copyright (C) 2025 University of California
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

import android.Manifest.permission
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.BatteryManager
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.telephony.TelephonyManager
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import edu.berkeley.boinc.rpc.DeviceStatusData
import edu.berkeley.boinc.utils.Logging.Category.DEVICE
import edu.berkeley.boinc.utils.Logging.logDebug
import edu.berkeley.boinc.utils.Logging.logInfo
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class DeviceStatus @Inject internal constructor(
    // android specifics
    // context required for reading device status
    private val context: Context, // manager based preferences
    private val appPrefs: AppPreferences,
    /**
     * Returns latest device status, without updating it.
     * If you need a up-to-date status, call update() instead.
     *
     * @return DeviceStatusData, wrapper for device status, contains data retrieved upon last update. Might be in initial state, if no update has successfully finished.
     */
    // variables describing device status in RPC
    val status: DeviceStatusData
) {
    // additional device status
    // true, if operating in stationary device mode
    private var stationaryDeviceMode = false

    /**
     * Returns whether API indicates that device does not have a battery
     * Not a reliable indicator, e.g. on Galaxy Nexus.
     * Offer stationary device mode preference based on its return value.
     *
     * @return true, if Android indicates absence of battery
     */
    // true, if API returns no battery. offer preference to go into stationary device mode
    var isStationaryDeviceSuspected: Boolean = false
        private set
    private var screenOn = true

    // connManager contains current wifi status
    private val connManager = ContextCompat.getSystemService(
        context,
        ConnectivityManager::class.java
    )

    // telManager to retrieve call state
    private val telManager =
        ContextCompat.getSystemService(context, TelephonyManager::class.java)

    // sticky intent, extras of Intent contain status, see BatteryManager.
    private var batteryStatus: Intent?

    init {
        batteryStatus =
            context.registerReceiver(null, IntentFilter(Intent.ACTION_BATTERY_CHANGED))
    }

    /**
     * Updates device status and returns the newly received values
     *
     * @param screenOn indicator whether device screen is currently on (checked in Monitor)
     * @return DeviceStatusData, wrapper for device status
     * @throws Exception if error occurs
     */
    @Throws(Exception::class)
    fun update(screenOn: Boolean): DeviceStatusData {
        this.screenOn = screenOn

        var change = determineBatteryStatus()
        change = change or determineNetworkStatus()
        change = change or determineUserActive()

        if (change) {
            logDebug(
                DEVICE,
                "change: " + " - stationary device: " + stationaryDeviceMode +
                        " ; ac: " +
                        status.isOnACPower + " ; level: " + status.batteryChargePct +
                        " ; temperature: " + status.batteryTemperatureCelsius +
                        " ; wifi: " +
                        status.isWiFiOnline + " ; user active: " + status.isUserActive
            )
        }

        return status
    }

    /**
     * Determines whether user is considered active.
     * Decision is also based on App preferences. User is considered active, when:
     * - telephone is active (call)
     * - screen is on AND preference "suspendWhenScreenOn" set AND NOT preference "stationaryDeviceMode" set
     *
     * @return true, if change since last run
     */
    private fun determineUserActive(): Boolean {
        val newUserActive: Boolean
        val telStatus: Int
        if (VERSION.SDK_INT >= VERSION_CODES.S) {
            telStatus =
                if (ActivityCompat.checkSelfPermission(context, permission.READ_PHONE_STATE) ==
                    PackageManager.PERMISSION_GRANTED
                ) {
                    telManager!!.callStateForSubscription
                } else {
                    TelephonyManager.CALL_STATE_IDLE
                }
        } else {
            @Suppress("deprecation")
            telStatus = telManager!!.callState
        }

        newUserActive = if (telStatus != TelephonyManager.CALL_STATE_IDLE) {
            true
        } else {
            (screenOn && appPrefs.suspendWhenScreenOn && !appPrefs.stationaryDeviceMode)
        }

        if (status.isUserActive != newUserActive) {
            status.isUserActive = newUserActive
            return true
        }

        return false
    }

    /**
     * Determines type of currently active network. Treats Ethernet as WiFi.
     *
     * @return true, if changed since last run
     */
    private fun determineNetworkStatus(): Boolean {
        var change = false
        val isWiFiOrEthernet = isNetworkTypeWiFiOrEthernet

        if (isWiFiOrEthernet) {
            // WiFi or ethernet is online
            if (!status.isWiFiOnline) {
                change = true // if different from before, set flag

                logDebug(DEVICE, "Unlimited Internet connection - WiFi or ethernet - found")
            }
            status.isWiFiOnline = true
        } else {
            // WiFi and ethernet are offline
            if (status.isWiFiOnline) {
                change = true // if different from before, set flag
            }
            status.isWiFiOnline = false
        }

        return change
    }

    private val isNetworkTypeWiFiOrEthernet: Boolean
        get() {
            if (VERSION.SDK_INT >= VERSION_CODES.M) {
                val network = connManager!!.activeNetwork
                if (network != null) {
                    val networkCapabilities = connManager
                        .getNetworkCapabilities(network)
                    if (networkCapabilities != null) {
                        return networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) ||
                                networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET)
                    }
                }
                return false
            } else {
                @Suppress("deprecation") val activeNetwork =
                    connManager!!.activeNetworkInfo ?: return false
                @Suppress("deprecation") val networkType = activeNetwork.type
                @Suppress("deprecation") val result =
                    networkType == ConnectivityManager.TYPE_WIFI ||
                            networkType == ConnectivityManager.TYPE_ETHERNET
                return result
            }
        }

    /**
     * Determines battery status of device
     *
     * @return true, if change since last run
     * @throws Exception if error occurs
     */
    @Throws(Exception::class)
    private fun determineBatteryStatus(): Boolean {
        // check battery
        var change = false
        batteryStatus =
            context.registerReceiver(null, IntentFilter(Intent.ACTION_BATTERY_CHANGED))
        if (batteryStatus != null) {
            isStationaryDeviceSuspected =
                !batteryStatus!!.getBooleanExtra(
                    BatteryManager.EXTRA_PRESENT,
                    true
                ) // if no battery present, suspect stationary device
            if (appPrefs.stationaryDeviceMode && isStationaryDeviceSuspected) {
                // API says no battery present (not reliable, e.g. Galaxy Nexus)
                // AND stationary device mode is enabled in preferences

                if (!stationaryDeviceMode) { // should not change during run-time. just triggered on initial read
                    change = true

                    logInfo(
                        DEVICE,
                        "No battery found and stationary device mode enabled in preferences -> skip battery status parsing"
                    )
                }
                stationaryDeviceMode = true
                setAttributesForStationaryDevice()
            } else {
                // battery present OR stationary device mode not enabled
                // parse and report actual values to client

                if (stationaryDeviceMode) {
                    change = true
                }
                stationaryDeviceMode = false

                // calculate charging level
                val level = batteryStatus!!.getIntExtra(BatteryManager.EXTRA_LEVEL, -1)
                val scale = batteryStatus!!.getIntExtra(BatteryManager.EXTRA_SCALE, -1)
                if (level == -1 || scale == -1) {
                    throw Exception("battery level parsing error")
                }
                val batteryPct = ((level / scale.toFloat()) * 100).toInt() // always rounds down
                if (batteryPct < 0 || batteryPct > 100) {
                    throw Exception("battery level parsing error")
                }
                if (batteryPct != status.batteryChargePct) {
                    status.batteryChargePct = batteryPct
                    change = true
                }

                // temperature
                val temperature =
                    batteryStatus!!.getIntExtra(
                        BatteryManager.EXTRA_TEMPERATURE,
                        -1
                    ) / 10 // always rounds down
                if (temperature < 0) {
                    throw Exception("temperature parsing error")
                }
                if (temperature != status.batteryTemperatureCelsius) {
                    status.batteryTemperatureCelsius = temperature
                    change = true
                }

                // plugged in
                // treat all charging modes uniformly on client side,
                // adapt on_ac_power according to power source preferences defined in manager
                val plugged = batteryStatus!!.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1)
                change = change or setAttributesForChargerType(plugged)
            }
        } else {
            throw Exception("battery intent null")
        }
        return change
    }

    /**
     * Sets attributes of DeviceStatusData according to stationary device.
     * If stationary device, allow computation independently of battery status,
     * is not handled in the client, but positive values are simulated here.
     * The policy might be subject to change.
     */
    private fun setAttributesForStationaryDevice() {
        status.isOnACPower = true
        status.batteryTemperatureCelsius = 0
        status.batteryChargePct = 100
    }

    /**
     * Sets attributes of DeviceStatusData according to manager based power source preference.
     * Client is not aware of power source preference, adapt value of on_ac_power, according
     * to the conformance of the actual charger type with the manager based preference.
     * This policy might be subject to change.
     *
     * @param chargerType BatteryManager class
     * @return true, if change since last run
     */
    private fun setAttributesForChargerType(chargerType: Int): Boolean {
        var change = false
        var enabled = false

        when (chargerType) {
            BatteryManager.BATTERY_PLUGGED_AC -> enabled = appPrefs.powerSourceAc
            BatteryManager.BATTERY_PLUGGED_WIRELESS -> enabled = appPrefs.powerSourceWireless
            BatteryManager.BATTERY_PLUGGED_USB -> enabled = appPrefs.powerSourceUsb
            else -> {}
        }

        if (enabled) {
            if (!status.isOnACPower) {
                change = true // if different from before, set flag
            }
            status.isOnACPower = true
        } else {
            if (status.isOnACPower) {
                change = true
            }
            status.isOnACPower = false
        }

        return change
    }
}
