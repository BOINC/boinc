<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/countries.inc");

db_init();
$user = get_logged_in_user();
check_tokens($user->authenticator);

page_head(tra("Edit account information"));

echo "<form method=post action=edit_user_info_action.php>";
echo form_tokens($user->authenticator);
start_table();
row2(tra("Name %1 real name or nickname%2", "<br><font size=-2>", "</font>"),
    "<input name=user_name size=30 value='$user->name'>"
);
row2(tra("URL %1 of your web page; optional%2", "<br><font size=-2>", "</font>"),
    "http://<input name=url size=50 value='$user->url'>"
);
row2_init(tra("Country"),
    "<select name=country>"
);
print_country_select($user->country);
echo "</select></td></tr>\n";
row2(tra("Postal (ZIP) code %1 Optional%2", "<br><font size=-2>", "</font>"),
    "<input name=postal_code size=20 value='$user->postal_code'>"
);

row2("", "<input type=submit value='".tra("Update info")."'>");
end_table();
echo "</form>\n";
page_tail();

?>
