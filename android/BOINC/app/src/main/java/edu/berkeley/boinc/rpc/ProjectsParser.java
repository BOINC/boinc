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

public class ProjectsParser extends BaseParser {
    static final String SHORT_TERM_DEBT_TAG = "short_term_debt";
    static final String LONG_TERM_DEBT_TAG = "long_term_debt";

    private List<Project> mProjects = new ArrayList<>();
    private Project mProject = null;
    private GuiUrl mGuiUrl = null;

    public final List<Project> getProjects() {
        return mProjects;
    }

    /**
     * Parse the RPC result (projects) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of projects info
     */
    public static List<Project> parse(String rpcResult) {
        try {
            ProjectsParser parser = new ProjectsParser();
            Xml.parse(rpcResult, parser);
            return parser.getProjects();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT)) {
            mProject = new Project();
        }
        else if(localName.equalsIgnoreCase(RPCCommonTags.GUI_URL)) {
            mGuiUrl = new GuiUrl();
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
            if(mProject != null) {
                // We are inside <project>
                if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT)) {
                    // Closing tag of <project> - add to vector and be ready for next one
                    if(!mProject.getMasterURL().isEmpty()) {
                        // master_url is a must
                        mProjects.add(mProject);
                    }
                    mProject = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(mGuiUrl != null) {
                        // We are inside <gui_url> element
                        if(localName.equalsIgnoreCase(RPCCommonTags.GUI_URL)) {
                            // finish of this <gui_url> element
                            mProject.getGuiURLs().add(mGuiUrl);
                            mGuiUrl = null;
                        }
                        else {
                            if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                                mGuiUrl.setName(mCurrentElement.toString());
                            }
                            else if(localName.equalsIgnoreCase(RPCCommonTags.DESCRIPTION)) {
                                mGuiUrl.setDescription(mCurrentElement.toString());
                            }
                            else if(localName.equalsIgnoreCase(RPCCommonTags.URL)) {
                                mGuiUrl.setUrl(mCurrentElement.toString());
                            }
                        }
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.MASTER_URL)) {
                        mProject.setMasterURL(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.PROJECT_DIR)) {
                        mProject.setProjectDir(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.RESOURCE_SHARE)) {
                        mProject.setResourceShare(Float.parseFloat(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT_NAME)) {
                        mProject.setProjectName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.USER_NAME)) {
                        mProject.setUserName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.TEAM_NAME)) {
                        mProject.setTeamName(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.HOSTID)) {
                        mProject.setHostId(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.HOST_VENUE)) {
                        mProject.setHostVenue(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.USER_TOTAL_CREDIT)) {
                        mProject.setUserTotalCredit(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.USER_EXPAVG_CREDIT)) {
                        mProject.setUserExpAvgCredit(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.HOST_TOTAL_CREDIT)) {
                        mProject.setHostTotalCredit(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.HOST_EXPAVG_CREDIT)) {
                        mProject.setHostExpAvgCredit(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.NRPC_FAILURES)) {
                        mProject.setNoOfRPCFailures(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.MASTER_FETCH_FAILURES)) {
                        mProject.setMasterFetchFailures(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.MIN_RPC_TIME)) {
                        mProject.setMinRPCTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.DOWNLOAD_BACKOFF)) {
                        mProject.setDownloadBackoff(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.UPLOAD_BACKOFF)) {
                        mProject.setUploadBackoff(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(SHORT_TERM_DEBT_TAG)) {
                        mProject.setCpuShortTermDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(LONG_TERM_DEBT_TAG)) {
                        mProject.setCpuLongTermDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CPU_BACKOFF_TIME)) {
                        mProject.setCpuBackoffTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CPU_BACKOFF_INTERVAL)) {
                        mProject.setCpuBackoffInterval(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CUDA_DEBT)) {
                        mProject.setCudaDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CUDA_SHORT_TERM_DEBT)) {
                        mProject.setCudaShortTermDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CUDA_BACKOFF_TIME)) {
                        mProject.setCudaBackoffTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.CUDA_BACKOFF_INTERVAL)) {
                        mProject.setCudaBackoffInterval(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ATI_DEBT)) {
                        mProject.setAtiDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ATI_SHORT_TERM_DEBT)) {
                        mProject.setAtiShortTermDebt(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ATI_BACKOFF_TIME)) {
                        mProject.setAtiBackoffTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ATI_BACKOFF_INTERVAL)) {
                        mProject.setAtiBackoffInterval(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.DURATION_CORRECTION_FACTOR)) {
                        mProject.setDurationCorrectionFactor(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.MASTER_URL_FETCH_PENDING)) {
                        mProject.setMasterURLFetchPending(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.SCHED_RPC_PENDING)) {
                        mProject.setScheduledRPCPending(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(RPCCommonTags.NON_CPU_INTENSIVE)) {
                        mProject.setNonCPUIntensive(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.SUSPENDED_VIA_GUI)) {
                        mProject.setSuspendedViaGUI(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.DONT_REQUEST_MORE_WORK)) {
                        mProject.setDoNotRequestMoreWork(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.SCHEDULER_RPC_IN_PROGRESS)) {
                        mProject.setSchedulerRPCInProgress(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ATTACHED_VIA_ACCT_MGR)) {
                        mProject.setAttachedViaAcctMgr(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.DETACH_WHEN_DONE)) {
                        mProject.setDetachWhenDone(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ENDED)) {
                        mProject.setEnded(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.TRICKLE_UP_PENDING)) {
                        mProject.setTrickleUpPending(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.PROJECT_FILES_DOWNLOADED_TIME)) {
                        mProject.setProjectFilesDownloadedTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.LAST_RPC_TIME)) {
                        mProject.setLastRPCTime(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.NO_CPU_PREF)) {
                        mProject.setNoCPUPref(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.NO_CUDA_PREF)) {
                        mProject.setNoCUDAPref(!mCurrentElement.toString().equals("0"));
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.NO_ATI_PREF)) {
                        mProject.setNoATIPref(!mCurrentElement.toString().equals("0"));
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectsParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
