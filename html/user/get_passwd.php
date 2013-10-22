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
require_once("../inc/user.inc");

check_get_args(array());

page_head(tra("Forgot your account info?"));

echo "<h3>"
    .tra("1) If you know your account's email address, and you can receive email there:")."</h3><p>"
    .tra("Enter the email address below, and click OK. You will be sent email instructions for resetting your password.");

$master_url = parse_config(get_config(), "<master_url>");
$x = strstr($master_url, "//");
$x = substr($x, 2);
$x = rtrim($x, "/");
$x = str_replace("/", "_", $x);
$account_file = "account_$x.xml";

start_table();
echo "<form method=post action=mail_passwd.php>\n";
row2(tra("Email address"),"<input type=\"text\" size=40 name=email_addr>");
row2("", "<input type=submit value=\"".tra("OK")."\">");
echo "</form>";
end_table();

echo "<p><h3>"
    .tra("2) If you forgot your account's email address, or you can't receive email there:")."</h3>"
    .tra("If you have run BOINC under this account, you can still access it. Here's how:")."

<ul>
<li> ".tra("Go to the BOINC data directory on your computer (its location is written to the Event Log at startup).")."
<li> ".tra("Find your account file for this project; it will be named <b>%1</b>.", $account_file)."
<li> ".tra("Open the file in a text editor like Notepad. You'll see something like")."
<pre>
&lt;account>
    &lt;master_url>$master_url&lt;/master_url>
    &lt;authenticator>8b8496fdd26df7dc0423ecd43c09a56b&lt;/authenticator>
    &lt;project_name>".PROJECT."&lt;/project_name>
    ...
&lt;/account>
</pre>

<li> ".tra("Select and Copy the string between %1 and %2 (%3 in the above example).", "&lt;authenticator>", "&lt;/authenticator>", "<b>8b8496fdd26df7dc0423ecd43c09a56b</b>")."

<li> ".tra("Paste the string into the field below, and click OK.")."
<li> ".tra("You will now be logged in to your account; update the email and password of your account.")."
</ul>
";
start_table();

echo "<form action=login_action.php method=post>\n";
row2(tra("Log in with authenticator"), "<input type=\"text\" name=authenticator size=40>");
row2(tra("Stay logged in on this computer"),
    "<input type=checkbox name=send_cookie checked>"
);
row2("", "<input type=submit value=\"".tra("OK")."\">");
echo "</form>";

end_table();


page_tail();

?>
