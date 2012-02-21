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

// TODO: - find out if this is used (add some logging mechanism or so)
//       - if used rewrite code to use display functions from util.inc
//       - if used find a way to give the user a language choice (projects must translate messages themself)

require_once("../inc/util.inc");

check_get_args(array());

db_init();
$user = get_logged_in_user();

if (!@file_get_contents('../ops/ffmail/subject')) {
    error_page(tra("This project hasn\'t created an email message - please notify its administrators"));
}

page_head(tra("Tell your friends about %1", PROJECT));

echo "
<table width=600><tr><td>
".tra("Help us by telling your friends, family and coworkers about %1", PROJECT)."
<p>
".tra("Fill in this form with the names and email addresses of people you think might be interested in %1. We'll send them an email in your name, and you can add your own message if you like.", PROJECT)."
<form method=get action=ffmail_action.php>
<table cellpadding=4>
<tr><td class=heading>".tra("Your name:")."</td><td class=heading>".tra("Your email address:")."</td></tr>
<tr><td><b>$user->name</b></td><td><b>$user->email_addr</b></td></tr>

<input type=hidden name=uname value=\"$user->name\">
<input type=hidden name=uemail value=\"$user->email_addr\">

<tr><td class=heading>".tra("Friend's name:")."</td><td class=heading>".tra("Friend's email address:")."</td></tr>
";
for ($i=0; $i<5; $i++) {
    echo "
        <tr><td><input size=30 name=n$i></td><td><input size=30 name=e$i></tr>
    ";
}
echo "
<tr><td class=heading colspan=2>".tra("Additional message (optional)")."</td></tr>
<tr><td colspan=2><textarea name=comment rows=8 cols=50></textarea></td></tr>
<tr><td align=center><input type=submit name=action value=".tra("Preview")."></td>
    <td align=center><input type=submit name=action value=".tra("Send")."></td>
</tr>
</table>
</form>
</td></tr></table>
";
page_tail();
?>
