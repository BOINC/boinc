<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/download.inc");

    init_session();
    db_init();

    page_head("Download BOINC software");
    if (!get_logged_in_user(false)) {
        echo "
            <font color=ff0000>
            <b>First-time ".PROJECT." participants</b>:
            <br>Don't download BOINC software now.
            <a href=create_account_form.php>Create an account</a>
            before you download.
            </font>
            <p>
            If you're a returning ".PROJECT." user:
        ";
    }
    echo "
        Select your computer type:<p>\n
    ";
    print_download_links();
    echo "
        <p>
        Instructions for installing and running BOINC are
        <a href=http://boinc.berkeley.edu/participate.php>here</a>.
        <p>
        If your computer is not one of the above types,
        you can
        <a href=http://boinc.berkeley.edu/anonymous_platform.php>download and compile the BOINC software yourself</a>.
        <p>
        BOINC can be customized for
        <a href=http://boinc.berkeley.edu/language.php>languages other than English</a>
        <p>
        <font size=-1>
        <a href=http://boinc.berkeley.edu>BOINC</a>
        is distributed computing software
        developed at the University of California by
        the SETI@home project.
        </font>
    ";
    page_tail();
?>
