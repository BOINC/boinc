<?php

require_once("db.inc");
require_once("util.inc");

db_init();
$user = get_logged_in_user();

page_head("Edit user information", $user);

echo "<form method=post action=edit_user_info_action.php>";
start_table();
row1("Edit account info");
row2("Name",
    "<input name=user_name size=30 value='$user->name'>"
);
row2("URL",
    "http://<input name=url size=30 value='$user->url'>"
);
row2_init("Country",
    "<select name=country>"
);
print_country_select($user->country);
echo "</select></td></tr>\n";
row2("Postal (ZIP) code",
    "<input name=postal_code size=20 value='$user->postal_code'>"
);
row2("", "<input type=submit value='Update info'>");
end_table();
echo "</form>\n";
page_tail();

?>
