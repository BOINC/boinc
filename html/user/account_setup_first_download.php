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

page_head("Download BOINC software");

echo "
    Your account is now configured according to your preferences
    (you can return to our web site to change your preferences later).
    <p>
    The next step is to download the BOINC software.
    Click on the kind of computer that you have to download
    the appropriate version of BOINC.
    <p>
";

print_download_links();

echo "
    <p>
    After the download is finished:
    <ul>
    <li><b>Windows users</b>:
        double-click on the downloaded file.
        This will install BOINC on your computer.
    <li><b>Macintosh OS/X users</b>: xxx
    <li><b>Unix and Linux users</b>:
        Use gunzip and tar to extract BOINC.
    </ul>

    When the BOINC software first runs,
    it will ask you for a project URL and an account key.
    Copy and paste the following:

    <ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: $user->authenticator
    </ul>

    That's it - you're done.
    Thanks for participating in ".PROJECT.".
    Visit our main page for more information.
";

page_tail();
?>
