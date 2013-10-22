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

public class App {
	public String name = "";
	public String user_friendly_name = "";
	public int non_cpu_intensive = 0;
	public Project project;
	
	public boolean compare(App myapp) {
		//Check if name is the same
		if(!this.name.equalsIgnoreCase(myapp.name)) {
			return false;
		}
		return true;
	}

	public final String getName() {
		return user_friendly_name.equals("") ? name : user_friendly_name;
	}
}
