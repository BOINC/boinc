<?php

require_once("../inc/util.inc");

$user = get_logged_in_user();
page_head("Weak account key");

$weak_auth = md5($user->authenticator.$user->passwd_hash);

echo "
    Your 'weak account key' lets you attach computers to your account
    on this project,
    without giving the ability to log in to your account
    or to change it in any way.
    This mechanism works only with projects that have
    upgraded their server software 7 Dec 2007 or later.
    <p>
    Your weak account key for this project is:
    <pre>
$weak_auth
    </pre>
    To use your weak account key on a given host,
    find or create the 'account file' for this project.
    This file has a name of the form <b>account_PROJECT_URL.xml</b>;
    for example the account file for SETI@home is
    <b>account_setiathome.berkeley.edu.xml</b>.
    <p>
    Create this file if needed.
    Set its contents to:
<pre>
&lt;account>
    &lt;master_url>PROJECT_URL&lt;/master_url>
    &lt;authenticator>WEAK_ACCOUNT_KEY&lt;/authenticator>
&lt;/account>
</pre>
    <p>
    Your weak account key is a function of your password.
    If you change your password,
    your weak account key changes,
    and your previous weak account key becomes invalid.
";

page_tail();
?>
