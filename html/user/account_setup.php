<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("prefs.inc");

    $authenticator = init_session();
    db_init();

    $user = get_user_from_auth($authenticator);
    require_login($user);

    page_head("Account configuration");
    echo "
        <h3>Account configuration</h3>
        ".PROJECT." uses the BOINC software system.
        <br>BOINC lets you divide your computer time between several
        distributed computing projects.
        <br>Are you currently participating in other projects that use BOINC?
        <p>
        <a href=account_setup_nonfirst.php>Yes</a>
        <p>
        <a href=account_setup_first.php>No</a>

    ";
    page_tail();

?>
