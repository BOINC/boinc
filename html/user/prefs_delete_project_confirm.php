<?php

parse_str(getenv("QUERY_STRING"));

echo "Are you sure you want to stop participating in the project $master_url?\n";
echo "<br><a href=prefs_delete_project?master_url=$master_url>Yes</a>\n";
echo "<br><a href=prefs_edit_projects.php>No</a>\n";
?>
