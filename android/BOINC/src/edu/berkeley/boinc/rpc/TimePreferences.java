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

import java.util.Locale;

public class TimePreferences {
	public double start_hour, end_hour;
	
	private static final String hourToString(double value) {
		int hour = (int)Math.floor(value);
		int minute = (int)Math.round((value-(double)hour)*60.0);
		minute = Math.min(59, minute);
		return String.format(Locale.US, "%02d:%02d",hour, minute);
	}
	
	public static final class TimeSpan {
		public double start_hour;
		public double end_hour;
		
		public String startHourString() {
			return hourToString(start_hour);
		}
		
		public String endHourString() {
			return hourToString(end_hour);
		}
	};
	
	public TimeSpan[] week_prefs  = new TimeSpan[7];
	
	public String startHourString() {
		return hourToString(start_hour);
	}
	
	public String endHourString() {
		return hourToString(end_hour);
	}
}
