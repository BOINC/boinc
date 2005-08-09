<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();
$user = get_logged_in_user();

page_head("Change password");

echo "<form method=post action=edit_passwd_action.php>\n";
start_table();
row1("Change password");
row2("New password", "<input type=password name=passwd size=30>");
row2("New password, again", "<input type=password name=passwd2 size=30>");
row2("", "<input type=submit value='Change password'>");
end_table();
echo "</form>\n";
page_tail();

?>
