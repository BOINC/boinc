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
package edu.berkeley.boinc.client;

import android.Manifest.permission;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.BatteryManager;
import android.os.Build;
import android.telephony.TelephonyManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import edu.berkeley.boinc.rpc.DeviceStatusData;
import edu.berkeley.boinc.utils.Logging;
import javax.inject.Inject;
import javax.inject.Singleton;

@Singleton
public class DeviceStatus {
    // variables describing device status in RPC
    private DeviceStatusData status;

    // additional device status
    // true, if operating in stationary device mode
    private boolean stationaryDeviceMode = false;
    // true, if API returns no battery. offer preference to go into stationary device mode
    private boolean stationaryDeviceSuspected = false;
    private boolean screenOn = true;

    // android specifics
    // context required for reading device status
    private Context context;
    // connManager contains current wifi status
    private ConnectivityManager connManager;
    // telManager to retrieve call state
    private TelephonyManager telManager;
    // sticky intent, extras of Intent contain status, see BatteryManager.
    private Intent batteryStatus;
    // manager based preferences
    private AppPreferences appPrefs;

    /**
     * Constructor. Needs to be called before calling update.
     *
     * @param context Application Context
     */
    @Inject
    DeviceStatus(Context context, AppPreferences appPrefs, DeviceStatusData status) {
        this.context = context;
        this.status = status;
        this.appPrefs = appPrefs;

        connManager = ContextCompat.getSystemService(context, ConnectivityManager.class);
        telManager = ContextCompat.getSystemService(context, TelephonyManager.class);
        batteryStatus =
                context.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
    }

    /**
     * Updates device status and returns the newly received values
     *
     * @param screenOn indicator whether device screen is currently on (checked in Monitor)
     * @return DeviceStatusData, wrapper for device status
     * @throws Exception if error occurs
     */
    public DeviceStatusData update(Boolean screenOn) throws Exception {
        if(context == null) {
            throw new Exception("DeviceStatus: can not update, Context not set.");
        }
        this.screenOn = screenOn;

        boolean change = determineBatteryStatus();
        change |= determineNetworkStatus();
        change |= determineUserActive();

        if(change) {
            Logging.logDebug(Logging.Category.DEVICE,
                             "change: " + " - stationary device: " + stationaryDeviceMode +
                             " ; ac: " +
                             status.isOnACPower() + " ; level: " + status.getBatteryChargePct() +
                             " ; temperature: " + status.getBatteryTemperatureCelsius() +
                             " ; wifi: " +
                             status.isWiFiOnline() + " ; user active: " + status.isUserActive());
        }

        return status;
    }

    /**
     * Returns latest device status, without updating it.
     * If you need a up-to-date status, call update() instead.
     *
     * @return DeviceStatusData, wrapper for device status, contains data retrieved upon last update. Might be in initial state, if no update has successfully finished.
     */
    public DeviceStatusData getStatus() {
        return status;
    }

    /**
     * Returns whether API indicates that device does not have a battery
     * Not a reliable indicator, e.g. on Galaxy Nexus.
     * Offer stationary device mode preference based on its return value.
     *
     * @return true, if Android indicates absence of battery
     */
    boolean isStationaryDeviceSuspected() {
        return stationaryDeviceSuspected;
    }

    /**
     * Determines whether user is considered active.
     * Decision is also based on App preferences. User is considered active, when:
     * - telephone is active (call)
     * - screen is on AND preference "suspendWhenScreenOn" set AND NOT preference "stationaryDeviceMode" set
     *
     * @return true, if change since last run
     */
    private boolean determineUserActive() {
        boolean newUserActive;
        int telStatus;
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if(ActivityCompat.checkSelfPermission(context, permission.READ_PHONE_STATE) ==
               PackageManager.PERMISSION_GRANTED) {
                telStatus = telManager.getCallStateForSubscription();
            } else {
                telStatus = TelephonyManager.CALL_STATE_IDLE;
            }
        } else {
            @SuppressWarnings( "deprecation" )
            int telStatus_ = telManager.getCallState();
            telStatus = telStatus_;
        }

        if(telStatus != TelephonyManager.CALL_STATE_IDLE) {
            newUserActive = true;
        }
        else {
            newUserActive = (screenOn && appPrefs.getSuspendWhenScreenOn() && !appPrefs.getStationaryDeviceMode());
        }

        if(status.isUserActive() != newUserActive) {
            status.setUserActive(newUserActive);
            return true;
        }

