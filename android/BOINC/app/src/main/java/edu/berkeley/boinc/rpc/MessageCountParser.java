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
import org.xml.sax.helpers.DefaultHandler;
import android.util.Xml;

public class MessageCountParser extends DefaultHandler {
	private boolean mParsed = false;
	private boolean mInReply = false;
	private int mSeqno = -1;
	private StringBuilder mCurrentElement = new StringBuilder();

	// Disable direct instantiation of this class
	private MessageCountParser() {}

	public final int seqno() {
		return mSeqno;
	}

	public static int getSeqno(String reply) {
		try {
			MessageCountParser parser = new MessageCountParser();
			Xml.parse(reply, parser);
			return parser.seqno();
		}
		catch (SAXException e) {
			return -1;
		}		

	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("boinc_gui_rpc_reply")) {
			mInReply = true;
		}
	}

	@Override
	public void characters(char[] ch, int start, int length) throws SAXException {
		super.characters(ch, start, length);
		// put it into StringBuilder
		int myStart = start;
		int myLength = length;
		if (mCurrentElement.length() == 0) {
			// still empty - trim leading white-spaces
			for ( ; myStart < length; ++myStart, --myLength) {
				if (!Character.isWhitespace(ch[myStart])) {
					// First non-white-space character
					break;
				}
			}
		}
		mCurrentElement.append(ch, myStart, myLength);
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);

		try {
			trimEnd();
			if (localName.equalsIgnoreCase("boinc_gui_rpc_reply")) {
				mInReply = false;
			}
			else if (mInReply && !mParsed) {
				if (localName.equalsIgnoreCase("seqno")) {
					mSeqno = Integer.parseInt(mCurrentElement.toString());
					mParsed = true;
				}
			}
		}
		catch (NumberFormatException e) {
		}
		mCurrentElement.setLength(0);
	}

	private void trimEnd() {
		int length = mCurrentElement.length();
		int i;
		// Trim trailing spaces
		for (i = length - 1; i >= 0; --i) {
			if (!Character.isWhitespace(mCurrentElement.charAt(i))) {
				// All trailing white-spaces are skipped, i is position of last character
				break;
			}
		}
		// i is position of last character
		mCurrentElement.setLength(i+1);
	}
}
