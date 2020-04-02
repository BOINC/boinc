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
import java.util.Collections;
import java.util.List;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

import edu.berkeley.boinc.utils.Logging;

public class TransfersParser extends BaseParser {
    static final String FILE_TRANSFER_TAG = "file_transfer";
    static final String FILE_XFER_TAG = "file_xfer";
    static final String LAST_BYTES_XFERRED_TAG = "last_bytes_xferred";

    private List<Transfer> mTransfers = new ArrayList<>();
    private Transfer mTransfer = null;

    List<Transfer> getTransfers() {
        return mTransfers;
    }

    /**
     * Parse the RPC result (projects) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of projects info
     */
    public static List<Transfer> parse(String rpcResult) {
        try {
            TransfersParser parser = new TransfersParser();
            Xml.parse(rpcResult, parser);
            return parser.getTransfers();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(FILE_TRANSFER_TAG)) {
            mTransfer = new Transfer();
        }
        else if(localName.equalsIgnoreCase(FILE_XFER_TAG)) {
            // Just constructor, flag should be set if it's present
            if(mTransfer != null) {
                mTransfer.setTransferActive(true);
            }
        }
        else if(localName.equalsIgnoreCase("persistent_file_xfer")) {
            // Just constructor, but nothing to do here
            // We just do not set mElementStarted flag here, so we will
            // avoid unnecessary work in BaseParser.characters()
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
            if(mTransfer != null) {
                // We are inside <file_transfer>
                if(localName.equalsIgnoreCase(FILE_TRANSFER_TAG)) {
                    // Closing tag of <project> - add to list and be ready for next one
                    if(!mTransfer.getProjectUrl().isEmpty() && !mTransfer.getName().isEmpty()) {
                        // project_url is a must
                        mTransfers.add(mTransfer);
                    }
                    mTransfer = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT_URL)) {
                        mTransfer.setProjectUrl(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                        mTransfer.setName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.GENERATED_LOCALLY)) {
                        mTransfer.setGeneratedLocally(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.IS_UPLOAD)) {
                        mTransfer.setUpload(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.NBYTES)) {
                        mTransfer.setNoOfBytes((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.STATUS)) {
                        mTransfer.setStatus(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.TIME_SO_FAR)) {
                        // inside <persistent_file_xfer>
                        mTransfer.setTimeSoFar((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.NEXT_REQUEST_TIME)) {
                        // inside <persistent_file_xfer>
                        mTransfer.setNextRequestTime((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(LAST_BYTES_XFERRED_TAG)) {
                        // inside <persistent_file_xfer>
                        // See also <bytes_xferred> below, both are setting the same parameters
                        if(mTransfer.getBytesTransferred() == 0) {
                            // Not set yet
                            mTransfer.setBytesTransferred((long) Double.parseDouble(mCurrentElement.toString()));
                        }
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.BYTES_XFERRED)) {
                        // Total bytes transferred, but this info is not available if networking
                        // is suspended. This info is present only inside <file_xfer> (active transfer)
                        // In such case we overwrite value set by <last_bytes_xferred>
                        mTransfer.setBytesTransferred((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.XFER_SPEED)) {
                        // inside <file_xfer>
                        mTransfer.setTransferSpeed(Float.parseFloat(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Transfer.Fields.PROJECT_BACKOFF)) {
                        mTransfer.setProjectBackoff((long) Double.parseDouble(mCurrentElement.toString()));
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR.equals(Boolean.TRUE)) {
                Log.e(Logging.TAG, "TransfersParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
