<?php
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/db.inc");

db_init();

$code = get_str("code");
$userid = get_int('userid');
$user = lookup_user_id($userid);
if (!$user) {
    error_page("no user");
}

if (salted_key($user->authenticator) != $code) {
    error_page("bad code");
}

$result = mysql_query("update user set send_email=0 where id=$userid");

page_head("$email removed from mailing list");

echo "
No further emails will be sent to $user->email_addr.
To resume getting emails,
go <a href=".URL_BASE."prefs_edit_form.php?subset=project>here</a>
";

page_tail();
?>
