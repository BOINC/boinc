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

page_head("Forgot your account info?");

echo "
<h3>1) If you know your account's email address,
    and you can receive email there:</h3>
<p>
Enter the email address below, and click OK.
You will be sent email instructions for resetting your password.
";

start_table();
echo "<form method=post action=mail_passwd.php>\n";
row2("Email address","<input size=40 name=email_addr>");
row2("", "<input type=submit value=OK>");
echo "</form>";
end_table();

echo "
<p>
<h3>2) If you forgot your account's email address,
    or you can't receive email there:</h3>

If you have run BOINC under the account,
you can still access it.
Here's how:

<ul>
<li> Go to the BOINC data directory on your computer
(on Windows this is usually <b>C:\\Documents and Settings\All Users\Application Data\BOINC</b> or <b>C:\\Program Files\BOINC</b>.
<li> Find your account file for this project;
it will have a name like <b>account_lhcathome.cern.ch.xml</b>
(where the project URL is <b>http://lhcathome.cern.ch</b>).
<li> Open the file in a text editor like Notepad.
You'll see something like
<pre>
&lt;account>
    &lt;master_url>http://lhcathome.cern.ch/&lt;/master_url>
    &lt;authenticator>8b8496fdd26df7dc0423ecd43c09a56b&lt;/authenticator>
    &lt;project_name>lhcathome&lt;/project_name>
    ...
&lt;/account>
</pre>

<li> Select and Copy the string between &lt;authenticator>
and &lt;/authenticator>
(<b>8b8496fdd26df7dc0423ecd43c09a56b</b> in the above example).

<li> Paste the string into the field below, and click OK.
<li> You will now be logged in to your account;
update the email and password of your account.
</ul>
";
start_table();

echo "<form action=login_action.php method=post>\n";
row2("Log in with authenticator", "<input name=authenticator size=40>");
row2("Stay logged in on this computer",
    "<input type=checkbox name=send_cookie checked>"
);
row2("", "<input type=submit value=OK>");
echo "</form>";

end_table();


page_tail();

?>
