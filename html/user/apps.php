<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

$platforms = BoincPlatform::enum("deprecated=0");

$xml = $_GET['xml'];
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
    echo "<app_versions>\n";
} else {
    page_head(tra("Applications"));
    echo tra("%1 currently has the following applications. When you participate in %1, work for one or more of these applications will be assigned to your computer. The current version of the application will be downloaded to your computer. This happens automatically; you don't have to do anything.", PROJECT)."<br><br>
    ";
    start_table();
}

$apps = BoincApp::enum("deprecated=0");

foreach ($apps as $app) {
    if ($xml) {
        echo "<application>\n";
        echo "    <name>$app->user_friendly_name</name>\n";
    } else {
        echo "
            <tr><th colspan=3>$app->user_friendly_name</th></tr>
            <tr><th>".tra("Platform")."</th><th>".tra("Current version")."</th><th>".tra("Installation time")."</th></tr>\n
        ";
    }
    for ($i=0; $i<sizeof($platforms); $i++) {
        $platform = $platforms[$i];
        $newest = null;
        $avs = BoincAppVersion::enum("appid=$app->id and platformid = $platform->id and deprecated=0");
        foreach($avs as $av) {
            if (!$newest || $av->version_num>$newest->version_num) {
                $newest = $av;
            }
        }
        if ($newest) {
            $y = pretty_time_str($newest->create_time);
            if ($xml) {
                echo "    <version>\n";
                echo "        <platform_short>$platform->name</platform_short>\n";
                echo "        <platform_long>$platform->user_friendly_name</platform_long>\n";
                echo "        <version_num>$newest->version_num</version_num>\n";
                echo "        <date>$y</date>\n";
                echo "        <date_unix>$newest->create_time</date_unix>\n";
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

if ($xml) {
    echo "</app_versions>\n";
} else {
    end_table();
    page_tail();
}
?>
