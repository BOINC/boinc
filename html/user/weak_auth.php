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

require_once("../inc/util.inc");
require_once("../inc/user.inc");

$user = get_logged_in_user();

$weak_auth = weak_auth($user);

// figure out the name of this project's account file.

// strip http://
//
$master_url = master_url();
$idx = strpos($master_url, '://');
if ($idx) {
    $url = substr($master_url, $idx+strlen('://'));
} else {
    $url = $master_url;
}

// convert invalid characters into underscores
//
for ($i=0; $i<strlen($url); $i++) {
    $c = $url[$i];
    if (!ctype_alnum($c) && $c != '.' && $c != '-' && $c != '_') {
        $url[$i] = '_';
    }
}

//remove trailing underscore(s)
//
$account_file = "account_" . rtrim($url, '_') . ".xml";

page_head(tra("Account keys"));
text_start();
echo "<table><tr><td>",
    tra("You can access your account either by using your email address and password,
    or by using an assigned 'account key'.
    Your account key is:"),
    "<p><pre>$user->authenticator</pre>
    <p>",
    tra("This key can be used to:"),
    "<ul>
    <li><a href=get_passwd.php>",tra("log in to your account on the web"),"</a>;
    <li>",
        tra("attach a computer to your account without using the BOINC Manager.
       To do so, install BOINC,
       create a file named %1 in the BOINC
       data directory, and set its contents to:","<b>$account_file</b>"),"
    <p><pre>",
    htmlspecialchars(
"<account>
    <master_url>".$master_url."</master_url>
    <authenticator>".$weak_auth."</authenticator>
</account>"),
    "</pre>
    </ul>
    <h2>", tra("Weak account key"), "</h2>",
    tra("Your 'weak account key' can be used to attach computers to your account
    as described above, but cannot be used to log in to your account or change it in any way.
    If you want to attach untrusted or insecure computers to your account,
    do so using your weak account key.
    Your weak account key is:"),"
    <p><pre>$weak_auth</pre><p>
    ",
    tra("The key depends on your account's email address and password.  If you change either of these, the weak account key will change."),"
    </td></tr></table>"
;
text_end();
page_tail();
?>
