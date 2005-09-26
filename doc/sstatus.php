<?php
require_once("docutil.php");

page_head("Server status");

echo "
Each project should export its server status in two forms:
<ul>
<li> <b>URL/server_status.php</b>: human-readable (web page).
<li> <b>URL/server_status.php?xml=1</b>: XML format, as follows:
".html_text("
<server_status>
    <update_time>1127772607</update_time>
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
            <host>kryten</host>
            <command>sah_validate4</command>
            <status>running</status>
        </daemon>
        ...
    </daemon_status>
	<database_file_states>
        <results_ready_to_send>563389</results_ready_to_send>
        <results_in_progress>1198237</results_in_progress>
        <workunits_waiting_for_validation>19</workunits_waiting_for_validation>
        <workunits_waiting_for_assimilation>16</workunits_waiting_for_assimilation>
        <workunits_waiting_for_deletion>0</workunits_waiting_for_deletion>
        <results_waiting_for_deletion>0</results_waiting_for_deletion>
        <transitioner_backlog_hours>-0.0002777</transitioner_backlog_hours>
    </database_file_states>
</server_status>
")."
</ul>

<p>
There are two ways to do this:

<ol>
<li> Copy the file server_status.php from html/ops to html/user.
This works fine as long as you don't need any customization,
and the DB queries in the page only take a few seconds
(the page is cached, so they are done infrequently).
<li> Write a periodic script that generates the 2 pages as files,
and put a script in user/server_status.php
that echoes one file or the other.
</ol>
";

page_tail();

?>
