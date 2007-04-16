<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");

    db_init();

    $user = get_logged_in_user();

    page_head("Account setup", $user);
    echo "
        <h3>Account setup</h3>
        ".PROJECT." uses the BOINC software system.
        <br>BOINC lets you divide your computer time between several
        distributed computing projects.
        <br>Is this your first BOINC project?
        <blockquote>
        <a href=account_setup_first.php><b>This is my first BOINC project</b></a>
        <p>
        <a href=account_setup_nonfirst.php><b>I'm currently participating in another BOINC project</b></a>

        </blockquote>
    ";
    page_tail();

?>
