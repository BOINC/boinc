<?php

function platform_downloads($platform) {
    $result = mysql_query("select * from core_version where platformid=$platform->id order by version_num desc");

    if (!$result) return;

    $found = false;

    if ($core_version = mysql_fetch_object($result)) {
        $version = sprintf("%.2f", $core_version->version_num/100);
        $url = parse_element($core_version->xml_doc, "<url>");
        row2("<a href=$url>$platform->user_friendly_name", $version);
        //$app_version->md5_cksum";
        $found = true;
    }

    mysql_free_result($result);
}

function print_download_links() {
    start_table();

    echo "<tr><th>Computer type</th><td>Version</th></tr>\n";
    $result = mysql_query("select * from platform order by name");
    while ($platform = mysql_fetch_object($result)) {
        platform_downloads($platform);
    }
    mysql_free_result($result);
    echo "</table>\n";
}

?>
