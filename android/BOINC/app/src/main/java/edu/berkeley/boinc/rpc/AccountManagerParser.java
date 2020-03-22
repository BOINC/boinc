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
import java.util.List;

import edu.berkeley.boinc.utils.Logging;

public class AccountManagerParser extends BaseParser {
    static final String ACCOUNT_MGR_TAG = "account_manager";
    static final String IMAGE_TAG = "image";

    private List<AccountManager> mAcctMgrInfos = new ArrayList<>();
	private AccountManager mAcctMgrInfo = null;

	List<AccountManager> getAccountManagerInfo() {
		return mAcctMgrInfos;
	}

	public static List<AccountManager> parse(String rpcResult) {
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
		if (localName.equalsIgnoreCase(ACCOUNT_MGR_TAG)) {
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
				if (localName.equalsIgnoreCase(ACCOUNT_MGR_TAG)) {
					// Closing tag of <account_manager> - add to vector and be ready for next one
					if (!mAcctMgrInfo.name.isEmpty()) {
						// name is a must
						mAcctMgrInfos.add(mAcctMgrInfo);
					}
					mAcctMgrInfo = null;
				}
				else {
					// Not the closing tag - we decode possible inner tags
					trimEnd();
					if (localName.equalsIgnoreCase(AccountManager.Fields.name)) { //project name
						mAcctMgrInfo.name = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase(AccountManager.Fields.url)) {
						mAcctMgrInfo.url = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase(AccountManager.Fields.description)) {
						mAcctMgrInfo.description = mCurrentElement.toString();
					}
					else if (localName.equalsIgnoreCase(IMAGE_TAG)) {
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
