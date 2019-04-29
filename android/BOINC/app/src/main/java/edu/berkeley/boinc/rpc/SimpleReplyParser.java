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

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Xml;

public class SimpleReplyParser extends BaseParser {
    private boolean mParsed = false;
    private boolean mInReply = false;
    private boolean mSuccess = false;

    private String errorMessage = null;

    public final String getErrorMessage() {
        return errorMessage;
    }

    // Disable direct instantiation of this class
    private SimpleReplyParser() {
    }

    public final boolean result() {
        return mSuccess;
    }

    public static SimpleReplyParser parse(String reply) {
        try {
            SimpleReplyParser parser = new SimpleReplyParser();
            Xml.parse(reply, parser);
            return parser;
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase("boinc_gui_rpc_reply")) {
            mInReply = true;
        }
        else {
            mElementStarted = true;
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);

        if(localName.equalsIgnoreCase("boinc_gui_rpc_reply")) {
            mInReply = false;
        }
        else if(mInReply && !mParsed) {
            if(localName.equalsIgnoreCase("success")) {
                mSuccess = true;
                mParsed = true;
            }
            else if(localName.equalsIgnoreCase("failure")) {
                mSuccess = false;
                mParsed = true;
            }
            else if(localName.equalsIgnoreCase("error")) {
                trimEnd();
                errorMessage = mCurrentElement.toString();
                mSuccess = false;
                mParsed = true;
            }
        }
        mElementStarted = false;
    }
}
