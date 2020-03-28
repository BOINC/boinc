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

import org.apache.commons.lang3.StringUtils;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

public class NoticesParser extends BaseParser {
    static final String NOTICE_TAG = "notice";

    private Notice mNotice = null;
    private List<Notice> mNotices = new ArrayList<>();

    public final List<Notice> getNotices() {
        return mNotices;
    }

    public static List<Notice> parse(String rpcResult) {
        try {
            NoticesParser parser = new NoticesParser();
            Xml.parse(rpcResult.replace("&", "&amp;"), parser);
            return parser.getNotices();
        }
        catch(SAXException e) {
            Log.d("NoticesParser", "SAXException " + e.getMessage() + e.getException());
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(NOTICE_TAG)) {
            mNotice = new Notice();
        }
        else {
            // primitive
            mElementStarted = true;
            mCurrentElement.setLength(0);
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        try {
            if(mNotice != null) {
                // inside <notice>
                if(localName.equalsIgnoreCase(NOTICE_TAG)) {
                    // Closing tag
                    if(mNotice.getSeqno() != -1) {
                        // seqno is a must
                        mNotices.add(mNotice);
                    }
                    mNotice = null;
                }
                else {
                    // decode inner tags
                    if(localName.equalsIgnoreCase(Notice.Fields.SEQNO)) {
                        mNotice.setSeqno(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.TITLE)) {
                        mNotice.setTitle(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.DESCRIPTION)) {
                        mNotice.setDescription(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.CREATE_TIME)) {
                        mNotice.setCreateTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.ARRIVAL_TIME)) {
                        mNotice.setArrivalTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.CATEGORY)) {
                        mNotice.setCategory(mCurrentElement.toString());
                        if(StringUtils.equalsAny(mNotice.getCategory(), "server", "scheduler")) {
                            mNotice.setServerNotice(true);
                        }
                        if(mNotice.getCategory().equals("client")) {
                            mNotice.setClientNotice(true);
                        }
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.LINK)) {
                        mNotice.setLink(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT_NAME)) {
                        mNotice.setProjectName(mCurrentElement.toString());
                    }
                }
            }
            mElementStarted = false;
        }
        catch(NumberFormatException e) {
            Log.d("NoticesParser", "NumberFormatException " + localName + " "
                                   + mCurrentElement.toString());
        }
    }
}
