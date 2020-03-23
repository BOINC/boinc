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
                    if(mNotice.seqno != -1) {
                        // seqno is a must
                        mNotices.add(mNotice);
                    }
                    mNotice = null;
                }
                else {
                    // decode inner tags
                    if(localName.equalsIgnoreCase(Notice.Fields.seqno)) {
                        mNotice.seqno = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.title)) {
                        mNotice.title = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.description)) {
                        mNotice.description = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.create_time)) {
                        mNotice.create_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.arrival_time)) {
                        mNotice.arrival_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.category)) {
                        mNotice.category = mCurrentElement.toString();
                        if(StringUtils.equalsAny(mNotice.category, "server", "scheduler")) {
                            mNotice.isServerNotice = true;
                        }
                        if(mNotice.category.equals("client")) {
                            mNotice.isClientNotice = true;
                        }
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.link)) {
                        mNotice.link = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Notice.Fields.project_name)) {
                        mNotice.project_name = mCurrentElement.toString();
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
