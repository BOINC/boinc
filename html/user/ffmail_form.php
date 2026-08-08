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

// TODO: - add a logging mechanism to see if people are using this.
//       - use display functions from util.inc
//       - give the user a language choice

require_once("../inc/util.inc");

// Spammers were abusing this feature,
// and AFAIK it was never used much.
// Screw it.

error_page("This feature is deprecated");

$user = get_logged_in_user();

page_head(tra("Tell your friends about %1", PROJECT));

$text = @file_get_contents('../ops/ffmail/text');
if (!$text) {
    $master_url = master_url();
    $text = "I'm using my computer to crunch numbers for a science project called ".PROJECT.".  The more computers participate, the more science gets done.  Would you like to join me?

To learn how, visit $master_url
";
}
$text .= "\n$user->name";

$subject = @file_get_contents('../ops/ffmail/subject');
if (!$subject) {
    $subject = "Join me at ".PROJECT;
}

echo tra("Use this form to send email messages to people you think might be interested in %1.", PROJECT);
echo "<p><form method=get action=ffmail_action.php>\n";

start_table();
row2("From:", "$user->name &lt;$user->email_addr>");
for ($i=0; $i<6; $i++) {
    row2(tra("To:"), "<input size=30 name=e$i>");
}
row2(tra("Subject"), "<input size=80 name=subject value=\"$subject\">");
row2(tra("Message"), "<textarea name=text rows=8 cols=50>$text</textarea>");
row2("", "<input class=\"btn btn-primary\" type=submit name=action value=".tra("Send").">");
end_table();
echo "</form>\n";
page_tail();
?>
