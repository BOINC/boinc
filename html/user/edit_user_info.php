<?php

require_once("edit.inc");
require_once("util.inc");

db_init();
$authenticator = init_session();
$user = get_user_from_auth($authenticator);
require_login($user);

page_head("Edit user information");
echo "<form method=post action=edit_action.php>\n
    ".TABLE2."\n
    <tr><td align=right><b>User name</b></td>\n
    <td><input name=my_name type=text size=30 value='$user->name'></td></tr>
    <tr><td align=right><b>Email address</b></td>\n
    <td><input name=my_email type=text size=50 value='$user->email_addr'></td></tr>
    <tr><td align=right><b>Country:</b></font></td>
    <td><select name=my_country>";
print_country_select($user->country);
echo "</select></td></tr>
    <tr><td align=right><b>Postal (ZIP) code</b></td>
    <td><input name=my_zip type=text size=20 value='$user->postal_code'></td></tr>
    <tr><td><br></td><td><input type=submit value='OK'>\n
    </table>\n
    </form>";
page_tail();

?>
