<?php

require_once("util.inc");
require_once("project_specific/project.inc");
require_once("gallery.inc");

db_init();

if ($_GET['cmd']) {
  execute_command();
  exit();
}

page_head("Profile Zone");

start_table_noborder();
rowify("
User profiles provide a way for individuals to share backgrounds and opinions with the " . PROJECT . " community. Explore the diversity of your fellow searchers, and contribute your own views for others to enjoy.
<p>
If you haven't already, you can <a href=create_profile.php>create your own user profile</a> for others to see!
");
rowify("<br>");
row1("User Profile Explorer");
rowify("
<ul>
<li>View the <a href=" . PROFILE_PATH . "user_gallery_1.html>User Picture Gallery</a>.
<li>Browse profiles <a href=" . PROFILE_PATH . "profile_country.html>by country</a>.
<li>Browse profiles <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=-1>at random</a>,
<a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=1>at random with pictures</a>, or 
<a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=0>at random without pictures</a>. 

<li>Alphabetical profile listings <i>(Coming Soon)</i>:<br>A B C D E F G H I J K L M N O P Q R S T U V W X Y Z<BR>
</ul>");

// TODO: Create management page with links to generate galleries.
// <a href=generate_picture_pages.php>[Generate]</a>
// <a href=generate_country_pages.php>[Generate]</a>

rowify("<br>");
row1("Name Search");
rowify("<br>");
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
    
    if ($_GET['pic'] == 0) {
      $result = mysql_query("SELECT userid FROM profile WHERE has_picture = 0");
    } else if ($_GET['pic'] == 1) {
      $result = mysql_query("SELECT userid FROM profile WHERE has_picture = 1");
    } else if ($_GET['pic'] == -1) {
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
      $result = mysql_query("SELECT id FROM user WHERE name LIKE \"%" . $_GET['name'] . "%\"");
      
      while($row = mysql_fetch_assoc($result)) {
	$result2 = mysql_query("SELECT userid FROM profile WHERE userid = " . $row['id']);
	if ($result2) {
	  $row2 = mysql_fetch_assoc($result2);
	
	  if ($row2['userid']) {
	    $members[] = $row2['userid'];
	  }
	}
      }
      
      show_search_results($members);
    }
  }
}

function show_search_results($members) {
  page_head("Profile Search Results");
  
  if (count($members) > 0) {
    show_user_table($members, 0, 10, 2);
  } else {
    echo "No profiles matched your query.<br>";
  }

  page_tail();

}

?>
