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

$url = parse_config(get_config(), "<master_url>");

$user = get_logged_in_user();
page_head(tra("Weak account key"));

$weak_auth = weak_auth($user);

// figure out the name of this project's account file.

// strip http://
$idx = strpos($url, '://');
if ($idx) {
	$url = substr($url, $idx+strlen('://'));
}

//convert invalid characters into underscores
for ($i=0; $i<strlen($url); $i++) {
	$c = $url[$i];
	if (!ctype_alnum($c) && $c != '.' && $c != '-' && $c != '_') {
		$url[$i] = '_';
	}
}
//remove trailing underscore(s)
$account_file = "account_" . rtrim($url, '_') . ".xml";

echo "<p>",tra("Your 'weak account key' lets you attach computers to your account on this project, without giving the ability to log in to your account or to change it in any way."), " ",
	tra("This mechanism works only with projects that have upgraded their server software 7 Dec 2007 or later."), "</p>",
	"<p>", tra("Your weak account key for this project is:"), "</p>",
	"<pre>$weak_auth</pre>",
	"<p>" , tra("To use your weak account key on a given host, find or create the 'account file' for this project. This file has a name of the form <b>account_PROJECT_URL.xml</b>. The account file for %1 is <b>%2</b>.", PROJECT, $account_file), "</p>",
	"<p>", tra("Create this file if needed. Set its contents to:"), "</p>",
	"<pre>",
	htmlspecialchars(
"<account>
	<master_url>PROJECT_URL</master_url>
	<authenticator>WEAK_ACCOUNT_KEY</authenticator>
</account>"),
	"</pre>",
	"<p>", tra("Your weak account key is a function of your password. If you change your password, your weak account key changes, and your previous weak account key becomes invalid."), "</p>"
;

page_tail();
?>
