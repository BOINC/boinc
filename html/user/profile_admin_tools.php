<?php

require_once("project_specific/project.inc");
require_once("util.inc");

page_head("Profile Administration");

echo "
<a href=generate_picture_pages.php>Generate Picture Gallery</a><br>
<a href=generate_country_pages.php>Generate Country Gallery</a><br>
";

page_tail();

?>
