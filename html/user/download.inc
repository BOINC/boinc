<?php

function platform_downloads($platform) {
    $result = mysql_query("select * from core_version where platformid=$platform->id and version_num>=200 order by version_num desc");

    if (!$result) return;

    $found = false;

    // if less than this many days old, print "New!"
    $NEW_THRESHOLD = 14;

    if ($core_version = mysql_fetch_object($result)) {
        $version = sprintf("%.2f", $core_version->version_num/100);
        $url = parse_element($core_version->xml_doc, "<url>");
        $date = date_str($core_version->create_time);
        if ($core_version->create_time > time()-($NEW_THRESHOLD*60*60*24)) {
            $date .= ' <font color=red>New!</font>';
        }
        row3("<a href=$url>$platform->user_friendly_name", $version, $date);
        //$app_version->md5_cksum";
        $found = true;
    }

    mysql_free_result($result);
}

function print_download_links() {
    start_table();

    echo "<tr><th>Computer type</th><th>Version</th><th>Release Date</th></tr>\n";
    $result = mysql_query("select * from platform order by name");
    while ($platform = mysql_fetch_object($result)) {
        if ($platform->deprecated) continue;
        platform_downloads($platform);
    }
    mysql_free_result($result);
    echo "</table>\n";
}

?>
