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

public class ResultsParser extends BaseParser {

    private ArrayList<Result> mResults = new ArrayList<>();
    private Result mResult = null;
    private boolean mInActiveTask = false;

    public ArrayList<Result> getResults() {
        return mResults;
    }

    /**
     * Parse the RPC result (results) and generate vector of results info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of results info
     */
    public static ArrayList<Result> parse(String rpcResult) {
        try {
            ResultsParser parser = new ResultsParser();
            Xml.parse(rpcResult, parser);
            return parser.getResults();
        }
        catch(SAXException e) {
            return null;
        }

    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase("result")) {
            mResult = new Result();
        }
        else if(localName.equalsIgnoreCase("active_task")) {
            mInActiveTask = true;
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
            if(mResult != null) {
                // We are inside <result>
                if(localName.equalsIgnoreCase("result")) {
                    // Closing tag of <result> - add to vector and be ready for
                    // next one
                    if(!mResult.name.equals("")) {
                        // name is a must
                        mResults.add(mResult);
                    }
                    mResult = null;
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(mInActiveTask) {
                        // we are in <active_task>
                        if(localName.equalsIgnoreCase("active_task")) {
                            // Closing of <active_task>
                            mResult.active_task = true;
                            mInActiveTask = false;
                        }
                        else if(localName.equalsIgnoreCase("active_task_state")) {
                            mResult.active_task_state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("app_version_num")) {
                            mResult.app_version_num = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("scheduler_state")) {
                            mResult.scheduler_state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("checkpoint_cpu_time")) {
                            mResult.checkpoint_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("current_cpu_time")) {
                            mResult.current_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("fraction_done")) {
                            mResult.fraction_done = Float.parseFloat(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("elapsed_time")) {
                            mResult.elapsed_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("swap_size")) {
                            mResult.swap_size = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("working_set_size_smoothed")) {
                            mResult.working_set_size_smoothed = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("estimated_cpu_time_remaining")) {
                            mResult.estimated_cpu_time_remaining = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("supports_graphics")) {
                            mResult.supports_graphics = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("graphic_mode_acked")) {
                            mResult.graphics_mode_acked = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("too_large")) {
                            mResult.too_large = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("needs_shmem")) {
                            mResult.needs_shmem = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("edf_scheduled")) {
                            mResult.edf_scheduled = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("pid")) {
                            mResult.pid = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("slot")) {
                            mResult.slot = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("graphics_exec_path")) {
                            mResult.graphics_exec_path = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase("slot_path")) {
                            mResult.slot_path = mCurrentElement.toString();
                        }
                    }
                    else {
                        // Not in <active_task>
                        if(localName.equalsIgnoreCase("name")) {
                            mResult.name = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase("wu_name")) {
                            mResult.wu_name = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase("project_url")) {
                            mResult.project_url = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase("version_num")) {
                            mResult.version_num = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("ready_to_report")) {
                            mResult.ready_to_report = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("got_server_ack")) {
                            mResult.got_server_ack = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("final_cpu_time")) {
                            mResult.final_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("final_elapsed_time")) {
                            mResult.final_elapsed_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("state")) {
                            mResult.state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("report_deadline")) {
                            mResult.report_deadline = (long) Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("received_time")) {
                            mResult.received_time = (long) Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("estimated_cpu_time_remaining")) {
                            mResult.estimated_cpu_time_remaining = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("exit_status")) {
                            mResult.exit_status = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase("suspended_via_gui")) {
                            mResult.suspended_via_gui = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("project_suspended_via_gui")) {
                            mResult.project_suspended_via_gui = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase("resources")) {
                            mResult.resources = mCurrentElement.toString();
                        }
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ResultsParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
