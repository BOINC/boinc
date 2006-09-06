<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$auth = process_user_text($_GET["account_key"]);

$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
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

    echo "<am_get_info_reply>
    <success/>
    $ret
</am_get_info_reply>
";

?>
