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


public class HostInfoParser extends BaseParser {
	private HostInfo mHostInfo = null;


	public final HostInfo getHostInfo() {
		return mHostInfo;
	}

	/**
	 * Parse the RPC result (host_info) and generate vector of projects info
	 * @param rpcResult String returned by RPC call of core client
	 * @return HostInfo
	 */
	public static HostInfo parse(String rpcResult) {
		try {
			HostInfoParser parser = new HostInfoParser();
			Xml.parse(rpcResult, parser);
			return parser.getHostInfo();
		}
		catch (SAXException e) {
			return null;
		}		
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("host_info")) {
			mHostInfo = new HostInfo();
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
			if (mHostInfo != null) {
				// we are inside <host_info>
				if (localName.equalsIgnoreCase("host_info")) {
					// Closing tag of <host_info> - nothing to do at the moment
				}
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase("timezone")) {
						mHostInfo.timezone = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("domain_name")) {
						mHostInfo.domain_name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("ip_addr")) {
						mHostInfo.ip_addr = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("host_cpid")) {
						mHostInfo.host_cpid = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("p_ncpus")) {
						mHostInfo.p_ncpus = Integer.parseInt(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("p_vendor")) {
						mHostInfo.p_vendor = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("p_model")) {
						mHostInfo.p_model = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("p_features")) {
						mHostInfo.p_features = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("p_fpops")) {
						mHostInfo.p_fpops = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("p_iops")) {
						mHostInfo.p_iops = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("p_membw")) {
						mHostInfo.p_membw = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("p_calculated")) {
						mHostInfo.p_calculated = (long)Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("product_name")) {
						mHostInfo.product_name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("m_nbytes")) {
						mHostInfo.m_nbytes = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("m_cache")) {
						mHostInfo.m_cache = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("m_swap")) {
						mHostInfo.m_swap = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("d_total")) {
						mHostInfo.d_total = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("d_free")) {
						mHostInfo.d_free = Double.parseDouble(mCurrentElement.toString());
					}
					else if (localName.equalsIgnoreCase("os_name")) {
						mHostInfo.os_name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("os_version")) {
						mHostInfo.os_version = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("virtualbox_version")) {
						mHostInfo.virtualbox_version = mCurrentElement.toString();
					}
				}
			}
		}
		catch (NumberFormatException e) {
		}
		mElementStarted = false;
	}
}
