<?php

function platform_downloads($platform, $core_app) {
    $result = mysql_query("select * from app_version where platformid=$platform->id and appid=$core_app->id");

    if (!$result) return;
    echo "<tr><td><b>$platform->name</b></td></tr>\n";
    while ($app_version = mysql_fetch_object($result)) {
	$version = sprintf(
            "<a href=dl_%s_%s.php>BOINC core client,<b> version %s.</b></a>", 
            $platform->name, $app_version->version_num,
            $app_version->version_num
        );
	echo "<tr><td>&#160;&#160;&#160;&#160;&#160;&#160;$version</td></tr>\n";
    	//$app_version->md5_cksum";
    }
    mysql_free_result($result);
}

    require_once("db.inc");
    require_once("util.inc");

    db_init();
    page_head("Download the BOINC client");

    $result = mysql_query("select * from app where name='core_client'");
    $core_app = mysql_fetch_object($result);
    mysql_free_result($result);

    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        platform_downloads($platform, $core_app);
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
