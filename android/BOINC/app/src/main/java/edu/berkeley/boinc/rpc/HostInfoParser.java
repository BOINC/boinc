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

import edu.berkeley.boinc.utils.Logging;

public class HostInfoParser extends BaseParser {
    static final String HOST_INFO_TAG = "host_info";

    private HostInfo mHostInfo = null;

    HostInfo getHostInfo() {
        return mHostInfo;
    }

    /**
     * Parse the RPC result (host_info) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return HostInfo
     */
    public static HostInfo parse(String rpcResult) {
        try {
            HostInfoParser parser = new HostInfoParser();
            Xml.parse(rpcResult, parser);
            return parser.getHostInfo();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(HOST_INFO_TAG)) {
            mHostInfo = new HostInfo();
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
            if(mHostInfo != null) {
                // we are inside <host_info>
                // Closing tag of <host_info> - nothing to do at the moment
                if (!localName.equalsIgnoreCase(HOST_INFO_TAG)) {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(localName.equalsIgnoreCase(HostInfo.Fields.TIMEZONE)) {
                        mHostInfo.setTimezone(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.DOMAIN_NAME)) {
                        mHostInfo.setDomainName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.IP_ADDR)) {
                        mHostInfo.setIpAddress(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.HOST_CPID)) {
                        mHostInfo.setHostCpid(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_NCPUS)) {
                        mHostInfo.setNoOfCPUs(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_VENDOR)) {
                        mHostInfo.setCpuVendor(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_MODEL)) {
                        mHostInfo.setCpuModel(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_FEATURES)) {
                        mHostInfo.setCpuFeatures(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_FPOPS)) {
                        mHostInfo.setCpuFloatingPointOps(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_IOPS)) {
                        mHostInfo.setCpuIntegerOps(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_MEMBW)) {
                        mHostInfo.setCpuMembw(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.P_CALCULATED)) {
                        mHostInfo.setCpuCalculated((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.PRODUCT_NAME)) {
                        mHostInfo.setProductName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.M_NBYTES)) {
                        mHostInfo.setMemoryInBytes(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.M_CACHE)) {
                        mHostInfo.setMemoryCache(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.M_SWAP)) {
                        mHostInfo.setMemorySwap(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.D_TOTAL)) {
                        mHostInfo.setTotalDiskSpace(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.D_FREE)) {
                        mHostInfo.setFreeDiskSpace(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.OS_NAME)) {
                        mHostInfo.setOsName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.OS_VERSION)) {
                        mHostInfo.setOsVersion(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(HostInfo.Fields.VIRTUALBOX_VERSION)) {
                        mHostInfo.setVirtualBoxVersion(mCurrentElement.toString());
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "HostInfoParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
