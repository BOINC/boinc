<?php
require_once("docutil.php");

page_head("Server status XML export");

echo "
BOINC-based projects offer the following XML export
at <code>URL/server_status.php</code>.
These are generally updated every 10 minutes or so -
do not poll more often than that.
These can be used to make web sites showing
the server status of multiple BOINC projects.
<p>
";
echo html_text("
<server_status>
    <update_time>1128535206</update_time>
    <daemon_status>
        <daemon>
            <host>jocelyn</host>
            <command>BOINC database</command>
            <status>running</status>
        </daemon>
        <daemon>
            <host>castelli</host>
            <command>master science database</command>
            <status>running</status>
        </daemon>
        <daemon>
            <host>klaatu</host>
            <command>data-driven web pages</command>
            <status>disabled</status>
        </daemon>
        <daemon>
            <host>galileo</host>
            <command>feeder</command>
            <status>not running</status>
        </daemon>
    </daemon_status>
	<database_file_states>
        <results_ready_to_send>614830</results_ready_to_send>
        <results_in_progress>1208959</results_in_progress>
        <workunits_waiting_for_validation>8</workunits_waiting_for_validation>
        <workunits_waiting_for_assimilation>2</workunits_waiting_for_assimilation>
        <workunits_waiting_for_deletion>4</workunits_waiting_for_deletion>
        <results_waiting_for_deletion>15</results_waiting_for_deletion>
        <transitioner_backlog_hours>0.00083333334</transitioner_backlog_hours>
    </database_file_states>
</server_status>");

page_tail();

?>
