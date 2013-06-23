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

public class PrefsListItemWrapper {
	
	public Context ctx;
	public Integer ID;
	public Integer categoryID;
	public Boolean isCategory;
	public String header;
	
	// Constructor for elements
	public PrefsListItemWrapper (Context ctx, Integer ID, Integer categoryID) {
		this.ctx = ctx;
		this.ID = ID;
		this.header = ctx.getString(ID);
		this.categoryID = categoryID;
		this.isCategory = false;
	}
	
	// Constructor for categories
	public PrefsListItemWrapper (Context ctx, Integer ID, Boolean isCategory) {
		this.ctx = ctx;
		this.ID = ID;
		this.isCategory = isCategory;
	}
}
