<?php

require_once("util.inc");
require_once("project.inc");

db_init();

if ($_GET['cmd']) {
  execute_command();
  exit();
}

page_head("Profile Zone");



start_table_noborder();
row1("User Profiles");
rowify("
<ul>
<li>View the <a href=" . PROFILE_PATH . "user_gallery_1.html>User Picture Gallery</a>.  <a href=generate_picture_pages.php>[Generate]</a>
<li>Browse profiles <a href=" . PROFILE_PATH . "profile_country.html>by country</a>.  <a href=generate_country_pages.php>[Generate]</a>
<li>Browse profiles <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand>at random</a>,
<a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=1>at random with pictures</a>, or 
<a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=0>at random without pictures</a>. 

<li>Alphabetical profile listings:<br>A B C D E F G H I J K L M N O P Q R S T U V W X Y Z<BR>
</ul>");
row1("Search");
end_table();

echo "
<form action=", $_SERVER['PHP_SELF'], " method=\"GET\">
<input type=\"hidden\" name=\"cmd\" value=\"search\">
<input type=\"text\" name=\"name\">
<input type=\"submit\" value=\"Submit\">
</form>";

end_table();
page_tail();

function execute_command() {

  // Request for a random profile.
  if ($_GET['cmd'] == "rand") {
    if ($_GET['pic']) {
      if ($_GET['pic'] == 0) {
	$result = mysql_query("SELECT userid FROM profile WHERE has_picture = 0");
      } else if ($_GET['pic'] == 1) {
	$result = mysql_query("SELECT userid FROM profile WHERE has_picture = 1");
      }
    } else {
      $result = mysql_query("SELECT userid FROM profile");
    }

    while ($row = mysql_fetch_row($result)) {
      $userIds[] = $row[0];
    }

    shuffle($userIds);
    
    header("Location: " . MASTER_URL . "view_profile.php?userid=" . $userIds[0]);
    exit();
  }
  
  else if ($_GET['cmd'] == "search") {
    
    if ($_GET['name']) {
      $result = mysql_query("SELECT id FROM user WHERE name = " . $_GET['name']);
      if ($result) {
	while ($row = mysql_fetch_row($result)) {
	  echo "<br>ID ", $row['0'], " has a name that matches your query.";
	}
      } else {
	echo "No names matched your query.<br>";
      }
    }
  }
}

?>