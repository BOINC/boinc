<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");

db_init();
$user = get_logged_in_user();

page_head("Account setup: done");
echo "
    <p>Your account setup is complete.</p>
    <p>Next you must download and install the BOINC software and
    tell your computer(s) to use this account.

    <p>When the BOINC software first runs, it will ask you for
    a <b>Project URL</b> and a <b>Account key</b>.
	<br>Copy and paste them from here:
    <p><ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: $user->authenticator
    </ul></p>

    You can now
    <a target = _blank href=http://boinc.berkeley.edu/download.php>download the BOINC software</a> to complete the ".PROJECT." installation.
    <p>Thanks for participating in ".PROJECT.".</p>
";

page_tail();

?>
