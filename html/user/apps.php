<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$platforms = array();
$r2 = mysql_query("select * from platform");
while ($platform = mysql_fetch_object($r2)) {
    if ($platform->deprecated) continue;
    array_push($platforms, $platform);
}
mysql_free_result($r2);

$xml = $_GET['xml'];
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
    echo "<app_versions>\n";
} else {
    page_head(tr(APPS_TITLE));
    echo tr(APPS_DESCRIPTION)."<br><br>
    ";
    start_table();
}
$result = mysql_query("select * from app where deprecated=0");


while ($app = mysql_fetch_object($result)) {
    if ($xml) {
        echo "<application>\n";
        echo "    <name>$app->user_friendly_name</name>\n";
    } else {
        echo "
            <tr><th colspan=3>$app->user_friendly_name</th></tr>
            <tr><th>".tr(APPS_PLATFORM)."</th><th>".tr(APPS_VERSION)."</th><th>".tr(APPS_INSTALLTIME)."</th></tr>\n
        ";
    }
    for ($i=0; $i<sizeof($platforms); $i++) {
        $platform = $platforms[$i];
        $newest = null;
        $r2 = mysql_query("select * from app_version where appid=$app->id and platformid = $platform->id");
        while ($av = mysql_fetch_object($r2)) {
            if ($av->deprecated) continue;
            if (!$newest || $av->version_num>$newest->version_num) {
                $newest = $av;
            }
        }
        if ($newest) {
            $y = pretty_time_str($newest->create_time);
            if ($xml) {
                echo "    <version>\n";
                echo "        <platform>$platform->user_friendly_name</platform>\n";
                echo "        <version_num>$newest->version_num</version_num>\n";
                echo "        <date>$y</date>\n";
                echo "    </version>\n";
            } else {
                $x = sprintf("%0.2f", $newest->version_num/100);
                echo "<tr>
                    <td>$platform->user_friendly_name</td>
                    <td>$x</td>
                    <td>$y</td>
                    </tr>
                ";
            }
        }
    }
    if ($xml) {
        echo "    </application>\n";
    }
}
mysql_free_result($result);
if ($xml) {
    echo "</app_versions>\n";
} else {
    end_table();
    page_tail();
}
?>
