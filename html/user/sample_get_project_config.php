<?php

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

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

$local_revision = trim(@file_get_contents("../../local.revision"));
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
echo "
</project_config>
";

?>
