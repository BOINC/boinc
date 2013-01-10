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

public class PrefsListItemWrapperText extends PrefsListItemWrapper {
	
	private final String TAG = "PrefsListItemWrapperText";

	public String header = "";
	public String status;
	public String display;
	
	public PrefsListItemWrapperText(Context ctx, Integer ID, String status) {
		super(ctx, ID);
		this.status = status;
		mapStrings(ID);
	}
	
	private void mapStrings(Integer id) {
		switch (id) {
		case R.string.prefs_project_email_header:
			header = ctx.getString(R.string.prefs_project_email_header);
			display = status;
			break;
		case R.string.prefs_project_pwd_header:
			header = ctx.getString(R.string.prefs_project_pwd_header);
			display = "*********"; 
			break;
		default:
			Log.d(TAG, "map failed!");
		}
	}
}
