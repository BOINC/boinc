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
package edu.berkeley.boinc.adapter;

import edu.berkeley.boinc.R;
import android.content.Context;
import android.util.Log;

public class PrefsListItemWrapperBool extends PrefsListItemWrapper {
	
	private final String TAG = "PrefsListItemWrapperBool";

	public String header = "";
	private String status_true = "";
	private String status_false = "";
	public String status_text;
	private Boolean status;
	
	public PrefsListItemWrapperBool(Context ctx, Integer ID, Integer categoryID, Boolean status) {
		super(ctx, ID, categoryID);
		this.status = status;
		mapStrings(ID);
		setStatusMessage();
	}
	
	private void mapStrings(Integer id) {
		switch (id) {
		case R.string.prefs_autostart_header:
			header = ctx.getString(R.string.prefs_autostart_header);
			status_true = ctx.getString(R.string.prefs_autostart_true);
			status_false = ctx.getString(R.string.prefs_autostart_false);
			break;
		case R.string.prefs_show_notification_header:
			header = ctx.getString(R.string.prefs_show_notification_header);
			status_true = ctx.getString(R.string.prefs_show_notification_true);
			status_false = ctx.getString(R.string.prefs_show_notification_false);
			break;
		case R.string.prefs_run_on_battery_header:
			header = ctx.getString(R.string.prefs_run_on_battery_header);
			status_true = ctx.getString(R.string.prefs_run_on_battery_true);
			status_false = ctx.getString(R.string.prefs_run_on_battery_false);
			break;
		case R.string.prefs_network_wifi_only_header:
			header = ctx.getString(R.string.prefs_network_wifi_only_header);
			status_true = ctx.getString(R.string.prefs_network_wifi_only_true);
			status_false = ctx.getString(R.string.prefs_network_wifi_only_false);
			break;
		case R.string.prefs_show_advanced_header:
			header = ctx.getString(R.string.prefs_show_advanced_header);
			// no status Strings
			break;
		default:
			Log.d(TAG, "map failed!");
		}
	}
	
	public void setStatus(Boolean newStatus) {
		this.status = newStatus;
		setStatusMessage();
	}
	
	public Boolean getStatus() {
		return this.status;
	}
	
	private void setStatusMessage() {
		if(status) {
			status_text = status_true;
		} else {
			status_text = status_false;
		}
		
	}
}
