<?php

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

$config = get_config();
$name = parse_config($config, "<long_name>");

echo "<project_config>
    <name>$name</name>
";

if (parse_bool($config, "disable_account_creation")) {
    echo "    <account_creation_disabled/>\n";
}
echo "
    <min_passwd_length>6</min_passwd_length>
</project_config>
";

?>
