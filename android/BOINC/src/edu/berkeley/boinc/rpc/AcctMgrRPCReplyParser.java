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
import edu.berkeley.boinc.utils.Logging;
import android.util.Log;
import android.util.Xml;

public class AcctMgrRPCReplyParser extends BaseParser{
	
	private AcctMgrRPCReply mAcctMgrRPCReply;

	public AcctMgrRPCReply getAccountMgrRPCReply() {
		return mAcctMgrRPCReply;
	}

	public static AcctMgrRPCReply parse(String rpcResult) {
		try {
			AcctMgrRPCReplyParser parser = new AcctMgrRPCReplyParser();
			Xml.parse(rpcResult.replace("<success/>", "<success>1</success>"), parser);
			return parser.getAccountMgrRPCReply();
		} catch (SAXException e) {
			if(Logging.WARNING) Log.w(Logging.TAG,"AcctMgrRPCReplyParser: malformated XML" + e.getMessage());
			return null;
		}
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("acct_mgr_rpc_reply")) {
			mAcctMgrRPCReply = new AcctMgrRPCReply();
		} else {
			mElementStarted = true;
			mCurrentElement.setLength(0);
		}
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);
		try {
			if (mAcctMgrRPCReply != null) {
				// inside <acct_mgr_rpc_reply>
				if (localName.equalsIgnoreCase("acct_mgr_rpc_reply")) {
					// closing tag
				}
				else {
					// decode inner tags
					if (localName.equalsIgnoreCase("error_num")) {
						mAcctMgrRPCReply.error_num = Integer.parseInt(mCurrentElement.toString());
					} else if (localName.equalsIgnoreCase("message")) {
						mAcctMgrRPCReply.messages.add(mCurrentElement.toString());
					}
				}
			}
		} catch (Exception e) {
			if(Logging.WARNING) Log.d(Logging.TAG, "AcctMgrRPCReplyParser Exception: " + e.getMessage());
		}
		mElementStarted = false;
	}

}
