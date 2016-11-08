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

public class WorkunitsParser extends BaseParser {

	private ArrayList<Workunit> mWorkunits = new ArrayList<Workunit>();
	private Workunit mWorkunit = null;


	public final ArrayList<Workunit> getWorkunits() {
		return mWorkunits;
	}

	/**
	 * Parse the RPC result (workunit) and generate corresponding vector
	 * @param rpcResult String returned by RPC call of core client
	 * @return vector of workunits
	 */
	public static ArrayList<Workunit> parse(String rpcResult) {
		try {
			WorkunitsParser parser = new WorkunitsParser();
			Xml.parse(rpcResult, parser);
			return parser.getWorkunits();
		}
		catch (SAXException e) {
			return null;
		}
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("workunit")) {
			mWorkunit = new Workunit();
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
			if (mWorkunit != null) {
				// We are inside <workunit>
				if (localName.equalsIgnoreCase("workunit")) {
					// Closing tag of <workunit> - add to vector and be ready for next one
					if (!mWorkunit.name.equals("")) {
						// name is a must
						mWorkunits.add(mWorkunit);
					}
					mWorkunit = null;
				}
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase("name")) {
						mWorkunit.name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("app_name")) {
						mWorkunit.app_name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("version_num")) {
						mWorkunit.version_num = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("rsc_fpops_est")) {
						mWorkunit.rsc_fpops_est = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("rsc_fpops_bound")) {
						mWorkunit.rsc_fpops_bound = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("rsc_memory_bound")) {
						mWorkunit.rsc_memory_bound = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("rsc_disk_bound")) {
						mWorkunit.rsc_disk_bound = Double.parseDouble(mCurrentElement.toString());
					}
				}
			}
		}
		catch (NumberFormatException e) {
		}
		mElementStarted = false;
	}
}
