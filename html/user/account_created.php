<?php

    include_once("util.inc");

    page_head("Account created");
    echo "Your account has been created,
        and an <b>account key</b> is being emailed to you.
        <p>
        <b>If you are already running the BOINC client:</b>
        <blockquote>
        Select the <b>Add Project</b> command in the BOINC client.
        Enter the project URL and your account key.
        </blockquote>
        <b>If you aren't running the BOINC client:</b>
        <blockquote>
        <a href=download.php>Download the BOINC client</a>.
        Install and run the client.
        Enter the project URL and your account key.
        </blockquote>
        <p>
        Your account initially has default <b>preferences</b>
        (limits on CPU, disk and network usage).
        <a href=prefs.php>View or edit these preferences</a>.
        ";

    page_tail();

?>
