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

import java.util.ArrayList;

// according to http://boinc.berkeley.edu/trac/wiki/WebRpc

public class ProjectConfig{
	public Integer error_num = 0; // if results are not present yet. (polling)
	public String name = "";
	public String masterUrl = "";
	public String localRevision = ""; // e.g. 4.3.2 can't be parse as int or float.
	public Integer minPwdLength = 0;
	public Boolean userName = false;
	public Boolean webStopped = false;
	public Boolean schedulerStopped = false;
	public Boolean accountCreationDisabled = false; 
	public Boolean clientAccountCreationDisabled = false;
	public Boolean accountManager = false;
	public Integer minClientVersion = 0;
	public String rpcPrefix = "";
	public ArrayList<PlatformInfo> platforms = new ArrayList<PlatformInfo>();
	public String termsOfUse;
	
}
