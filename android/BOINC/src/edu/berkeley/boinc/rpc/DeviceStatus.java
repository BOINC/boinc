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

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.BatteryManager;
import android.util.Log;

public class DeviceStatus{
	
	private final String TAG = "Rpc.DeviceState";
	
	// variables describing device status
	private boolean on_ac_power;
	private boolean on_usb_power; //not used
	private int battery_charge_pct;
	private int battery_state; //not used
	private int battery_temperature_celcius;
	private boolean wifi_online;

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
	// returns true if data model has actually changed
	// returns false if device status is unchanged -> avoid RPC call
	public Boolean update() throws Exception {
		if(ctx == null) {
			Log.w(TAG,"context not set");
			return false;
		}
		
		Boolean change = false;
		
		// check battery
		batteryStatus = ctx.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		if(batteryStatus != null){
			
			// calculate charging level
			int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
			int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
			int batteryPct = (int) ((level / (float) scale) * 100); // always rounds down
			if(batteryPct <= 1 || batteryPct > 100) throw new Exception("battery level parsing error");
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
			int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
			if(status != BatteryManager.BATTERY_STATUS_DISCHARGING){
				// power supply online. not equivalent to BATTERY_STATUS_CHARGING which is not the case when full and plugged in
				if(!on_ac_power) change = true; // if different from before, set flag
				on_ac_power = true;
			} else {
				//power supply offline
				if(on_ac_power) change = true; // if different from before, set flag
				on_ac_power = false;
			}
		}
		
		// check wifi status
		if(connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isConnected()){
			//wifi is online
			if(!wifi_online) change = true; // if different from before, set flag
			wifi_online = true;
		} else {
			//wifi offline
			if(wifi_online) change = true; // if different from before, set flag
			wifi_online = false;
		}
		
		Log.d(TAG, "change: " + change + " - power supply: " + on_ac_power + " ; level: " + battery_charge_pct + " ; temperature: " + battery_temperature_celcius + " ; wifi: " + wifi_online);
		return change;
	}
	
	// getter
	public boolean isOn_ac_power() {
		return on_ac_power;
	}

	public boolean isOn_usb_power() {
		return on_usb_power;
	}

	public int getBattery_charge_pct() {
		return battery_charge_pct;
	}

	public int getBattery_state() {
		return battery_state;
	}

	public int getBattery_temperature_celcius() {
		return battery_temperature_celcius;
	}

	public boolean isWifi_online() {
		return wifi_online;
	}
}
