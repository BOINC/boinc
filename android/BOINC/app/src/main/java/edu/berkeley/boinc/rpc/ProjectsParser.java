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

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

import edu.berkeley.boinc.utils.Logging;


public class ProjectsParser extends BaseParser {

    private ArrayList<Project> mProjects = new ArrayList<>();
    private Project mProject = null;
    private GuiUrl mGuiUrl = null;


    public final ArrayList<Project> getProjects() {
        return mProjects;
    }

    /**
     * Parse the RPC result (projects) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of projects info
     */
    public static ArrayList<Project> parse(String rpcResult) {
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
        if(localName.equalsIgnoreCase("project")) {
            mProject = new Project();
        }
        else if(localName.equalsIgnoreCase("gui_url")) {
            mGuiUrl = new GuiUrl();
        }
        else {
            // Another element, hopefully primitive and not constructor
            // (although unknown constructor does not hurt, because there will be primitive start anyway)
            mElementStarted = true;
            mCurrentElement.setLength(0);
        }
    }

    // Method characters(char[] ch, int start, int length) is implemented by BaseParser,
    // filling mCurrentElement (including stripping of leading whitespaces)
    //@Override
    //public void characters(char[] ch, int start, int length) throws SAXException { }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        try {
            if(mProject != null) {
                // We are inside <project>
                if(localName.equalsIgnoreCase("project")) {
                    // Closing tag of <project> - add to vector and be ready for next one
                    if(!mProject.master_url.equals("")) {
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
                        if(localName.equalsIgnoreCase("gui_url")) {
                            // finish of this <gui_url> element
                            mProject.gui_urls.add(mGuiUrl);
                            mGuiUrl = null;
                        }
                        else {
                            if(localName.equalsIgnoreCase("name")) {
                                mGuiUrl.name = mCurrentElement.toString();
                            }
                            else if(localName.equalsIgnoreCase("description")) {
                                mGuiUrl.description = mCurrentElement.toString();
                            }
                            else if(localName.equalsIgnoreCase("url")) {
                                mGuiUrl.url = mCurrentElement.toString();
                            }
                        }
                    }
                    else if(localName.equalsIgnoreCase("master_url")) {
                        mProject.master_url = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("project_dir")) {
                        mProject.project_dir = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("resource_share")) {
                        mProject.resource_share = Float.parseFloat(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("project_name")) {
                        mProject.project_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("user_name")) {
                        mProject.user_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("team_name")) {
                        mProject.team_name = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("hostid")) {
                        mProject.hostid = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("host_venue")) {
                        mProject.venue = mCurrentElement.toString();
                    }
                    else if(localName.equalsIgnoreCase("user_total_credit")) {
                        mProject.user_total_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("user_expavg_credit")) {
                        mProject.user_expavg_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("host_total_credit")) {
                        mProject.host_total_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("host_expavg_credit")) {
                        mProject.host_expavg_credit = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("nrpc_failures")) {
                        mProject.nrpc_failures = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("master_fetch_failures")) {
                        mProject.master_fetch_failures = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("min_rpc_time")) {
                        mProject.min_rpc_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("download_backoff")) {
                        mProject.download_backoff = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("upload_backoff")) {
                        mProject.upload_backoff = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("short_term_debt")) {
                        mProject.cpu_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("long_term_debt")) {
                        mProject.cpu_long_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cpu_backoff_time")) {
                        mProject.cpu_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cpu_backoff_interval")) {
                        mProject.cpu_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cuda_debt")) {
                        mProject.cuda_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cuda_short_term_debt")) {
                        mProject.cuda_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cuda_backoff_time")) {
                        mProject.cuda_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("cuda_backoff_interval")) {
                        mProject.cuda_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("ati_debt")) {
                        mProject.ati_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("ati_short_term_debt")) {
                        mProject.ati_short_term_debt = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("ati_backoff_time")) {
                        mProject.ati_backoff_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("ati_backoff_interval")) {
                        mProject.ati_backoff_interval = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("duration_correction_factor")) {
                        mProject.duration_correction_factor = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("master_url_fetch_pending")) {
                        //						String trimmed = mCurrentElement.trim();
                        //						mProject.master_url_fetch_pending = !trimmed.equals("0");
                        mProject.master_url_fetch_pending = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("sched_rpc_pending")) {
                        mProject.sched_rpc_pending = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("non_cpu_intensive")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.non_cpu_intensive = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("suspended_via_gui")) {
                        mProject.suspended_via_gui = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("dont_request_more_work")) {
                        mProject.dont_request_more_work = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("scheduler_rpc_in_progress")) {
                        mProject.scheduler_rpc_in_progress = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("attached_via_acct_mgr")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.attached_via_acct_mgr = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("detach_when_done")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.detach_when_done = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("ended")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.ended = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("trickle_up_pending")) {
                        mProject.trickle_up_pending = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("project_files_downloaded_time")) {
                        mProject.project_files_downloaded_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("last_rpc_time")) {
                        mProject.last_rpc_time = Double.parseDouble(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase("no_cpu_pref")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.no_cpu_pref = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("no_cuda_pref")) {
                        //String trimmed = mCurrentElement.trim();
                        mProject.no_cuda_pref = !mCurrentElement.toString().equals("0");
                    }
                    else if(localName.equalsIgnoreCase("no_ati_pref")) {
                        //String trimmed = mCurrentElement.trim();
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
