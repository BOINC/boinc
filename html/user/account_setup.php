<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/translation.inc");

    db_init();

    $user = get_logged_in_user();

    page_head(tr(AC_SETUP_TITLE));
    echo tr(AC_SETUP_USES_BOINC)."
        <br>".tr(AC_SETUP_DIVISION)."
        <p>".tr(AC_SETUP_ISFIRST)."
        <blockquote>
        <a href=account_setup_first.php><b>".tr(AC_SETUP_FIRST)."</b></a>
        <p>
        <a href=account_setup_nonfirst.php><b>".tr(AC_SETUP_NONFIRST)."</b></a>

        </blockquote>
    ";
    page_tail();

?>
