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

public class ResultsParser extends BaseParser {
    static final String RESULT_TAG = "result";

    private List<Result> mResults = new ArrayList<>();
    private Result mResult = null;
    private boolean mInActiveTask = false;

    public List<Result> getResults() {
        return mResults;
    }

    /**
     * Parse the RPC result (results) and generate vector of results info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return vector of results info
     */
    public static List<Result> parse(String rpcResult) {
        try {
            ResultsParser parser = new ResultsParser();
            Xml.parse(rpcResult, parser);
            return parser.getResults();
        }
        catch(SAXException e) {
            return Collections.emptyList();
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(RESULT_TAG)) {
            mResult = new Result();
        }
        else if(localName.equalsIgnoreCase(Result.Fields.active_task)) {
            mInActiveTask = true;
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
            if(mResult != null) {
                // We are inside <result>
                if(localName.equalsIgnoreCase(RESULT_TAG)) {
                    // Closing tag of <result> - add to vector and be ready for
                    // next one
                    if(!mResult.name.isEmpty()) {
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
                        if(localName.equalsIgnoreCase(Result.Fields.active_task)) {
                            // Closing of <active_task>
                            mResult.active_task = true;
                            mInActiveTask = false;
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.active_task_state)) {
                            mResult.active_task_state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.app_version_num)) {
                            mResult.app_version_num = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.scheduler_state)) {
                            mResult.scheduler_state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.checkpoint_cpu_time)) {
                            mResult.checkpoint_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.current_cpu_time)) {
                            mResult.current_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.fraction_done)) {
                            mResult.fraction_done = Float.parseFloat(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.elapsed_time)) {
                            mResult.elapsed_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.swap_size)) {
                            mResult.swap_size = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.working_set_size_smoothed)) {
                            mResult.working_set_size_smoothed = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.estimated_cpu_time_remaining)) {
                            mResult.estimated_cpu_time_remaining = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.supports_graphics)) {
                            mResult.supports_graphics = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.graphics_mode_acked)) {
                            mResult.graphics_mode_acked = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.too_large)) {
                            mResult.too_large = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.needs_shmem)) {
                            mResult.needs_shmem = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.edf_scheduled)) {
                            mResult.edf_scheduled = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.pid)) {
                            mResult.pid = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.slot)) {
                            mResult.slot = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.graphics_exec_path)) {
                            mResult.graphics_exec_path = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.slot_path)) {
                            mResult.slot_path = mCurrentElement.toString();
                        }
                    }
                    else {
                        // Not in <active_task>
                        if(localName.equalsIgnoreCase(RPCCommonTags.NAME)) {
                            mResult.name = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.wu_name)) {
                            mResult.wu_name = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase(RPCCommonTags.PROJECT_URL)) {
                            mResult.project_url = mCurrentElement.toString();
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.version_num)) {
                            mResult.version_num = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.ready_to_report)) {
                            mResult.ready_to_report = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.got_server_ack)) {
                            mResult.got_server_ack = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.final_cpu_time)) {
                            mResult.final_cpu_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.final_elapsed_time)) {
                            mResult.final_elapsed_time = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.state)) {
                            mResult.state = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.report_deadline)) {
                            mResult.report_deadline = (long) Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.received_time)) {
                            mResult.received_time = (long) Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.estimated_cpu_time_remaining)) {
                            mResult.estimated_cpu_time_remaining = Double.parseDouble(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.exit_status)) {
                            mResult.exit_status = Integer.parseInt(mCurrentElement.toString());
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.suspended_via_gui)) {
                            mResult.suspended_via_gui = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.project_suspended_via_gui)) {
                            mResult.project_suspended_via_gui = !mCurrentElement.toString().equals("0");
                        }
                        else if(localName.equalsIgnoreCase(Result.Fields.resources)) {
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
