<?php

require_once("../inc/util.inc");

page_head("Result state explanation");

    $field = $_GET["field"];

    if ($field == "result_server_state") {
        echo "
            The <b>server state</b> of a result has the following values:
            <dl>
            <dt><b>Inactive</b>
            <dd> The result is not ready to send
               (for example, because its input files are unavailable)
            <dt><b>Unsent</b>
            <dd> The result is ready to send,
               but hasn't been sent yet.
            <dt><b>In Progress</b>
            <dd> The result has been sent; waiting for completion.
            <dt><b>Over</b>
            <dd> The result has been sent and either
               it has timed out or a reply has been received.
            </dl>
        ";
    }

    if ($field == "result_outcome") {
        echo "
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
            <dt><b>Validate Error</b>
            <dd> The result was found to be incorrect or erroneous.
                 This can happen because your computer did not carry out the computations
                 correctly, or because an error was introduced when the results were returned
                 to the server.
            </dl>
        ";
    }

    if ($field == "result_client_state") {
        echo "
            The <b>client state</b> of a result is defined
            if its outcome is <b>client error</b>.
            It indicates the stage of processing
            at which the error occurred.
            It has the following values:
            <dl>
            <dt> <b>Downloading</b>
            <dd> The client couldn't download the application or input files.
            <dt> <b>Computing</b>
            <dd> An error occurred during computation.
            <dt> <b>Uploading</b>
            <dd> The client couldn't upload the output files.
            </dl>
        ";
    }

    if ($field == "validate_state") {
    }
page_tail();
?>
