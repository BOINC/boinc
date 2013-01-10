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

/*
 * IRemoteService defines an AIDL interface.
 * It allows BOINC Managers to call methods on the Android BOINC Client service.
 * The provided methods are in addition to usual web-RPC mechanism used for both components to communicate.
 * Therefore, they are specific for Android implementations of BOINC.
 */

package edu.berkeley.boinc;

import java.util.List;

interface IClientService {
	
	/* Register project URLs in Client's Android application and receive auth. token.
	 * This project URL is required to e.g. properly detach a project upon application uninstall.
	 * 
	 * URLs: ArrayList of all URLs that project intents to attach
	 * packageName: Android package name of project application
	 * 
	 * returns web-RPC authentication token, which is required to establish socket communication to BOINC Client. Can be null, if problem occurs!*/
	String getAuthToken(in List<String> URLs, in String packageName);
}
