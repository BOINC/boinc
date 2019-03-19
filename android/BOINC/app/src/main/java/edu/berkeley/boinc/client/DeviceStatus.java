/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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

import edu.berkeley.boinc.rpc.DeviceStatusData;
import edu.berkeley.boinc.utils.*;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.BatteryManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class DeviceStatus {

    // variables describing device status in RPC
    private DeviceStatusData status = new DeviceStatusData();

    // additional device status
    // true, if operating in stationary device mode
    private boolean stationaryDeviceMode = false;
    // true, if API returns no battery. offer preference to go into stationary device mode
    private boolean stationaryDeviceSuspected = false;
    private boolean screenOn = true;

    // android specifics
    // context required for reading device status
    private Context ctx;
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
     * @param ctx Application Context
     */
    public DeviceStatus(Context ctx, AppPreferences appPrefs) {
        this.ctx = ctx;
        this.connManager = (ConnectivityManager) ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
        this.telManager = (TelephonyManager) ctx.getSystemService(Context.TELEPHONY_SERVICE);
        this.batteryStatus = ctx.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        this.appPrefs = appPrefs;
    }

    /**
     * Updates device status and returns the newly received values
     *
     * @param screenOn indicator whether device screen is currently on (checked in Monitor)
     * @return DeviceStatusData, wrapper for device status
     * @throws Exception if error occurs
     */
    public DeviceStatusData update(Boolean screenOn) throws Exception {
        if(ctx == null) {
            throw new Exception("DeviceStatus: can not update, Context not set.");
        }
        this.screenOn = screenOn;

        Boolean change = determineBatteryStatus();
        change = change | determineNetworkStatus();
        change = change | determineUserActive();

        if(change) {
            if(Logging.DEBUG) {
                Log.i(Logging.TAG, "change: " + change +
                                   " - stationary device: " + stationaryDeviceMode +
                                   " ; ac: " + status.on_ac_power +
                                   " ; level: " + status.battery_charge_pct +
                                   " ; temperature: " + status.battery_temperature_celsius +
                                   " ; wifi: " + status.wifi_online +
                                   " ; user active: " + status.user_active);
            }
        }

        return status;
    }

    /**
     * Returns latest device status, without updating it.
     * If you need a up-to-date status, call udpate() instead.
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
    public Boolean isStationaryDeviceSuspected() {
        return stationaryDeviceSuspected;
    }

    /**
     * Determines whether user is considered active.
     * Decision is also based on App preferences. User is considered active, when:
     * - telephone is active (call)
     * - screen is on AND preference "suspendWhenScreenOn" set AND NOT preference "stationaryDeviceMode" set
     *
     * @return true, if change since last run
     * @throws Exception if error occurs
     */
    private Boolean determineUserActive() throws Exception {
        Boolean newUserActive = status.user_active;
        int telStatus = telManager.getCallState();

        if(telStatus != TelephonyManager.CALL_STATE_IDLE) {
            newUserActive = true;
        }
        else {
            newUserActive = (screenOn && appPrefs.getSuspendWhenScreenOn() && !appPrefs.getStationaryDeviceMode());
        }

        if(status.user_active != newUserActive) {
            status.user_active = newUserActive;
            return true;
        }

        return false;
    }

    /**
     * Determines type of currently active network. Treats Ethernet as Wifi.
     *
     * @return true, if change since last run
     * @throws Exception if error occurs
     */
    private Boolean determineNetworkStatus() throws Exception {
        Boolean change = false;
        NetworkInfo activeNetwork = connManager.getActiveNetworkInfo();
        int networkType = -1;
        if(activeNetwork != null) {
            networkType = activeNetwork.getType();
        }
        if(networkType == ConnectivityManager.TYPE_WIFI || networkType == 9) { // 9 = ConnectivityManager.TYPE_ETHERNET
            //wifi or ethernet is online
            if(!status.wifi_online) {
                change = true; // if different from before, set flag
                if(Logging.ERROR) {
                    Log.d(Logging.TAG, "Unlmited internet connection - wifi or ethernet - found. type: " + networkType);
                }
            }
            status.wifi_online = true;
        }
        else {
            //wifi and ethernet are offline
            if(status.wifi_online) {
                change = true; // if different from before, set flag
            }
            status.wifi_online = false;
        }
        return change;
    }

    /**
     * Determines battery status of device
     *
     * @return true, if change since last run
     * @throws Exception if error occurs
     */
    private Boolean determineBatteryStatus() throws Exception {
        // check battery
        Boolean change = false;
        batteryStatus = ctx.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        if(batteryStatus != null) {
            stationaryDeviceSuspected =
                    !batteryStatus.getBooleanExtra(BatteryManager.EXTRA_PRESENT, true); // if no battery present, suspect stationary device
            if(appPrefs.getStationaryDeviceMode() && stationaryDeviceSuspected) {
                // API says no battery present (not reliable, e.g. Galaxy Nexus)
                // AND stationary device mode is enabled in preferences

                if(!stationaryDeviceMode) { // should not change during run-time. just triggered on initial read
                    change = true;
                    if(Logging.ERROR) {
                        Log.d(Logging.TAG, "No battery found and stationary device mode enabled in preferences -> skip battery status parsing");
                    }
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
                if(batteryPct != status.battery_charge_pct) {
                    status.battery_charge_pct = batteryPct;
                    change = true;
                }

                // temperature
                int temperature =
                        batteryStatus.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1) / 10; // always rounds down
                if(temperature < 0) {
                    throw new Exception("temperature parsing error");
                }
                if(temperature != status.battery_temperature_celsius) {
                    status.battery_temperature_celsius = temperature;
                    change = true;
                }

                // plugged in
                // treat all charging modes uniformly on client side,
                // adapt on_ac_power according to power source preferences defined in manager
                int plugged = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
                change = change | setAttributesForChargerType(plugged);
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
        status.on_ac_power = true;
        status.battery_temperature_celsius = 0;
        status.battery_charge_pct = 100;
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
    private Boolean setAttributesForChargerType(int chargerType) {
        Boolean change = false;
        Boolean enabled = false;

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
        }

        if(enabled) {
            if(!status.on_ac_power) {
                change = true; // if different from before, set flag
            }
            status.on_ac_power = true;
        }
        else {
            if(status.on_ac_power) {
                change = true;
            }
            status.on_ac_power = false;
        }

        return change;
    }
}
