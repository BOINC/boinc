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

import android.util.Xml;

import org.apache.commons.lang3.StringUtils;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import java.util.ArrayList;
import java.util.List;

public class AppsParser extends BaseParser {
    static final String APP_TAG = "app";

    private List<App> mApps = new ArrayList<>();
    private App mApp = null;

    public final List<App> getApps() {
        return mApps;
    }

    /**
     * Parse the RPC result (app) and generate vector of app
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of app
     */
    public static List<App> parse(String rpcResult) {
        try {
            AppsParser parser = new AppsParser();
            Xml.parse(rpcResult, parser);
            return parser.getApps();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(APP_TAG)) {
            mApp = new App();
        }
        else {
            // Another element, hopefully primitive and not constructor
            // (although unknown constructor does not hurt, because there will be primitive start anyway)
            mElementStarted = true;
            mCurrentElement.setLength(0);
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        if(mApp != null) {
            // We are inside <app>
            if(localName.equalsIgnoreCase(APP_TAG)) {
                // Closing tag of <app> - add to vector and be ready for next one
                if(StringUtils.isNotEmpty(mApp.getName())) {
                    // name is a must
                    mApps.add(mApp);
                }
                mApp = null;
            }
            else {
                // Not the closing tag - we decode possible inner tags
                trimEnd();
                if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                    mApp.setName(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(RPCCommonTags.USER_FRIENDLY_NAME)) {
                    mApp.setUserFriendlyName(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(RPCCommonTags.NON_CPU_INTENSIVE)) {
                    mApp.setNonCpuIntensive(Integer.parseInt(mCurrentElement.toString()));
                }
            }
        }
        mElementStarted = false;
    }
}
