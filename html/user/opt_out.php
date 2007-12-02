<?php
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/boinc_db.inc");

$code = get_str("code");
$userid = get_int('userid');
$user = lookup_user_id($userid);
if (!$user) {
    error_page("no user");
}

if (salted_key($user->authenticator) != $code) {
    error_page("bad code");
}

$result = $user->update("send_email=0");

if ($result) {
    page_head("$email removed from mailing list");
    echo "
        No further emails will be sent to $user->email_addr.
        To resume getting emails,
        go <a href=".URL_BASE."prefs_edit.php?subset=project>here</a>
    ";
    page_tail();
}
error_page("database error");

?>