        return false;
    }

    /**
     * Determines type of currently active network. Treats Ethernet as WiFi.
     *
     * @return true, if changed since last run
     */
    private boolean determineNetworkStatus() {
        boolean change = false;
        final boolean isWiFiOrEthernet = isNetworkTypeWiFiOrEthernet();

        if(isWiFiOrEthernet) {
            // WiFi or ethernet is online
            if(!status.isWiFiOnline()) {
                change = true; // if different from before, set flag

                Logging.logDebug(Logging.Category.DEVICE, "Unlimited Internet connection - WiFi or ethernet - found");
            }
            status.setWiFiOnline(true);
        }
        else {
            // WiFi and ethernet are offline
            if(status.isWiFiOnline()) {
                change = true; // if different from before, set flag
            }
            status.setWiFiOnline(false);
        }

        return change;
    }

    private boolean isNetworkTypeWiFiOrEthernet() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            final Network network = connManager.getActiveNetwork();
            if (network != null) {
                final NetworkCapabilities networkCapabilities = connManager
                        .getNetworkCapabilities(network);
                if (networkCapabilities != null) {
                    return networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) ||
                           networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET);
                }
            }
            return false;
        } else {
            @SuppressWarnings( "deprecation" )
            final android.net.NetworkInfo activeNetwork = connManager.getActiveNetworkInfo();
            if(activeNetwork == null) {
                return false;
            }
            @SuppressWarnings( "deprecation" )
            final int networkType = activeNetwork.getType();
            @SuppressWarnings( "deprecation" )
            final boolean result = networkType == ConnectivityManager.TYPE_WIFI ||
                             networkType == ConnectivityManager.TYPE_ETHERNET;
            return result;
        }
    }

    /**
     * Determines battery status of device
     *
     * @return true, if change since last run
     * @throws Exception if error occurs
     */
    private boolean determineBatteryStatus() throws Exception {
        // check battery
        boolean change = false;
        batteryStatus = context.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        if(batteryStatus != null) {
            stationaryDeviceSuspected =
                    !batteryStatus.getBooleanExtra(BatteryManager.EXTRA_PRESENT, true); // if no battery present, suspect stationary device
            if(appPrefs.getStationaryDeviceMode() && stationaryDeviceSuspected) {
                // API says no battery present (not reliable, e.g. Galaxy Nexus)
                // AND stationary device mode is enabled in preferences

                if(!stationaryDeviceMode) { // should not change during run-time. just triggered on initial read
                    change = true;

                    Logging.logInfo(Logging.Category.DEVICE, "No battery found and stationary device mode enabled in preferences -> skip battery status parsing");
                }
                stationaryDeviceMode = true;
                setAttributesForStationaryDevice();
            }
            else {
                // battery present OR stationary device mode not enabled
                // parse and report actual values to client

                if(stationaryDeviceMode) {
                    change = true;
                }
                stationaryDeviceMode = false;

                // calculate charging level
                int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
                int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
                if(level == -1 || scale == -1) {
                    throw new Exception("battery level parsing error");
                }
                int batteryPct = (int) ((level / (float) scale) * 100); // always rounds down
                if(batteryPct < 0 || batteryPct > 100) {
                    throw new Exception("battery level parsing error");
                }
                if(batteryPct != status.getBatteryChargePct()) {
                    status.setBatteryChargePct(batteryPct);
                    change = true;
                }

                // temperature
                int temperature =
                        batteryStatus.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1) / 10; // always rounds down
                if(temperature < 0) {
                    throw new Exception("temperature parsing error");
                }
                if(temperature != status.getBatteryTemperatureCelsius()) {
                    status.setBatteryTemperatureCelsius(temperature);
                    change = true;
                }

                // plugged in
                // treat all charging modes uniformly on client side,
                // adapt on_ac_power according to power source preferences defined in manager
                int plugged = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
                change |= setAttributesForChargerType(plugged);
            }
        }
        else {
            throw new Exception("battery intent null");
        }
        return change;
    }

    /**
     * Sets attributes of DeviceStatusData according to stationary device.
     * If stationary device, allow computation independently of battery status,
     * is not handled in the client, but positive values are simulated here.
     * The policy might be subject to change.
     */
    private void setAttributesForStationaryDevice() {
        status.setOnACPower(true);
        status.setBatteryTemperatureCelsius(0);
        status.setBatteryChargePct(100);
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
    private boolean setAttributesForChargerType(int chargerType) {
        boolean change = false;
        boolean enabled = false;

        switch(chargerType) {
            case BatteryManager.BATTERY_PLUGGED_AC:
                enabled = appPrefs.getPowerSourceAc();
                break;
            case BatteryManager.BATTERY_PLUGGED_WIRELESS:
                enabled = appPrefs.getPowerSourceWireless();
                break;
            case BatteryManager.BATTERY_PLUGGED_USB:
                enabled = appPrefs.getPowerSourceUsb();
                break;
            default:
                break;
        }

        if(enabled) {
            if(!status.isOnACPower()) {
                change = true; // if different from before, set flag
            }
            status.setOnACPower(true);
        }
        else {
            if(status.isOnACPower()) {
                change = true;
            }
            status.setOnACPower(false);
        }

        return change;
    }
}
