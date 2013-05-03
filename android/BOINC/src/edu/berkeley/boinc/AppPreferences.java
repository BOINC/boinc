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
package edu.berkeley.boinc;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

public class AppPreferences {
	
	private final String PREFS = "PREFS";
	private final String TAG = "AppPreferences";
	private SharedPreferences prefs;
	
	private Boolean autostart;
	private Boolean showNotification;
	private Boolean showAdvanced;
	
	public void readPrefs (Context ctx) {
		if(prefs == null) {
			prefs = ctx.getSharedPreferences(PREFS, 0);
		}
		//second parameter of reading function is the initial value after installation.
		autostart = prefs.getBoolean("autostart", false);
		showNotification = prefs.getBoolean("showNotification", true);
		showAdvanced = prefs.getBoolean("showAdvanced", false);
		
		Log.d(TAG, "appPrefs read successful." + autostart + showAdvanced);
	}
	
	public void setAutostart(Boolean as) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putBoolean("autostart", as);
		editor.commit();
		this.autostart = as;
	}
	
	public Boolean getAutostart () {
		return this.autostart;
	}

	public void setShowNotification(Boolean as) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putBoolean("showNotification", as);
		editor.commit();
		this.showNotification = as;
	}

	public Boolean getShowNotification() {
		return this.showNotification;
	}
	
	public void setShowAdvanced(Boolean as) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putBoolean("showAdvanced", as);
		editor.commit();
		this.showAdvanced = as;
	}
	
	public Boolean getShowAdvanced () {
		return this.showAdvanced;
	}
}
