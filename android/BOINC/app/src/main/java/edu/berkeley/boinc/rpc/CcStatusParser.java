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

import android.util.Log;
import android.util.Xml;

import java.util.Objects;

import edu.berkeley.boinc.utils.Logging;

public class CcStatusParser extends BaseParser {
    static final String CC_STATUS_TAG = "cc_status";

    private CcStatus mCcStatus;

    final CcStatus getCcStatus() {
        return mCcStatus;
    }

    public static CcStatus parse(String rpcResult) {
        try {
            CcStatusParser parser = new CcStatusParser();
            Xml.parse(rpcResult, parser);
            return parser.getCcStatus();
        }
        catch(SAXException e) {
            Log.v(Logging.TAG, Objects.requireNonNull(e.getMessage()));
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(CC_STATUS_TAG)) {
            mCcStatus = new CcStatus();
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
        try {
            if(mCcStatus != null && // We are inside <cc_status>
               !localName.equalsIgnoreCase(CC_STATUS_TAG) // Closing tag of <cc_status> - nothing to do at the moment
            ) {
                trimEnd();
                // Not the closing tag - we decode possible inner tags
                if(localName.equalsIgnoreCase(CcStatus.Fields.TASK_MODE)) {
                    mCcStatus.setTaskMode(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.TASK_MODE_PERM)) {
                    mCcStatus.setTaskModePerm(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.TASK_MODE_DELAY)) {
                    mCcStatus.setTaskModeDelay(Double.parseDouble(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.TASK_SUSPEND_REASON)) {
                    mCcStatus.setTaskSuspendReason(Integer.parseInt(mCurrentElement.toString()));
                }
                if(localName.equalsIgnoreCase(CcStatus.Fields.NETWORK_MODE)) {
                    mCcStatus.setNetworkMode(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.NETWORK_MODE_PERM)) {
                    mCcStatus.setNetworkModePerm(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.NETWORK_MODE_DELAY)) {
                    mCcStatus.setNetworkModeDelay(Double.parseDouble(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.NETWORK_SUSPEND_REASON)) {
                    mCcStatus.setNetworkSuspendReason(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.NETWORK_STATUS)) {
                    mCcStatus.setNetworkStatus(Integer.parseInt(mCurrentElement.toString()));
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.AMS_PASSWORD_ERROR)) {
                    if(mCurrentElement.length() > 1) {
                        mCcStatus.setAmsPasswordError(0 != Integer.parseInt(mCurrentElement.toString()));
                    }
                    else {
                        mCcStatus.setAmsPasswordError(true);
                    }
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.MANAGER_MUST_QUIT)) {
                    if(mCurrentElement.length() > 1) {
                        mCcStatus.setManagerMustQuit(0 != Integer.parseInt(mCurrentElement.toString()));
                    }
                    else {
                        mCcStatus.setManagerMustQuit(true);
                    }
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.DISALLOW_ATTACH)) {
                    if(mCurrentElement.length() > 1) {
                        mCcStatus.setDisallowAttach(0 != Integer.parseInt(mCurrentElement.toString()));
                    }
                    else {
                        mCcStatus.setDisallowAttach(true);
                    }
                }
                else if(localName.equalsIgnoreCase(CcStatus.Fields.SIMPLE_GUI_ONLY)) {
                    if(mCurrentElement.length() > 1) {
                        mCcStatus.setSimpleGuiOnly(0 != Integer.parseInt(mCurrentElement.toString()));
                    }
                    else {
                        mCcStatus.setSimpleGuiOnly(true);
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "CcStatusParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
