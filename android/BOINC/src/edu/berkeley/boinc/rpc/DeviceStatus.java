/*******************************************************************************
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
 ******************************************************************************/
package edu.berkeley.boinc.rpc;

import edu.berkeley.boinc.utils.*;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.BatteryManager;
import android.util.Log;

public class DeviceStatus{
	
	// current data in structure valid?
	// is only true if "update()" finished, i.e. did not fire exception
	// if false, getter methods return negative values and cause client
	// to suspend computation and network
	public boolean valid = false;
	
	// variables describing device status
	private boolean on_ac_power = false;
	private boolean on_usb_power = false;
	private int battery_charge_pct = 0;
	private int battery_state = -1; //not used
	private int battery_temperature_celcius = 100;
	private boolean wifi_online = false;

	// android specifics
	private Context ctx;// context required for reading device status
	private ConnectivityManager connManager; // connManager contains current wifi status
	private Intent batteryStatus; // sticky intent, extras of Intent contain status, see BatteryManager.
	
	// constructor
	public DeviceStatus(Context ctx) {
		this.ctx = ctx;
		this.connManager = (ConnectivityManager) ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
		batteryStatus = ctx.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
	}
	
	// polls current device status
	public Boolean update() throws Exception {
		// invalid data
		valid = false;
		
		if(ctx == null) {
			if(Logging.WARNING) Log.w(Logging.TAG,"context not set");
			return false;
		}
		
		Boolean change = false;
		
		// check battery
		batteryStatus = ctx.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		if(batteryStatus != null){
			
			// calculate charging level
			int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
			int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
			if(level == -1 || scale == -1) throw new Exception("battery level parsing error");
			int batteryPct = (int) ((level / (float) scale) * 100); // always rounds down
			if(batteryPct < 1 || batteryPct > 100) throw new Exception("battery level parsing error");
			if(batteryPct != battery_charge_pct) {
				battery_charge_pct = batteryPct;
				change = true;
			}
			
			// temperature
			int temperature = batteryStatus.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1) / 10; // always rounds down
			if(temperature < 0) throw new Exception("temperature parsing error");
			if(temperature != battery_temperature_celcius) {
				battery_temperature_celcius = temperature;
				change = true;
			}
			
			// plugged in
			int plugged = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
			switch (plugged) {
				case BatteryManager.BATTERY_PLUGGED_AC:
					if(!on_ac_power) change = true; // if different from before, set flag
					on_ac_power = true;
					break;
				case BatteryManager.BATTERY_PLUGGED_USB:
					if(!on_usb_power) change = true; // if different from before, set flag
					on_usb_power = true;
					break;
				default: 
					if(on_usb_power || on_ac_power) change = true;
					on_usb_power = false;
					on_ac_power = false;
					break;
			}
		} else throw new Exception ("battery intent null");
		
		// check wifi status
		//if(connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isAvailable()){
		NetworkInfo activeNetwork = connManager.getActiveNetworkInfo();
		if(activeNetwork == null || activeNetwork.getType() != ConnectivityManager.TYPE_WIFI) {
			//wifi offline
			if(wifi_online) change = true; // if different from before, set flag
			wifi_online = false;
		} else {
			//wifi is online
			if(!wifi_online) change = true; // if different from before, set flag
			wifi_online = true;
		}
		
		if(change) if(Logging.DEBUG) Log.d(Logging.TAG, "change: " + change + " - ac: " + on_ac_power + " ; usb: " + on_usb_power + " ; level: " + battery_charge_pct + " ; temperature: " + battery_temperature_celcius + " ; wifi: " + wifi_online);
		if(Logging.VERBOSE) Log.v(Logging.TAG, "change: " + change + " - ac: " + on_ac_power + " ; usb: " + on_usb_power + " ; level: " + battery_charge_pct + " ; temperature: " + battery_temperature_celcius + " ; wifi: " + wifi_online);
		
		valid = true; // end reached without exception
		return change;
	}
	
	// getter
	// if data not valid, i.e. has not finished update routine
	// getter shall return values that cause client so suspend
	public boolean isOn_ac_power() {
		if(!valid) return false;
		return on_ac_power;
	}

	public boolean isOn_usb_power() {
		if(!valid) return false;
		return on_usb_power;
	}

	public int getBattery_charge_pct() {
		if(!valid) return 0;
		return battery_charge_pct;
	}

	public int getBattery_state() {
		if(!valid) return -1;
		return battery_state;
	}

	public int getBattery_temperature_celcius() {
		if(!valid) return 100;
		return battery_temperature_celcius;
	}

	public boolean isWifi_online() {
		if(!valid) return false;
		return wifi_online;
	}
}
