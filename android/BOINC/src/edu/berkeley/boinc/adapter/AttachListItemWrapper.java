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

import edu.berkeley.boinc.rpc.ProjectInfo;

public class AttachListItemWrapper {
	
	public Boolean isCategory;
	public String categoryName;
	
	public Boolean isManual;
	
	public Boolean isProject;
	public ProjectInfo project;
	
	// Constructor for projects
	public AttachListItemWrapper (ProjectInfo project) {
		this.project = project;
		this.isProject = true;
		this.isCategory = false;
		this.isManual = false;
	}
	
	// Constructor for categories
	public AttachListItemWrapper (String categoryName) {
		this.categoryName = categoryName;
		this.isProject = false;
		this.isCategory = true;
		this.isManual = false;
	}
	
	// Constructor for manual field
	public AttachListItemWrapper () {
		this.isProject = false;
		this.isCategory = false;
		this.isManual = true;
	}
	
}
