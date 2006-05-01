<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

db_init();

$auth = process_user_text($_GET["account_key"]);

xml_header();

function reply($x) {
    echo "<am_get_info_reply>
    $x
</am_get_info_reply>
";
    exit();
}

function error($x) {
    reply("<error>$x</error>");
}

function success($x) {
    reply("<success/>\n$x");
}

$user = lookup_user_auth($auth);
if (!$user) {
    error("no such user");
}

$name = urlencode($user->name);
$country = urlencode($user->country);
$postal_code = urlencode($user->postal_code);
$url = urlencode($user->url);

$ret = "<id>$user->id</id>
<name>$name</name>
<country>$country</country>
<postal_code>$postal_code</postal_code>
<global_prefs>
$user->global_prefs
</global_prefs>
<project_prefs>
$user->project_prefs
</project_prefs>
<url>$url</url>
<send_email>$user->send_email</send_email>
<show_hosts>$user->show_hosts</show_hosts>
<teamid>$user->teamid</teamid>
<venue>$user->venue</venue>";

if ($user->teamid) {
    $team = lookup_team($user->teamid);
    if ($team->userid == $user->id) {
        $ret = $ret . "<teamfounder/>\n";
    }
}

success($ret);

?>
