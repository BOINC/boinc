<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/util.inc");


$field = $_GET["field"];

switch($field) {
case "result_server_state":
    page_head("Server states");
    echo "
        <p>
        A tasks's <b>server state</b> indicates
        whether the task has been sent to a computer,
        and if so whether the computer has finished it.
        Possible values are:
        <p>
    ";
    start_table();
    row2_plain("<b>Inactive</b>",
        "The task is not ready to send
        (for example, because its input files are unavailable)"
    );
    row2_plain("<b>Unsent</b>",
        "The task is ready to send, but hasn't been sent yet."
    );
    row2_plain("<b>In Progress</b>",
        "The task has been sent; waiting for completion."
    );
    row2_plain("<b>Over</b>",
        "The task has been sent to a computer and either
        it has timed out or the computer has reported its completion."
    );
    break;

case "result_outcome":
    page_head("Outcomes");
    echo "
        <p>
        A tasks's <b>outcome</b> is defined if its server state is <b>over</b>.
        Possible values are:
        <p>
    ";
    start_table();
    row2_plain("<b>Unknown</b>",
        "The task was sent to a computer, but the computer has not
        yet completed the work and reported the outcome."
    );
    row2_plain("<b>Success</b>",
        "A computer completed and reported the task successfully."
    );
    row2_plain("<b>Couldn't send</b>",
        "The server wasn't able to send the task to a computer
        (perhaps because its resource requirements were too large)"
    );
    row2_plain("<b>Client error</b>",
        "The task was sent to a computer and an error occurred."
    );
    row2_plain("<b>No reply</b>",
        "The task was sent to a computer
        and no reply was received within the time limit."
    );
    row2_plain("<b>Didn't need</b>",
        "The task wasn't sent to a computer because
        enough other tasks were completed for this workunit."
    );
    row2_plain("<b>Validate error</b>",
        "The task was reported but could not be validated,
        typically because the output files were lost on the server."
    );
    break;

case "result_client_state":
    page_head("Client states");
    echo "<p>A result's <b>client state</b>
        indicates the stage of processing at which an error occurred.
        <p>
    ";
    start_table();
    row2_plain("<b>New</b>",
        "The computer has not yet completed the task."
    );
    row2_plain("<b>Done</b>",
        "The computer completed the task successfully."
    );
    row2_plain("<b>Downloading</b>",
        "The computer couldn't download the application or input files."
    );
    row2_plain("<b>Computing</b>",
        "An error occurred during computation."
    );
    row2_plain("<b>Uploading</b>",
        "The computer couldn't upload the output files."
    );
    break;

case "result_time":
    page_head("Time reported and deadline");
    echo "
        <p>
        A task's <b>Time reported or deadline</b> field depends
        on whether the task has been reported yet:
        <p>
    ";
    start_table();
    row2("Already reported", "The date/time it was reported");
    row2("Not reported yet, deadline in the future",
        "Deadline, shown in green."
    );
    row2("Not reported yet, deadline in the past",
        "Deadline, shown in red."
    );
    break;

default:
    page_head("Unknown field");
}

end_table();
page_tail();
?>
