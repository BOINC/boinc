<?php

function xml_header() {
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
}

function xml_error($msg) {
    xml_header();
    echo "<error>$msg</error>\n";
}

function show_user_xml($user) {
    xml_header();
    echo "<user>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <teamid>$user->teamid</teamid>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
</user>
";
}

?>
