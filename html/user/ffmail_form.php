<?php

require_once("../inc/util.inc");

db_init();
$user = get_logged_in_user();

if (!@file_get_contents('../ops/ffmail/subject')) {
    error_page(
        'This project hasn\'t created an email message -
        please notify its administrators'
    );
}

page_head("Tell your friends about ".PROJECT);

echo "
<table width=600><tr><td>
Help us by telling your friends, family and coworkers about ".PROJECT.".
<p>
Fill in this form with the names and email addresses
of people you think might be interested in ".PROJECT.".
We'll send them an email in your name,
and you can add your own message if you like.
<form method=get action=ffmail_action.php>
<table cellpadding=4>
<tr><td class=heading>Your name:</td><td class=heading>Your email address:</td></tr>
<tr><td><b>$user->name</b></td><td><b>$user->email_addr</b></td></tr>

<input type=hidden name=uname value=\"$user->name\">
<input type=hidden name=uemail value=\"$user->email_addr\">

<tr><td class=heading>Friend's name:</td><td class=heading>Friend's email address:</td></tr>
";
for ($i=0; $i<5; $i++) {
    echo "
        <tr><td><input size=30 name=n$i></td><td><input size=30 name=e$i></tr>
    ";
}
echo "
<tr><td class=heading colspan=2>Additional message (optional)</td></tr>
<tr><td colspan=2><textarea name=comment rows=8 cols=50></textarea></td></tr>
<tr><td align=center><input type=submit name=action value=Preview></td>
    <td align=center><input type=submit name=action value=Send></td>
</tr>
</table>
</form>
</td></tr></table>
";
page_tail();
?>
