<?php

require_once("../inc/xml.inc");

xml_header();
echo "
<project_config>
    <name>Example name</name>
    <min_passwd_length>7</min_passwd_length>
</project_config>
";

?>
