<?php

include_once("db.inc");
include_once("util.inc");
include_once("prefs.inc");

$authenticator = init_session();
db_init();

$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}
page_head("Account setup: done", $user);
echo "
    <h3>Account setup: done</h3>

    Your account setup is complete.
    <br>Next you must tell your computer(s) to use this account.
    <ul>
    <li><b>Windows users:</b>
        Open the BOINC application window by
        double-clicking the BOINC system tray icon.
        Choose the 'Add Project' item in the Projects menu.
        It will ask you for a project URL and an account key.
    <li><b>Macintosh users:</b>
        Open the BOINC application window by XXX.
        Choose the 'Add Project' item in the Projects menu.
        It will ask you for a project URL and an account key.
    <li><b>Unix and Linux users:</b>
        Quit the BOINC client.
        Then run the BOINC client program with the -add_project option.
        It will ask you for a project URL and an account key.
    </ul>
    In each case copy and paste the following:
    <ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: $user->authenticator
    </ul>
                            
    This completes the ".PROJECT." installation.
    <br>Thanks for participating in ".PROJECT.".
    <br>Visit our <a href=index.php>main page</a> for more information.
";

page_tail();

?>
