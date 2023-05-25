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
 * IClientRemoteService AIDL interface.
 * This AIDL file provides an interface to manage projects in an organized manner.
 * In order to use this interface, bind to edu.berkeley.boinc.client.ClientRemoteService.
 */

package edu.berkeley.boinc.client;

import java.util.List;
import edu.berkeley.boinc.rpc.AccountOut;

interface IClientRemoteService {

	/* Checks whether interface recipient is ready to serve commands.
	 * returns success*/
	boolean isReady();

	/* Returns the version code specified in AndroidManifest.xml.
	 * can be used to detect new AIDL versions.
	 * returns version code as Integer*/
	 int getVersionCode();

//== project management ==
	/* Attach project to BOINC application.
	 * packageName: package name of Android application causing this attach
	 * url: project URL
	 * id: project login identifier, i.e. email address
	 * pwd: project login password in plain text. Password gets encrypted by BOINC before transmitted over the internet.
	 * returns success*/
	boolean attachProject(in String packageName, in String url, in String id, in String authenticator);

	/* Check whether given project URL is attached to BOINC
	 * url: project URL
	 * returns success*/
	boolean checkProjectAttached(in String url);

	/* Verifies given credentials to project account
	 * url: project URL
	 * id: project login identifier, i.e. email address
	 * pwd: project login password in plain text. Password gets encrypted by BOINC before transmitted over the internet.
	 *
	 * returns AccountOut object including status code (0:verified, -206:pwd incorrect, -136:id unknown,
	 * -113:internet connection error) status message and authenticator required by attachProject*/
	AccountOut verifyCredentials(in String url, in String id, in String pwd);

	/* Detaches given project from BOINC application.
	 * packageName: package name of Android application causing this detach
	 * url: project URL
	 * returns success*/
	boolean detachProject(in String packageName, in String url);

	/* Creates new account at specified project URL
	 * url: project URL
	 * email: email address
	 * userName: user name, optional
	 * pwd: project login password in plain text. Password gets encrypted by BOINC before transmitted over the internet.
	 * teamName: team name, optional
	 *
	 * returns AccountOut object including status code, status message and authenticator required by attachProject*/
	AccountOut createAccount(in String url, in String email, in String userName, in String pwd, in String teamName);

//== authentication ==
	/* Retrieval the web-RPC authentication token, required to open a socket connection to the BOINC Client directly.
	 * please make sure, you attached all required projects using this interface!
	 * returns token, null if error*/
	String getRpcAuthToken();
}

