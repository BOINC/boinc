<?php

require_once("db.inc");
require_once("util.inc");


    $authenticator = init_session();
    db_init();

    page_head("Download the BOINC client");
    echo "
        To participate in ".PROJECT." you must
        install BOINC* software on your computer.
        <p>
        If BOINC is already installed on your computer,
        <a href=prefs_edit_form.php>finish setting up your account</a>.
        <p>
        Otherwise <a href=download.php>download BOINC</a>.
        <p>
        <font size=-1>
        *BOINC is distributed computing software
        developed at the University of California by
        the SETI@home project.
        </font>
    ";
    print_download_links();
    page_tail();
?>
