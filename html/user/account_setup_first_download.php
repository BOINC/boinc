<?php

require_once("db.inc");
require_once("util.inc");
require_once("download.inc");

db_init();
$authenticator = init_session();
$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}

page_head("Download BOINC software", $user);

echo "
    Your account setup is complete.
    <br>Next you must download the BOINC software.
    <br>Click on the type of computer that you have:
    <p>
";

print_download_links();

echo "
    <p>
    After the download is finished:
    <ul>
    <li><b>Windows users</b>:
        open the downloaded file.
        This will install BOINC on your computer.
    <li><b>Macintosh OS/X users</b>: BOINC will install itself automatically.
    <li><b>Unix and Linux users</b>:
        Use gunzip and tar to extract BOINC.
    </ul>

    When BOINC first runs,
    it will ask you for a project URL and an account key.
    Copy and paste the following:

    <ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: <b>$user->authenticator</b>
    </ul>

    This completes the ".PROJECT." installation.
    <br>Thanks for participating in ".PROJECT.".
    <br>Visit our <a href=index.php>main page</a> for more information.
";

page_tail();
?>
