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


public class AppsParser extends BaseParser {
	private static final String TAG = "AppsParser";

	private ArrayList<App> mApps = new ArrayList<App>();
	private App mApp = null;


	public final ArrayList<App> getApps() {
		return mApps;
	}

	/**
	 * Parse the RPC result (app) and generate vector of app
	 * @param rpcResult String returned by RPC call of core client
	 * @return vector of app
	 */
	public static ArrayList<App> parse(String rpcResult) {
		try {
			AppsParser parser = new AppsParser();
			Xml.parse(rpcResult, parser);
			return parser.getApps();
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
		if (localName.equalsIgnoreCase("app")) {
			mApp = new App();
		}
		else {
			// Another element, hopefully primitive and not constructor
			// (although unknown constructor does not hurt, because there will be primitive start anyway)
			mElementStarted = true;
			mCurrentElement.setLength(0);
		}
	}

	// Method characters(char[] ch, int start, int length) is implemented by BaseParser,
	// filling mCurrentElement (including stripping of leading whitespaces)
	//@Override
	//public void characters(char[] ch, int start, int length) throws SAXException { }

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);
		if (mApp != null) {
			// We are inside <app>
			if (localName.equalsIgnoreCase("app")) {
				// Closing tag of <app> - add to vector and be ready for next one
				if (!mApp.name.equals("")) {
					// name is a must
					mApps.add(mApp);
				}
				mApp = null;
			}
			else {
				// Not the closing tag - we decode possible inner tags
				trimEnd();
				if (localName.equalsIgnoreCase("name")) {
					mApp.name = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("user_friendly_name")) {
					mApp.user_friendly_name = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("non_cpu_intensive")) {
					mApp.non_cpu_intensive = Integer.parseInt(mCurrentElement.toString());
				}
			}
		}
		mElementStarted = false;
	}
}
