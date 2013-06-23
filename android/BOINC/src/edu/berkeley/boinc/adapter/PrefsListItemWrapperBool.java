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

import android.content.Context;

public class PrefsListItemWrapperBool extends PrefsListItemWrapper {
	
	//private final String TAG = "PrefsListItemWrapperBool";

	private Boolean status;
	
	public PrefsListItemWrapperBool(Context ctx, Integer ID, Integer categoryID, Boolean status) {
		super(ctx, ID, categoryID);
		this.status = status;
	}
	
	public void setStatus(Boolean newStatus) {
		this.status = newStatus;
	}
	
	public Boolean getStatus() {
		return this.status;
	}
}
