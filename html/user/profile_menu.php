<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/profile.inc");

db_init();


$cmd = $_GET['cmd'];
if ($cmd) {
    execute_command($cmd);
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
include( PROFILE_PATH . "uotd.html" );
echo "</td></tr>";

rowify("<br>");
row1("User Profile Explorer");
echo "<tr><td>
    <ul>
    <li>View the <a href=" . URL_BASE . "user_profile/user_gallery_1.html>User Picture Gallery</a>.
    <li>Browse profiles <a href=" . URL_BASE . "profile_country.html>by country</a>.
    <li>Browse profiles <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=-1>at random</a>,
    <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=1>at random with pictures</a>, or 
    <a href=" . $_SERVER['PHP_SELF'] . "?cmd=rand&pic=0>at random without pictures</a>. 

    <li>Alphabetical profile listings <i></i>:<br>
";

include( PROFILE_PATH . "profile_alpha.html" );

echo "<br></ul></td></tr>";

rowify("<br>");
row1("Search user names");

rowify("
    <form action=user_search_action.php method=GET>
    <input name=search_string>
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

function execute_command($cmd) {
    // Request for a random profile.
    //
    if ($cmd == "rand") {
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
  
}

?>
