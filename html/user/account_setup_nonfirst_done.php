<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");
include_once("../inc/translation.inc");

db_init();
$user = get_logged_in_user();

page_head(tr(AC_DONE_TITLE));
echo "
    <h3>".tr(AC_DONE_TITLE)."</h3>

    ".tr(AC_DONE_COMPLETE)."
    <br>".tr(AC_DONE_MUST_USE)."
    <ul>
    <li><b>".tr(AC_DONE_WIN)."</b>
    ".tr(AC_DONE_WIN_TEXT)."
    <li><b>".tr(AC_DONE_UNIX)."</b>
    ".tr(AC_DONE_UNIX_TEXT)."
    </ul>
    ".tr(AC_DONE_ANYCASE)."
    <ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account Key: $user->authenticator
    </ul>
    ".tr(AC_DONE_INSTALLED)."                            
    <br>".tr(AC_DONE_THANKS)."
";

page_tail();

?>
