<?php

require_once("../inc/util.inc");


$field = $_GET["field"];

switch($field) {
case "result_server_state":
    page_head("Server state");
    echo "
        <p>
        The <b>server state</b> of a result is one of:
        <dl>
        <dt><b>Inactive</b>
        <dd> The result is not ready to send
           (for example, because its input files are unavailable)
        <dt><b>Unsent</b>
        <dd> The result is ready to send, but hasn't been sent yet.
        <dt><b>In Progress</b>
        <dd> The result has been sent; waiting for completion.
        <dt><b>Over</b>
        <dd> The result has been sent to a host and either
           it has timed out or the host has reported its completion.
        </dl>
    ";
    break;

case "result_outcome":
    page_head("Outcome");
    echo "
        <p>
        The <b>outcome</b> of a result is defined if
        its server state is <b>over</b>.
        <br>
        It has the following values:
        <dl>
        <dt> <b>Success</b>
        <dd> A client completed the result successfully.
        <dt> <b>Couldn't send</b>
        <dd> The server wasn't able to send the result to a client
            (perhaps because its resource requirements were too large)
        <dt> <b>Client error</b>
        <dd> The result was sent to a client and an error occurred.
        <dt> <b>No reply</b>
        <dd> The result was sent to a client
            and no reply was received within the time limit.
        <dt> <b>Didn't need</b>
        <dd> The result wasn't sent to a client because
            enough other results were returned for this work unit.
        <dt><b>Validate error</b>
        <dd> The result was reported but could not be validated,
            typically because the output files were lost on the server.
        </dl>
    ";
    break;

case "result_client_state":
    page_head("Client state");
    echo "
        <p>
        The <b>client state</b> of a result is either <b>Done</b>
        for a successful result, or it has one
        of the following values if the outcome is <b>client error</b>.

        <dl>
        <dt> <b>Downloading</b>
        <dd> The client couldn't download the application or input files.
        <dt> <b>Computing</b>
        <dd> An error occurred during computation.
        <dt> <b>Uploading</b>
        <dd> The client couldn't upload the output files.
        </dl>
        These states indicate the stage of processing at which
        the client error occurred.
    ";
    break;

case "result_time":
    page_head("Time reported and deadline");
    echo "
        <p>
        The <b>Time reported or deadline</b> field shows:
        <ul>
        <li> If the result has been reported, the time it was reported.
        <li> If the result has not been reported yet,
            and its deadline is in the future,
            the deadline is shown in green.
        <li> If the result has not been reported yet,
            and its deadline is in the past,
            the deadline is shown in red.
        </ul>
    ";
    break;

default:
    page_head("Unknown field");
}

page_tail();
?>
