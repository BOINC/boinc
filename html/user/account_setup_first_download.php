<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/download.inc");

db_init();
$user = get_logged_in_user();

page_head("Account setup complete");

echo "
    <p>
    Next step:
    <a target=_new href=http://boinc.berkeley.edu/download.php>download BOINC software</a>
    and follow the directions to install and run it.

    <p>
    When BOINC first runs,
    it will ask you for a project URL and an account key.
    Copy and paste the following:

    <ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: <b>$user->authenticator</b>
    </ul>

    <br>Thanks for participating in ".PROJECT.".
";

page_tail();
?>
