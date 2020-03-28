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

import android.util.Xml;

public class ProjectInfoParser extends BaseParser {
    private List<ProjectInfo> mProjectInfos = new ArrayList<>();
    private ProjectInfo mProjectInfo = null;
    private List<String> mPlatforms;
    private Boolean withinPlatforms = false;

    List<ProjectInfo> getProjectInfos() {
        return mProjectInfos;
    }

    public static List<ProjectInfo> parse(String rpcResult) {
        try {
            ProjectInfoParser parser = new ProjectInfoParser();
            // report malformated XML to BOINC and remove String.replace here...
            Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser);
            return parser.getProjectInfos();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT)) {
            mProjectInfo = new ProjectInfo();
        }
        else if(localName.equalsIgnoreCase(ProjectInfo.Fields.PLATFORMS)) {
            mPlatforms = new ArrayList<>(); //initialize new list (flushing old elements)
            withinPlatforms = true;
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
        if(mProjectInfo != null) {
            if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT)) {
                // Closing tag of <project> - add to list and be ready for next one
                if(!mProjectInfo.getName().isEmpty()) {
                    // name is a must
                    mProjectInfos.add(mProjectInfo);
                }
                mProjectInfo = null;
            }
            else if(localName.equalsIgnoreCase(ProjectInfo.Fields.PLATFORMS)) {
                // closing tag of platform names
                mProjectInfo.setPlatforms(mPlatforms);
                withinPlatforms = false;
            }
            else {
                // Not the closing tag - we decode possible inner tags
                trimEnd();
                if(localName.equalsIgnoreCase(RPCCommonTags.NAME) &&
                   withinPlatforms.equals(Boolean.FALSE)) {
                    //project name
                    mProjectInfo.setName(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(RPCCommonTags.URL)) {
                    mProjectInfo.setUrl(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(ProjectInfo.Fields.GENERAL_AREA)) {
                    mProjectInfo.setGeneralArea(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(ProjectInfo.Fields.SPECIFIC_AREA)) {
                    mProjectInfo.setSpecificArea(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(RPCCommonTags.DESCRIPTION)) {
                    mProjectInfo.setDescription(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(ProjectInfo.Fields.HOME)) {
                    mProjectInfo.setHome(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(RPCCommonTags.NAME) &&
                        withinPlatforms.equals(Boolean.TRUE)) {
                    //platform name
                    mPlatforms.add(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(ProjectInfo.Fields.IMAGE_URL)) {
                    mProjectInfo.setImageUrl(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(ProjectInfo.Fields.SUMMARY)) {
                    mProjectInfo.setSummary(mCurrentElement.toString());
                }
            }
        }
        mElementStarted = false;
    }
}
