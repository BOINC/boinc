<?php

require_once("../inc/xml.inc");

xml_header();
echo "
<project_config>
    <uses_email_id>1</uses_email_id>
    <name>Example name</name>
    <min_passwd_length>7</min_passwd_length>
</project_config>
";

?>
