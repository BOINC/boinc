<?php

require_once("db.inc");
require_once("util.inc");

function platform_downloads($platform, $core_app) {
    $result = mysql_query("select * from app_version where platformid=$platform->id and appid=$core_app->id");

    if (!$result) return;

    $found = false;

    $download_url = parse_config("<download_url>");
    echo "<tr><td><b>$platform->name</b></td></tr>\n";
    while ($app_version = mysql_fetch_object($result)) {
        $filename = parse_element($app_version->xml_doc, "<name>");
        if (!$filename) { echo "CAN'T FIND FILENAMEn $app_version->xml_doc\n";}
	$version = sprintf(
            "<a href=$download_url/$filename>BOINC core client, <b>version %s %.2f</b></a>", 
            $platform->name,
            $app_version->version_num/100
        );
	echo "<tr><td>$version</td></tr>\n";
    	//$app_version->md5_cksum";
        $found = true;
    }
    if (!$found)
        echo "<tr><td>No version found</td></tr>";

    mysql_free_result($result);
}


    $authenticator = init_session();
    db_init();
    page_head("Download the BOINC client");

    start_table();

    $result = mysql_query("select * from app where name='core client'");
    $core_app = mysql_fetch_object($result);
    mysql_free_result($result);

    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        platform_downloads($platform, $core_app);
    }
    mysql_free_result($result);
    echo "</table>\n<p>\n";
    page_tail();
?>
