<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/prefs.inc");
include_once("../inc/translation.inc");

db_init();
$user = get_logged_in_user();

page_head(tr(AC_DONE_TITLE));
echo "
    <p>".tr(AC_DONE_COMPLETE)."</p>
    <p>".tr(AC_DONE_BOINC)."

    <p>".tr(AC_DONE_URL)."
	<br>".tr(AC_DONE_COPYPASTE)."
    <p><ul>
    <li>Project URL: <b>".MASTER_URL."</b>
    <li>Account key: $user->authenticator
    </ul></p>

    ";
    printf(tr(AC_DONE_GETDOWNLOAD), "<a target = _blank href=http://boinc.berkeley.edu/download.php>".tr(AC_DONE_DL_BOINC)."</a>");
echo "<p>".tr(AC_DONE_THANKS)."</p>";

page_tail();

?>
