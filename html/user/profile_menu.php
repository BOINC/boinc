<?php

require_once("db.inc");
require_once("util.inc");
require_once("project_specific/project.inc");
require_once("profile.inc");


db_init();

if ($_GET['cmd']) {
  execute_command();
  exit();
}

page_head("Profile Zone");

start_table_noborder();
rowify("
    User profiles provide a way for individuals to share backgrounds
    and opinions with the " . PROJECT . " community.
    Explore the diversity of your fellow searchers,
    and contribute your own views for others to enjoy.
    <p>
    If you haven't already, you can
    <a href=create_profile.php>create your own user profile</a>
    for others to see!
");
rowify("<br>");

$today = getdate(time());
$UOTD_heading = "User of the Day -- " . $today['month'] . " " . $today['mday'] . ", " . $today['year'];
row1($UOTD_heading);
echo "<tr><td>";
include("uotd.html");
echo "</td></tr>";

rowify("<br>");
row1("User Profile Explorer");
echo "<tr><td>
    <ul>
    <li>View the <a href=" . PROFILE_PATH . "user_gallery_1.html>User Picture Gallery</a>.
    <li>Browse profiles <a href=" . PROFILE_PATH . "profile_country.html>by country</a>.
    <li>Browse profiles <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=-1>at random</a>,
    <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=1>at random with pictures</a>, or 
    <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=0>at random without pictures</a>. 

    <li>Alphabetical profile listings <i></i>:<br>
";

include "profile_alpha.html";

echo "<br></ul></td></tr>";

rowify("<br>");
row1("Search user names");

rowify("
    <form action=".$_SERVER['PHP_SELF']." method=GET>
    <input type=hidden name=cmd value=search>
    <input name=uname>
    <input type=submit value=OK>
    </form>
");
row1("Search profile text");
rowify("
    <form action=profile_search_action.php method=GET>
    <input name=search_string>
    <input type=submit value=OK>
    </form>
");
end_table();

page_tail();

function execute_command() {

    // Request for a random profile.
    //
    if ($_GET['cmd'] == "rand") {
        if ($_GET['pic'] == 0) {
            $result = mysql_query("SELECT userid FROM profile WHERE has_picture=0");
        } else if ($_GET['pic'] == 1) {
            $result = mysql_query("SELECT userid FROM profile WHERE has_picture=1");
        } else if ($_GET['pic'] == -1) {
            $result = mysql_query("SELECT userid FROM profile");
        }

        while ($row = mysql_fetch_row($result)) {
            $userIds[] = $row[0];
        }

        if (count($userIds) == 0) {
            echo "No profiles matched your query.<br>";
            exit();
        }

        shuffle($userIds);
        header("Location: " . URL_BASE . "view_profile.php?userid=" . $userIds[0]);
        exit();
    }
  
    else if ($_GET['cmd'] == "search") {
    
        if ($_GET['name']) {
            $result = mysql_query("SELECT id FROM user WHERE name LIKE \"%" . $_GET['uname'] . "%\"");
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

// TODO: This function should generate multiple pages,
//  and should possibly take the number of results to display
// from a user-input parameter.
// Look at build_profile_pages for an example of a good way to do this
// (albeit writing to a file in that case).

function show_search_results($members) {
    page_head("Profile Search Results");
  
    if (count($members) > 0) {
        show_user_table($members, 0, 20, 2);
    } else {
        echo "No profiles matched your query.<br>";
    }

    page_tail();
}

?>
