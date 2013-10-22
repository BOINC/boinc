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
import android.util.Xml;


public class ProjectInfoParser extends BaseParser {

	private ArrayList<ProjectInfo> mProjectInfos = new ArrayList<ProjectInfo>();
	private ProjectInfo mProjectInfo = null;
	private ArrayList<String> mPlatforms;
	Boolean withinPlatforms = false;


	public final ArrayList<ProjectInfo> getProjectInfos() {
		return mProjectInfos;
	}

	public static ArrayList<ProjectInfo> parse(String rpcResult) {
		try {
			ProjectInfoParser parser = new ProjectInfoParser();
			// report malformated XML to BOINC and remove String.replace here...
			Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser);
			return parser.getProjectInfos();
		}
		catch (SAXException e) {
			return null;
		}
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("project")) {
			mProjectInfo = new ProjectInfo();
		} else if (localName.equalsIgnoreCase("platforms")) {
			mPlatforms = new ArrayList<String>(); //initialize new list (flushing old elements)
			withinPlatforms = true;
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
		if (mProjectInfo != null) {
			if (localName.equalsIgnoreCase("project")) {
				// Closing tag of <project> - add to vector and be ready for next one
				if (!mProjectInfo.name.equals("")) {
					// name is a must
					mProjectInfos.add(mProjectInfo);
				}
				mProjectInfo = null;
			} else if(localName.equalsIgnoreCase("platforms")) { // closing tag of platform names
				mProjectInfo.platforms = mPlatforms;
				withinPlatforms = false;
			}
			else {
				// Not the closing tag - we decode possible inner tags
				trimEnd();
				if (localName.equalsIgnoreCase("name") && !withinPlatforms) { //project name
					mProjectInfo.name = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("url")) {
					mProjectInfo.url = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("general_area")) {
					mProjectInfo.generalArea = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("specific_area")) {
					mProjectInfo.specificArea = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("description")) {
					mProjectInfo.description = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("home")) {
					mProjectInfo.home = mCurrentElement.toString();
				}
				else if (localName.equalsIgnoreCase("name") && withinPlatforms) { //platform name
					mPlatforms.add(mCurrentElement.toString());
				}
				else if (localName.equalsIgnoreCase("image")) {
					mProjectInfo.imageUrl = mCurrentElement.toString();
				}
			}
		}
		mElementStarted = false;
	}
}
