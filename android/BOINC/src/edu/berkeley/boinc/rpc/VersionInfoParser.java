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

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import android.util.Xml;

public class VersionInfoParser extends BaseParser {
	private VersionInfo mVersionInfo = null;

	public final VersionInfo getVersionInfo() {
		return mVersionInfo;
	}

	/**
	 * Parse the RPC result (host_info) and generate vector of projects info
	 * @param rpcResult String returned by RPC call of core client
	 * @return VersionInfo (of core client)
	 */
	public static VersionInfo parse(String rpcResult) {
		try {
			VersionInfoParser parser = new VersionInfoParser();
			Xml.parse(rpcResult, parser);
			return parser.getVersionInfo();
		}
		catch (SAXException e) {
			return null;
		}		
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("server_version")) {
			mVersionInfo = new VersionInfo();
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
		try {
			if (mVersionInfo != null) {
				// we are inside <server_version>
				if (localName.equalsIgnoreCase("server_version")) {
					// Closing tag of <server_version> - nothing to do at the moment
				}
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase("major")) {
						mVersionInfo.major = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("minor")) {
						mVersionInfo.minor = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("release")) {
						mVersionInfo.release = Integer.parseInt(mCurrentElement.toString());
					}
				}
			}
		}
		catch (NumberFormatException e) {
		}
		mElementStarted = false; // to be clean for next one
	}
}
