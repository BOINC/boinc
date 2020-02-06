/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

public class AppPreferences {

    private final String PREFS = "PREFS";
    private SharedPreferences prefs;

    private Boolean autostart;
    private Boolean showNotificationForNotices;
    private Boolean showAdvanced;
    private Integer logLevel;
    private Boolean powerSourceAc;
    private Boolean powerSourceUsb;
    private Boolean powerSourceWireless;
    private Boolean stationaryDeviceMode; // disable battery status parsing
    private Boolean suspendWhenScreenOn;

    public void readPrefs(Context ctx) {
        if(prefs == null) {
            prefs = ctx.getSharedPreferences(PREFS, 0);
        }
        //second parameter of reading function is the initial value after installation.
        autostart = prefs.getBoolean("autostart", ctx.getResources().getBoolean(R.bool.prefs_default_autostart));
        showNotificationForNotices =
                prefs.getBoolean("showNotification", ctx.getResources().getBoolean(R.bool.prefs_default_notification_notices));
        showAdvanced = prefs.getBoolean("showAdvanced", ctx.getResources().getBoolean(R.bool.prefs_default_advanced));
        logLevel = prefs.getInt("logLevel", ctx.getResources().getInteger(R.integer.prefs_default_loglevel));
        Logging.setLogLevel(logLevel);
        powerSourceAc = prefs.getBoolean("powerSourceAc", ctx.getResources().getBoolean(R.bool.prefs_power_source_ac));
        powerSourceUsb =
                prefs.getBoolean("powerSourceUsb", ctx.getResources().getBoolean(R.bool.prefs_power_source_usb));
        powerSourceWireless =
                prefs.getBoolean("powerSourceWireless", ctx.getResources().getBoolean(R.bool.prefs_power_source_wireless));
        stationaryDeviceMode =
                prefs.getBoolean("stationaryDeviceMode", ctx.getResources().getBoolean(R.bool.prefs_stationary_device_mode));
        suspendWhenScreenOn =
                prefs.getBoolean("suspendWhenScreenOn", ctx.getResources().getBoolean(R.bool.prefs_suspend_when_screen_on));

        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "appPrefs read successful." + autostart + showNotificationForNotices + showAdvanced + logLevel +
                  powerSourceAc + powerSourceUsb + powerSourceWireless);
        }
    }

    public void setAutostart(Boolean as) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("autostart", as);
        editor.apply();
        this.autostart = as;
    }

    public Boolean getAutostart() {
        return this.autostart;
    }

    public void setShowNotificationForNotices(Boolean as) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("showNotification", as);
        editor.apply();
        this.showNotificationForNotices = as;
    }

    public Boolean getShowNotificationForNotices() {
        return this.showNotificationForNotices;
    }

    public void setShowAdvanced(Boolean as) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("showAdvanced", as);
        editor.apply();
        this.showAdvanced = as;
    }

    public Boolean getShowAdvanced() {
        return this.showAdvanced;
    }

    public void setLogLevel(Integer logLevel) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt("logLevel", logLevel);
        editor.apply();
        this.logLevel = logLevel;
        Logging.setLogLevel(logLevel);
    }

    public Integer getLogLevel() {
        return this.logLevel;
    }

    public void setPowerSourceAc(Boolean ac) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("powerSourceAc", ac);
        editor.apply();
        this.powerSourceAc = ac;
    }

    public void setPowerSourceUsb(Boolean usb) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("powerSourceUsb", usb);
        editor.apply();
        this.powerSourceUsb = usb;
    }

    public void setPowerSourceWireless(Boolean wireless) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("powerSourceWireless", wireless);
        editor.apply();
        this.powerSourceWireless = wireless;
    }

    public Boolean getPowerSourceAc() {
        return this.powerSourceAc;
    }

    public Boolean getPowerSourceUsb() {
        return this.powerSourceUsb;
    }

    public Boolean getPowerSourceWireless() {
        return this.powerSourceWireless;
    }

    public void setStationaryDeviceMode(Boolean sdm) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("stationaryDeviceMode", sdm);
        editor.apply();
        this.stationaryDeviceMode = sdm;
    }

    public Boolean getStationaryDeviceMode() {
        return this.stationaryDeviceMode;
    }

    public void setSuspendWhenScreenOn(Boolean swso) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("suspendWhenScreenOn", swso);
        editor.apply();
        this.suspendWhenScreenOn = swso;
    }

    public Boolean getSuspendWhenScreenOn() {
        return this.suspendWhenScreenOn;
    }
}
