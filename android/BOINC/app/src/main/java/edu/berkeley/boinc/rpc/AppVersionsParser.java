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

import org.apache.commons.lang3.StringUtils;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import edu.berkeley.boinc.utils.Logging;

public class AppVersionsParser extends BaseParser {
    static final String APP_VERSION_TAG = "app_version";

    private List<AppVersion> mAppVersions = new ArrayList<>();
    private AppVersion mAppVersion = null;

    final List<AppVersion> getAppVersions() {
        return mAppVersions;
    }

    /**
     * Parse the RPC result (app_version) and generate corresponding vector
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of application version
     */
    public static List<AppVersion> parse(String rpcResult) {
        try {
            AppVersionsParser parser = new AppVersionsParser();
            Xml.parse(rpcResult, parser);
            return parser.getAppVersions();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(APP_VERSION_TAG)) {
            mAppVersion = new AppVersion();
        }
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        super.characters(ch, start, length);
        mCurrentElement.setLength(0); // clear buffer after superclass operation
        // still empty - trim leading whitespace characters and append
        mCurrentElement.append(StringUtils.stripStart(new String(ch), null));
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        try {
            trimEnd();
            if(mAppVersion != null) {
                // We are inside <app_version>
                if(localName.equalsIgnoreCase(APP_VERSION_TAG)) {
                    // Closing tag of <app_version> - add to vector and be ready for next one
                    if(StringUtils.isNotEmpty(mAppVersion.getAppName())) {
                        // app_name is a must
                        mAppVersions.add(mAppVersion);
                    }
                    mAppVersion = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    if(localName.equalsIgnoreCase(AppVersion.Fields.APP_NAME)) {
                        mAppVersion.setAppName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(AppVersion.Fields.VERSION_NUM)) {
                        mAppVersion.setVersionNum(Integer.parseInt(mCurrentElement.toString()));
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR.equals(Boolean.TRUE)) {
                Log.e(Logging.TAG, "AppVersionsParser.endElement error: ", e);
            }
        }
        mCurrentElement.setLength(0); // to be clean for next one
    }
}
