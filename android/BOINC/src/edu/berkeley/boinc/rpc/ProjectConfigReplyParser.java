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

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import edu.berkeley.boinc.debug.Logging;
import android.util.Log;
import android.util.Xml;

public class ProjectConfigReplyParser extends BaseParser {
	private static final String TAG = "ProjectConfigParser";

	private ProjectConfig mProjectConfig = new ProjectConfig();
	
	private ArrayList<PlatformInfo> mPlatforms;
	private Boolean withinPlatforms = false;
	private String platformName = "";
	private String platformFriendlyName = "";
	private String platformPlanClass = "";
	
	
	public final ProjectConfig getProjectConfig() {
		return mProjectConfig;
	}

	/**
	 * Parse the RPC result (state) and generate vector of projects info
	 * @param rpcResult String returned by RPC call of core client
	 * @return connected client state
	 */
	public static ProjectConfig parse(String rpcResult) {
		try {
			ProjectConfigReplyParser parser = new ProjectConfigReplyParser();
			//TODO report malformated XML to BOINC and remove String.replace here...
			//<boinc_gui_rpc_reply>
			//<?xml version="1.0" encoding="ISO-8859-1" ?>
			//<project_config>
			Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser);
			return parser.getProjectConfig();
		}
		catch (SAXException e) {
			if (Logging.DEBUG) Log.d(TAG, "Malformed XML:\n" + rpcResult);
			else if (Logging.INFO) Log.i(TAG, "Malformed XML");
			return null;
		}		

	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("project_config")) {
			mProjectConfig = new ProjectConfig();
		} else if (localName.equalsIgnoreCase("platforms")) {
			withinPlatforms = true;
			mPlatforms = new ArrayList<PlatformInfo>(); //initialize new list (flushing old elements)
		}
		else {
			// Another element, hopefully primitive and not constructor
			// (although unknown constructor does not hurt, because there will be primitive start anyway)
			mElementStarted = true;
			mCurrentElement.setLength(0);
		}
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);
		try {
			if (mProjectConfig != null) {
				if (localName.equalsIgnoreCase("platforms")) {// closing tag of platform names
					mProjectConfig.platforms = mPlatforms;
					withinPlatforms = false;
				} 
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase("name")) {
						mProjectConfig.name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("master_url")) {
						mProjectConfig.masterUrl = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("local_revision")) {
						mProjectConfig.localRevision = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("min_passwd_length")) {
						mProjectConfig.minPwdLength = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("user_name")) {
						mProjectConfig.userName = true;
					}
					else if (localName.equalsIgnoreCase("uses_username")) {
						mProjectConfig.userName = true;
					}
					else if (localName.equalsIgnoreCase("web_stopped")) {
						mProjectConfig.webStopped = false; //default in case parsing fails
						if(Integer.parseInt(mCurrentElement.toString()) != 0) mProjectConfig.webStopped = true;
					}
					else if (localName.equalsIgnoreCase("sched_stopped")) {
						mProjectConfig.schedulerStopped = false; //default in case parsing fails
						if(Integer.parseInt(mCurrentElement.toString()) != 0) mProjectConfig.schedulerStopped = true;
					}
					else if (localName.equalsIgnoreCase("client_account_creation_disabled")) {
						mProjectConfig.clientAccountCreationDisabled = true;
					}
					else if (localName.equalsIgnoreCase("min_client_version")) {
						mProjectConfig.minClientVersion = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("rpc_prefix")) {
						mProjectConfig.rpcPrefix = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("platform_name") && withinPlatforms) { 
						platformName = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("user_friendly_name") && withinPlatforms) { 
						platformFriendlyName = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("plan_class") && withinPlatforms) { 
						platformPlanClass = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("terms_of_use")) {
						mProjectConfig.termsOfUse = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("platform") && withinPlatforms) { // finish platform object and add to array
						mPlatforms.add(new PlatformInfo(platformName, platformFriendlyName, platformPlanClass));
						platformFriendlyName = "";
						platformName = "";
						platformPlanClass = "";
					}
					else if (localName.equalsIgnoreCase("platform") && withinPlatforms) { // finish platforms
						withinPlatforms = false;
						mProjectConfig.platforms = mPlatforms;
					}
					else if (localName.equalsIgnoreCase("error_num")) { // reply is not present yet
						mProjectConfig.error_num = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("client_account_creation_disabled")) {
						mProjectConfig.clientAccountCreationDisabled = true;
					}
					else if (localName.equalsIgnoreCase("account_manager")) { 
						mProjectConfig.accountManager = true;
					}
				}
			}
			mElementStarted = false;
		}
		catch (NumberFormatException e) {
			if (Logging.INFO) Log.i(TAG, "Exception when decoding " + localName);
		}
	}
}
