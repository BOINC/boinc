<?php

require_once("edit.inc");
require_once("util.inc");

db_init();
$authenticator = init_session();
$user = get_user_from_auth($authenticator);
require_login($user);

page_head("Edit email address");

if (is_valid_email_addr($user->email_addr)) {
    $email_text = $user->email_addr;
} else {
    $email_text = "Verification pending";
}

echo "<form method=post action=edit_email_action.php>\n";
start_table();
row1("Edit email address");
row2("Email address
    <br><font size=-1>Must be a valid address of the form 'name@domain'</font>",
    "<input name=email_addr size=50 value='$email_text'>"
);
row2("", "<input type=submit value='Update email address'>");
end_table();
echo "</form>\n";
page_tail();

?>
