<?php

function platform_downloads($platform, $core_app) {
    $result = mysql_query("select * from app_version where platformid=$platform->id and appid=$core_app->id");

    echo "Platform $platform->name:\n";
    while ($app_version = mysql_fetch_object($result)) {
        echo "<br>version $app_version->version_num $app_version->md5_cksum";
    }
}

    include_once("db.inc");

    db_init();
    $result = mysql_query("select * from app where name='core_client'");
    $core_app = mysql_fetch_object($result);
    mysql_free_result($result);

    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        platform_downloads($platform, $core_app);
    }
    mysql_free_result($result);
?>
