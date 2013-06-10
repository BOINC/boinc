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

import edu.berkeley.boinc.utils.*;

import edu.berkeley.boinc.R;
import android.content.Context;
import android.util.Log;

public class PrefsListItemWrapperValue extends PrefsListItemWrapper {

	public String header = "";
	public String description = "";
	public String unit = "";
	public Double status;
	public Boolean isPct = false; // shows whether value is percentage, therefore using a SeekBar
	public Boolean isNumber = false; // shows whether value is a number (e.g. cpu cores) to adjust seekbar
	
	public PrefsListItemWrapperValue(Context ctx, Integer ID, Integer categoryID, Double status) {
		super(ctx, ID, categoryID);
		this.status = status;
		mapStrings(ID);
	}
	
	private void mapStrings(Integer id) {
		switch (id) {
		case R.string.battery_charge_min_pct_header:
			header = ctx.getString(R.string.battery_charge_min_pct_header);
			description = ctx.getString(R.string.battery_charge_min_pct_description);
			unit = ctx.getString(R.string.prefs_unit_pct);
			break;
		case R.string.prefs_disk_max_pct_header:
			header = ctx.getString(R.string.prefs_disk_max_pct_header);
			description = ctx.getString(R.string.prefs_disk_max_pct_description);
			unit = ctx.getString(R.string.prefs_unit_pct);
			break;
		case R.string.prefs_disk_min_free_gb_header:
			header = ctx.getString(R.string.prefs_disk_min_free_gb_header);
			description = ctx.getString(R.string.prefs_disk_min_free_gb_description);
			unit = ctx.getString(R.string.prefs_unit_gb);
			break;
		case R.string.prefs_network_daily_xfer_limit_mb_header:
			header = ctx.getString(R.string.prefs_network_daily_xfer_limit_mb_header);
			description = ctx.getString(R.string.prefs_network_daily_xfer_limit_mb_description);
			unit = ctx.getString(R.string.prefs_unit_mb);
			break;
		case R.string.prefs_cpu_number_cpus_header:
			header = ctx.getString(R.string.prefs_cpu_number_cpus_header);
			description = ctx.getString(R.string.prefs_cpu_number_cpus_description);
			isNumber = true;
			break;
		case R.string.prefs_cpu_other_load_suspension_header:
			header = ctx.getString(R.string.prefs_cpu_other_load_suspension_header);
			description = ctx.getString(R.string.prefs_cpu_other_load_suspension_description);
			unit = ctx.getString(R.string.prefs_unit_pct);
			break;
		case R.string.prefs_cpu_time_max_header:
			header = ctx.getString(R.string.prefs_cpu_time_max_header);
			description = ctx.getString(R.string.prefs_cpu_time_max_description);
			unit = ctx.getString(R.string.prefs_unit_pct);
			break;
		case R.string.prefs_memory_max_idle_header:
			header = ctx.getString(R.string.prefs_memory_max_idle_header);
			description = ctx.getString(R.string.prefs_memory_max_idle_description);
			unit = ctx.getString(R.string.prefs_unit_pct);
			break;
		default:
			if(Logging.DEBUG) Log.d(Logging.TAG, "PrefsListItemWrapperValue map failed!");
		}
		if(unit.equals(ctx.getString(R.string.prefs_unit_pct))) isPct = true;
	}
}
