<?php

require_once("util.inc");

parse_str(getenv("QUERY_STRING"));

page_head("Delete project confirmation");
echo "Are you sure you want to stop participating in the project $master_url?\n";
echo "<br><a href=prefs_delete_project.php?master_url=$master_url>Yes</a>\n";
echo "<br><a href=prefs_edit_projects.php>No</a>\n";
echo "<p>\n";
page_tail();
?>
