/*
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
 */

package edu.berkeley.boinc.rpc;

import android.util.Log;
import android.util.Xml;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import java.util.ArrayList;

import edu.berkeley.boinc.utils.Logging;

public class AccountManagerParser extends BaseParser{

	private ArrayList<AccountManager> mAcctMgrInfos = new ArrayList<>();
	private AccountManager mAcctMgrInfo = null;

	public ArrayList<AccountManager> getAccountManagerInfo() {
		return mAcctMgrInfos;
	}

	public static ArrayList<AccountManager> parse(String rpcResult) {
		try {
			AccountManagerParser parser = new AccountManagerParser();
			Xml.parse(rpcResult, parser);
			return parser.getAccountManagerInfo();
		} catch (SAXException e) {
			return null;
		}
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("account_manager")) {
			mAcctMgrInfo = new AccountManager();
		} else {
			mElementStarted = true;
			mCurrentElement.setLength(0);
		}
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);
		try {
			if (mAcctMgrInfo != null) {
				// inside <acct_mgr_info>
				if (localName.equalsIgnoreCase("account_manager")) {
					// Closing tag of <account_manager> - add to vector and be ready for next one
					if (!mAcctMgrInfo.name.equals("")) {
						// name is a must
						mAcctMgrInfos.add(mAcctMgrInfo);
					}
					mAcctMgrInfo = null;
				}
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase("name")) { //project name
						mAcctMgrInfo.name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("url")) {
						mAcctMgrInfo.url = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("description")) {
						mAcctMgrInfo.description = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase("image")) {
						mAcctMgrInfo.imageUrl = mCurrentElement.toString();
					}
				}
			}
		} catch (Exception e) {
			if(Logging.ERROR) Log.e(Logging.TAG,"AccountManagerParser.endElement error: ",e);
		}
		mElementStarted = false;
	}
}
