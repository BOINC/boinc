<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/countries.inc");

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
    "http://<input name=url size=50 value='$user->url'>"
);
row2_init("Country",
    "<select name=country>"
);
print_country_select($user->country);
echo "</select></td></tr>\n";
row2("Postal (ZIP) code",
    "<input name=postal_code size=20 value='$user->postal_code'>"
);

$x = "<textarea name=signature rows=4 cols=50>$user->signature</textarea>";
row2("Signature for message boards", $x);
row2("", "<input type=submit value='Update info'>");
end_table();
echo "</form>\n";
page_tail();

?>
