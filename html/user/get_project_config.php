<?php

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

$name = parse_config(get_config(), "<long_name>");

echo "<project_config>
    <name>$name</name>
    <min_passwd_length>6</min_passwd_length>
</project_config>
";

?>
