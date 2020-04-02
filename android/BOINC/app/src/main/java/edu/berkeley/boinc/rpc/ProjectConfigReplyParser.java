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
import java.util.List;

import org.apache.commons.lang3.StringUtils;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

import edu.berkeley.boinc.utils.Logging;

public class ProjectConfigReplyParser extends BaseParser {
    static final String PLATFORM_TAG = "platform";
    static final String PROJECT_CONFIG_TAG = "project_config";
    static final String USER_NAME_TAG = "user_name";
    static final String USES_USER_NAME_TAG = "uses_username";

    private ProjectConfig mProjectConfig = new ProjectConfig();

    private List<PlatformInfo> mPlatforms;
    private Boolean withinPlatforms = false;
    private String platformName = "";
    private String platformFriendlyName = "";
    private String platformPlanClass = "";

    ProjectConfig getProjectConfig() {
        return mProjectConfig;
    }

    /**
     * Parse the RPC result (state) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return connected client state
     */
    public static ProjectConfig parse(String rpcResult) {
        try {
            ProjectConfigReplyParser parser = new ProjectConfigReplyParser();
            //TODO report malformated XML to BOINC and remove String.replace here...
            //<boinc_gui_rpc_reply>
            //<?xml version="1.0" encoding="ISO-8859-1" ?>
            //<project_config>
            Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser);
            return parser.getProjectConfig();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(PROJECT_CONFIG_TAG)) {
            mProjectConfig = new ProjectConfig();
        }
        else if(localName.equalsIgnoreCase(ProjectConfig.Fields.PLATFORMS)) {
            withinPlatforms = true;
            mPlatforms = new ArrayList<>(); //initialize new list (flushing old elements)
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
            if(mProjectConfig != null) {
                if(localName.equalsIgnoreCase(ProjectConfig.Fields.PLATFORMS)) {
                    // closing tag of platform names
                    mProjectConfig.setPlatforms(mPlatforms);
                    withinPlatforms = false;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                        mProjectConfig.setName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.MASTER_URL)) {
                        mProjectConfig.setMasterUrl(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.WEB_RPC_URL_BASE)) {
                        mProjectConfig.setWebRpcUrlBase(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.LOCAL_REVISION)) {
                        mProjectConfig.setLocalRevision(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.MIN_PWD_LENGTH)) {
                        mProjectConfig.setMinPwdLength(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(StringUtils.equalsAnyIgnoreCase(localName, USER_NAME_TAG,
                                                            USES_USER_NAME_TAG)) {
                        mProjectConfig.setUsesName(true);
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.WEB_STOPPED)) {
                        mProjectConfig.setWebStopped(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.SCHEDULER_STOPPED)) {
                        mProjectConfig.setSchedulerStopped(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED)) {
                        mProjectConfig.setClientAccountCreationDisabled(true);
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.MIN_CLIENT_VERSION)) {
                        mProjectConfig.setMinClientVersion(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.RPC_PREFIX)) {
                        mProjectConfig.setRpcPrefix(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(PlatformInfo.Fields.NAME) &&
                            withinPlatforms.equals(Boolean.TRUE)) {
                        platformName = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.USER_FRIENDLY_NAME) &&
                            withinPlatforms.equals(Boolean.TRUE)) {
                        platformFriendlyName = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.PLAN_CLASS) &&
                            withinPlatforms.equals(Boolean.TRUE)) {
                        platformPlanClass = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.TERMS_OF_USE)) {
                        mProjectConfig.setTermsOfUse(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(PLATFORM_TAG) &&
                            withinPlatforms.equals(Boolean.TRUE)) {
                        // finish platform object and add to array
                        mPlatforms.add(new PlatformInfo(platformName, platformFriendlyName,
                                                        platformPlanClass));
                        platformFriendlyName = "";
                        platformName = "";
                        platformPlanClass = "";
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.ERROR_NUM)) {
                        // reply is not present yet
                        mProjectConfig.setErrorNum(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(ProjectConfig.Fields.ACCOUNT_MANAGER)) {
                        mProjectConfig.setAccountManager(true);
                    }
                }
            }
            mElementStarted = false;
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR.equals(Boolean.TRUE)) {
                Log.e(Logging.TAG, "ProjectConfigReplyParser.endElement error: ", e);
            }
        }
    }
}
