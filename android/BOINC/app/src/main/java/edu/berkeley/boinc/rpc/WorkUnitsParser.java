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
import java.util.Collections;
import java.util.List;

import edu.berkeley.boinc.utils.Logging;

public class WorkUnitsParser extends BaseParser {
    static final String WORKUNIT_TAG = "workunit";

    private List<WorkUnit> mWorkUnits = new ArrayList<>();
    private WorkUnit mWorkUnit = null;

    final List<WorkUnit> getWorkUnits() {
        return mWorkUnits;
    }

    /**
     * Parse the RPC result (workunit) and generate corresponding vector
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of workunits
     */
    public static List<WorkUnit> parse(String rpcResult) {
        try {
            WorkUnitsParser parser = new WorkUnitsParser();
            Xml.parse(rpcResult, parser);
            return parser.getWorkUnits();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(WORKUNIT_TAG)) {
            mWorkUnit = new WorkUnit();
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
            if(mWorkUnit != null) {
                // We are inside <workunit>
                if(localName.equalsIgnoreCase(WORKUNIT_TAG)) {
                    // Closing tag of <workunit> - add to vector and be ready for next one
                    if(!mWorkUnit.getName().isEmpty()) {
                        // name is a must
                        mWorkUnits.add(mWorkUnit);
                    }
                    mWorkUnit = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                        mWorkUnit.setName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.APP_NAME)) {
                        mWorkUnit.setAppName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.VERSION_NUM)) {
                        mWorkUnit.setVersionNum(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.RSC_FPOPS_EST)) {
                        mWorkUnit.setRscFloatingPointOpsEst(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.RSC_FPOPS_BOUND)) {
                        mWorkUnit.setRscFloatingPointOpsBound(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.RSC_MEMORY_BOUND)) {
                        mWorkUnit.setRscMemoryBound(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(WorkUnit.Fields.RSC_DISK_BOUND)) {
                        mWorkUnit.setRscDiskBound(Double.parseDouble(mCurrentElement.toString()));
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "WorkunitsParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
