<?php

require_once("../inc/util.inc");


$field = $_GET["field"];

switch($field) {
case "result_server_state":
    page_head("Server states");
    echo "
        <p>
        A result's <b>server state</b> keeps track of
        whether the result has been sent to a host,
        and if so whether the host has finished it.
        Possible values are:
        <p>
    ";
    start_table();
    row2_plain("<b>Inactive</b>",
        "The result is not ready to send
       (for example, because its input files are unavailable)"
    );
    row2_plain("<b>Unsent</b>",
        "The result is ready to send, but hasn't been sent yet."
    );
    row2_plain("<b>In Progress</b>",
        "The result has been sent; waiting for completion."
    );
    row2_plain("<b>Over</b>",
        "The result has been sent to a host and either
       it has timed out or the host has reported its completion."
    );
    break;

case "result_outcome":
    page_head("Outcomes");
    echo "
        <p>
        A result's <b>outcome</b> is defined if
        its server state is <b>over</b>.
        Possible values are:
        <p>
    ";
    start_table();
    row2_plain("<b>Unknown</b>",
        "The result was sent to a client, but the client has not
        yet completed the work and reported the outcome."
    );
    row2_plain("<b>Success</b>",
        "A client completed the result successfully."
    );
    row2_plain("<b>Couldn't send</b>",
        "The server wasn't able to send the result to a client
        (perhaps because its resource requirements were too large)"
    );
    row2_plain("<b>Client error</b>",
        "The result was sent to a client and an error occurred."
    );
    row2_plain("<b>No reply</b>",
        "The result was sent to a client
        and no reply was received within the time limit."
    );
    row2_plain("<b>Didn't need</b>",
        "The result wasn't sent to a client because
        enough other results were returned for this work unit."
    );
    row2_plain("<b>Validate error</b>",
        "The result was reported but could not be validated,
        typically because the output files were lost on the server."
    );
    break;

case "result_client_state":
    page_head("Client states");
    echo "<p>A result's <b>client state</b>
        indicates the stage of processing at which
        an error occurred.
        <p>
    ";
    start_table();
    row2_plain("<b>New</b>",
        "The client has not yet completed the work.  Since the
        processing is not over, the the final client state at
        outcome is not yet known."
    );
    row2_plain("<b>Done</b>",
        "No error occurred."
    );
    row2_plain("<b>Downloading</b>",
        "The client couldn't download the application or input files."
    );
    row2_plain("<b>Computing</b>",
        "An error occurred during computation."
    );
    row2_plain("<b>Uploading</b>",
        "The client couldn't upload the output files."
    );
    break;

case "result_time":
    page_head("Time reported and deadline");
    echo "
        <p>
        A result's <b>Time reported or deadline</b> field depends
        on whether the result has been reported yet:
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
