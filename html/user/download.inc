<?php

function platform_downloads($platform, $core_app) {
    $result = mysql_query("select * from app_version where platformid=$platform->id and appid=$core_app->id order by version_num desc");

    if (!$result) return;

    $found = false;

    $download_url = parse_config("<download_url>");
    if ($app_version = mysql_fetch_object($result)) {
        $filename = parse_element($app_version->xml_doc, "<name>");
        if (!$filename) {
            echo "CAN'T FIND FILENAMEn $app_version->xml_doc\n";
        }
        $version = sprintf("%.2f", $app_version->version_num/100);
        row2("<a href=$download_url/$filename>$platform->user_friendly_name", $version);
        //$app_version->md5_cksum";
        $found = true;
    }

    mysql_free_result($result);
}

function print_download_links() {
    start_table();

    $result = mysql_query("select * from app where name='core client'");
    $core_app = mysql_fetch_object($result);
    mysql_free_result($result);

    echo "<tr><th>Computer type</th><td>Version</th></tr>\n";
    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        platform_downloads($platform, $core_app);
    }
    mysql_free_result($result);
    echo "</table>\n";
}

?>
