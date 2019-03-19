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

import java.util.ArrayList;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import android.util.Log;
import android.util.Xml;

import edu.berkeley.boinc.utils.Logging;


public class AppVersionsParser extends DefaultHandler {

    private ArrayList<AppVersion> mAppVersions = new ArrayList<>();
    private AppVersion mAppVersion = null;
    private StringBuilder mCurrentElement = new StringBuilder();


    public final ArrayList<AppVersion> getAppVersions() {
        return mAppVersions;
    }

    /**
     * Parse the RPC result (app_version) and generate corresponding vector
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of application version
     */
    public static ArrayList<AppVersion> parse(String rpcResult) {
        try {
            AppVersionsParser parser = new AppVersionsParser();
            Xml.parse(rpcResult, parser);
            return parser.getAppVersions();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase("app_version")) {
            mAppVersion = new AppVersion();
        }
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        super.characters(ch, start, length);
        // put it into StringBuilder
        int myStart = start;
        int myLength = length;
        if(mCurrentElement.length() == 0) {
            // still empty - trim leading white-spaces
            for(; myStart < length; ++myStart, --myLength) {
                if(!Character.isWhitespace(ch[myStart])) {
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
            if(mAppVersion != null) {
                // We are inside <app_version>
                if(localName.equalsIgnoreCase("app_version")) {
                    // Closing tag of <app_version> - add to vector and be ready for next one
                    if(!mAppVersion.app_name.equals("")) {
                        // app_name is a must
                        mAppVersions.add(mAppVersion);
                    }
                    mAppVersion = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    if(localName.equalsIgnoreCase("app_name")) {
                        mAppVersion.app_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("version_num")) {
                        mAppVersion.version_num = Integer.parseInt(mCurrentElement.toString());
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "AppVersionsParser.endElement error: ", e);
            }
        }
        mCurrentElement.setLength(0); // to be clean for next one
    }

    private void trimEnd() {
        int length = mCurrentElement.length();
        int i;
        // Trim trailing spaces
        for(i = length - 1; i >= 0; --i) {
            if(!Character.isWhitespace(mCurrentElement.charAt(i))) {
                // All trailing white-spaces are skipped, i is position of last character
                break;
            }
        }
        // i is position of last character
        mCurrentElement.setLength(i + 1);
    }
}
