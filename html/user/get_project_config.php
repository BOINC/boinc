<?php

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

$config = get_config();
$name = parse_config($config, "long_name");

echo "<project_config>\n";
echo "    <name>$name</name>\n";
echo "    <min_passwd_length>7</min_passwd_length>\n";
echo "</project_config>\n";

?>
