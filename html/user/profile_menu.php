<?php

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/profile.inc");
require_once("../inc/uotd.inc");

db_init();

$option = get_str('cmd', true);
if ($option) {
    select_profile($option);
    exit();
}

page_head("Profiles");

echo "
    <p>
    <b>Profiles</b> let individuals share backgrounds
    and opinions with the " . PROJECT . " community.
    Explore the diversity of your fellow volunteers,
    and contribute your own views for others to enjoy.
    <p>
    If you haven't already, you can
    <a href=create_profile.php>create your own user profile</a>
    for others to see!
";
rowify("<br>");
start_table_noborder();

$today = getdate(time());
$UOTD_heading = "User of the Day -- " . $today['month'] . " " . $today['mday'] . ", " . $today['year'];
row1($UOTD_heading);
echo "<tr><td>";
    $profile = get_current_uotd();
    if ($profile) {
        $user = lookup_user_id($profile->userid);
        echo uotd_thumbnail($profile, $user);
        echo user_links($user)."<br>";
        echo sub_sentence(output_transform(strip_tags($profile->response1)), ' ', 150, true);
    }

echo "</td></tr>";

rowify("<br>");
row1("User Profile Explorer");
echo "<tr><td>
    <ul>
    <li>View the <a href=\"" . URL_BASE . "user_profile/user_gallery_1.html\">User Picture Gallery</a>.</li>
    <li>Browse profiles <a href=\"" . URL_BASE . "user_profile/profile_country.html\">by country</a>.</li>
    <li>Browse profiles <a href=\"?cmd=rand&pic=-1\">at random</a>,
    <a href=\"?cmd=rand&pic=1\">at random with pictures</a>, or 
    <a href=\"?cmd=rand&pic=0\">at random without pictures</a>.</li>
";
if (file_exists(PROFILE_PATH . "profile_alpha.html")) {
    echo "<li>Alphabetical profile listings:<br>";

    include( PROFILE_PATH . "profile_alpha.html" );
}
echo "</ul></td></tr>";

row1("Search profile text");
rowify("
    <form action=\"profile_search_action.php\" method=\"GET\">
    <input name=\"search_string\">
    <input type=\"submit\" value=\"OK\">
    </form>
");
end_table();

page_tail();

function select_profile($cmd) {
    // Request for a random profile.
    //
    if ($cmd == "rand") {
        $profiles = array();
        if ($_GET['pic'] == 0) {
            $profiles = BoincProfile::enum("has_picture=0", "limit 1000");
        } else if ($_GET['pic'] == 1) {
            $profiles = BoincProfile::enum("has_picture=1", "limit 1000");
        } else if ($_GET['pic'] == -1) {
            $profiles = BoincProfile::enum(null, "limit 1000");
        }

        if (count($profiles) == 0) {
            page_head("No profiles");
            echo "No profiles matched your query.<br>";
            page_tail();
            exit();
        }

        shuffle($profiles);
        $userid = $profiles[0]->userid;
        header("Location: ".URL_BASE."view_profile.php?userid=$userid");
        exit();
    }
}

?>
