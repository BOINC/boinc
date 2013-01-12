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
	
	//TODO needs to be restructured for multi-project support
	
	private final String PREFS = "PREFS";
	private final String TAG = "AppPreferences";
	private SharedPreferences prefs;
	
	private String email;
	private String pwd;
	private String md5; // holds projects authentication token, as looked up during login
	private Boolean autostart;
	
	public void readPrefs (Context ctx) {
		if(prefs == null) {
			prefs = ctx.getSharedPreferences(PREFS, 0);
		}
		//second parameter of reading function is the initial value after installation of AndroidBOINC on new device.
		autostart = prefs.getBoolean("autostart", false);
		email = prefs.getString("email", "");
		pwd = prefs.getString("pwd", "");
		md5 = prefs.getString("md5", "");
		
		Log.d(TAG, "appPrefs read successful." + autostart + email + pwd + md5);
	}
	
	public void setEmail(String email) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putString("email", email);
		editor.commit();
		this.email = email;
	}
	
	public void setPwd(String pwd) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putString("pwd", pwd);
		editor.commit();
		this.pwd = pwd;
	}
	
	public void setMd5(String md5) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putString("md5", md5);
		editor.commit();
		this.md5 = md5;
	}
	
	public void setAutostart(Boolean as) {
		SharedPreferences.Editor editor = prefs.edit();
		editor.putBoolean("autostart", as);
		editor.commit();
		this.autostart = as;
	}
	
	public String getEmail () {
		return this.email;
	}
	
	public String getPwd () {
		return this.pwd;
	}
	
	public String getMd5 () {
		return this.md5;
	}
	
	public Boolean getAutostart () {
		return this.autostart;
	}
}
