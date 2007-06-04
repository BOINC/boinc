<?php

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

// This all needs to work even when DB is down.
// So cache list of platforms in a file,
// and update it every hour if possible.
//
function show_platforms() {
    $path = "../cache/platform_list.xml";
    $mtime = @filemtime($path);
    if ($mtime && (time() - $mtime) < 3600) {
        @readfile($path);
    } else {
        require_once("../inc/db.inc");
        $retval = db_init_aux(true);
        if (!$retval) {
            $query = 'select name, user_friendly_name from platform, app_version where app_version.platformid = platform.id and app_version.deprecated=0 group by name';
            $result = mysql_query($query);
            $f = fopen($path, "w");
            fwrite($f, "<platforms>\n");
            while ($p = mysql_fetch_object($result)) {
                fwrite($f,
                    "  <platform>\n    <name>$p->name</name>\n    <user_friendly_name>$p->user_friendly_name</user_friendly_name>\n  </platform>\n"
                );
            }
            mysql_free_result($result);
            fwrite($f, "</platforms>\n");
            fclose($f);
            @readfile($path);
        }
    }
}

$config = get_config();
$long_name = parse_config($config, "<long_name>");
$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) {
    $min_passwd_length = 6;
}
$disable_account_creation = parse_bool($config, "disable_account_creation");
$master_url = parse_config($config, "<master_url>");

echo "<project_config>
    <name>$long_name</name>
    <master_url>$master_url</master_url>
";

$local_revision = trim(file_get_contents("../../local.revision"));
if ($local_revision) {
    echo "<local_revision>$local_revision</local_revision>\n";
}

if (web_stopped()) {
    echo "
        <error_num>-183</error_num>
        <web_stopped>1</web_stopped>
    ";
} else {
    echo "<web_stopped>0</web_stopped>\n";
    if ($disable_account_creation || defined('INVITE_CODES')) {
        echo "    <account_creation_disabled/>\n";
    }
    echo "
        <min_passwd_length>$min_passwd_length</min_passwd_length>
    ";
}
if (sched_stopped()) {
    echo "<sched_stopped>1</sched_stopped>\n";
} else {
    echo "<sched_stopped>0</sched_stopped>\n";
}

show_platforms();

echo "
</project_config>
";

?>
