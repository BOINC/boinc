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
import java.util.List;

import edu.berkeley.boinc.utils.Logging;


public class ProjectsParser extends BaseParser {
    static final String GUI_URL_TAG = "gui_url";

    static final String PROJECT_TAG = "project";
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
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(PROJECT_TAG)) {
            mProject = new Project();
        }
        else if(localName.equalsIgnoreCase(GUI_URL_TAG)) {
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
                if(localName.equalsIgnoreCase(PROJECT_TAG)) {
                    // Closing tag of <project> - add to vector and be ready for next one
                    if(!mProject.master_url.isEmpty()) {
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
                        if(localName.equalsIgnoreCase(GUI_URL_TAG)) {
                            // finish of this <gui_url> element
                            mProject.gui_urls.add(mGuiUrl);
                            mGuiUrl = null;
                        }
                        else {
                            if(localName.equalsIgnoreCase(GuiUrl.Fields.name)) {
                                mGuiUrl.name = mCurrentElement.toString();
                            }
                            else if(localName.equalsIgnoreCase(GuiUrl.Fields.description)) {
                                mGuiUrl.description = mCurrentElement.toString();
                            }
                            else if(localName.equalsIgnoreCase(GuiUrl.Fields.url)) {
                                mGuiUrl.url = mCurrentElement.toString();
                            }
                        }
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.master_url)) {
                        mProject.master_url = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.project_dir)) {
                        mProject.project_dir = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.resource_share)) {
                        mProject.resource_share = Float.parseFloat(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.project_name)) {
                        mProject.project_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.user_name)) {
                        mProject.user_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.team_name)) {
                        mProject.team_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.hostid)) {
                        mProject.hostid = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.host_venue)) {
                        mProject.host_venue = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.user_total_credit)) {
                        mProject.user_total_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.user_expavg_credit)) {
                        mProject.user_expavg_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.host_total_credit)) {
                        mProject.host_total_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.host_expavg_credit)) {
                        mProject.host_expavg_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.nrpc_failures)) {
                        mProject.nrpc_failures = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.master_fetch_failures)) {
                        mProject.master_fetch_failures = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.min_rpc_time)) {
                        mProject.min_rpc_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.download_backoff)) {
                        mProject.download_backoff = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.upload_backoff)) {
                        mProject.upload_backoff = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(SHORT_TERM_DEBT_TAG)) {
                        mProject.cpu_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(LONG_TERM_DEBT_TAG)) {
                        mProject.cpu_long_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cpu_backoff_time)) {
                        mProject.cpu_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cpu_backoff_interval)) {
                        mProject.cpu_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cuda_debt)) {
                        mProject.cuda_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cuda_short_term_debt)) {
                        mProject.cuda_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cuda_backoff_time)) {
                        mProject.cuda_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.cuda_backoff_interval)) {
                        mProject.cuda_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ati_debt)) {
                        mProject.ati_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ati_short_term_debt)) {
                        mProject.ati_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ati_backoff_time)) {
                        mProject.ati_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ati_backoff_interval)) {
                        mProject.ati_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.duration_correction_factor)) {
                        mProject.duration_correction_factor = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.master_url_fetch_pending)) {
                        mProject.master_url_fetch_pending = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.sched_rpc_pending)) {
                        mProject.sched_rpc_pending = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.non_cpu_intensive)) {
                        mProject.non_cpu_intensive = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.suspended_via_gui)) {
                        mProject.suspended_via_gui = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.dont_request_more_work)) {
                        mProject.dont_request_more_work = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.scheduler_rpc_in_progress)) {
                        mProject.scheduler_rpc_in_progress = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.attached_via_acct_mgr)) {
                        mProject.attached_via_acct_mgr = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.detach_when_done)) {
                        mProject.detach_when_done = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.ended)) {
                        mProject.ended = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.trickle_up_pending)) {
                        mProject.trickle_up_pending = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.project_files_downloaded_time)) {
                        mProject.project_files_downloaded_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.last_rpc_time)) {
                        mProject.last_rpc_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.no_cpu_pref)) {
                        mProject.no_cpu_pref = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.no_cuda_pref)) {
                        mProject.no_cuda_pref = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase(Project.Fields.no_ati_pref)) {
                        mProject.no_ati_pref = !mCurrentElement.toString().equals("0");
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
