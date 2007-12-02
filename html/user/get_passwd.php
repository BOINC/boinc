<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");

page_head("Forgot your password?");

echo "
<h3>Get email instructions for setting your password</h3>
";

start_table();
echo "<form method=post action=mail_passwd.php>\n";
row2("Email address","<input size=40 name=email_addr>");
row2("", "<input type=submit value=OK>");
echo "</form>";
end_table();

echo "
<p>
<h3>If your account's email address is no longer valid</h3>

If you have run BOINC under the account,
you can access the account
even if you don't know the password and the email
address is no longer valid.  Here's how:

<ul>
<li> Go to the BOINC directory on your computer
(on Windows this is usually <b>C:\\Program Files\BOINC</b>.
<li> Find your account file for this project;
this will have a name like <b>account_lhcathome.cern.ch.xml</b>
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
