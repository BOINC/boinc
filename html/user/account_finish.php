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

// Users are taken here after creating an account via the Wizard.
// They've already entered an email address and password.
// Now get a name, country, and zip code

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');

$auth = get_str("auth");
$user = lookup_user_auth($auth);
if (!$user) {
    error_page("no such account");
}
page_head("Finish account setup");

echo "
    <form action=account_finish_action.php method=post>
";
start_table();
row2(
    tra("Name")."<br><span class=\"description\">".tra("Identifies you on our web site. Use your real name or a nickname.")."</span>",
    "<input name=\"name\" size=\"30\" value=\"$user->name\">"
);
row2_init(
    tra("Country")."<br><span class=\"description\">".tra("Select the country you want to represent, if any.")."</span>",
    "<select name=\"country\">"
);
print_country_select();
echo "</select></td></tr>\n";
row2(
    tra("Postal or ZIP Code")."<br><span class=\"description\">".tra("Optional")."</span>",
    "<input name=\"postal_code\" size=\"20\">"
);
row2("",
    "<input type=\"submit\" value=\"OK\">"
);
end_table();
echo "
    <input type=hidden name=auth value=$auth>
    </form>
";

page_tail();
