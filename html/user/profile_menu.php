<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


require_once("../inc/util.inc");
require_once("../inc/profile.inc");
require_once("../inc/uotd.inc");

if (DISABLE_PROFILES) error_page("Profiles are disabled");

check_get_args(array("cmd", "pic"));

$option = get_str('cmd', true);
if ($option) {
    select_profile($option);
    exit();
}

page_head(tra("Profiles"));

echo "
    <p>".tra("%1 Profiles %2 let individuals share backgrounds and opinions with the %3 community.", "<b>", "</b>", PROJECT)." ".
    tra("Explore the diversity of your fellow volunteers, and contribute your own views for others to enjoy.")."
    <p>".tra("If you haven't already, you can %1 create your own user profile %2 for others to see!", "<a href=\"create_profile.php\">", "</a>");

start_table();

$today = getdate(time());
$UOTD_heading = tra("User of the Day")." -- " . $today['month'] . " " . $today['mday'] . ", " . $today['year'];
row1($UOTD_heading);
echo "<tr><td>";
    $profile = get_current_uotd();
    if ($profile) {
        $user = BoincUser::lookup_id($profile->userid);
        echo uotd_thumbnail($profile, $user);
        echo user_links($user, BADGE_HEIGHT_MEDIUM)."<br>";
        $resp = output_transform($profile->response1);
        $resp = sanitize_tags($resp);
        echo sub_sentence($resp, ' ', 150, true);
    }

echo "</td></tr>";

row1(tra("User Profile Explorer"));
echo "<tr><td>
    <ul>
    <li>".tra("View the %1 User Picture Gallery %2.", "<a href=\"" . url_base() . "user_profile/user_gallery_1.html\">", "</a>")."</li>
    <li>".tra("Browse profiles %1 by country %2.", "<a href=\"" . url_base() . "user_profile/profile_country.html\">", "</a>")."</li>
    <li>".tra("Browse profiles %1 at random %2, %3 at random with pictures %2, or %4 at random without pictures %2.", "<a href=\"?cmd=rand&amp;pic=-1\">", "</a>",
            "<a href=\"?cmd=rand&amp;pic=1\">", "<a href=\"?cmd=rand&amp;pic=0\">")."</li>
";
if (file_exists(PROFILE_PATH . "profile_alpha.html")) {
    echo "<li>".tra("Alphabetical profile listings:")."<br>";

    include( PROFILE_PATH . "profile_alpha.html" );
}
echo "</ul></td></tr>";

row1(tra("Search profile text"));
rowify(
    sprintf(
        '<form action="profile_search_action.php" method="GET">
        <input type="text" name="search_string">
        <input class="btn btn-sm" %s type="submit" value="%s">
        </form>',
        button_style(),
        tra("Search")
    )
);
end_table();

page_tail();

function select_profile($cmd) {
    // Request for a random profile.
    //
    if ($cmd == "rand") {
        $profiles = array();
        $pic = get_int('pic');
        if ($pic == 0) {
            $profiles = BoincProfile::enum("has_picture=0", "limit 1000");
        } else if ($pic == 1) {
            $profiles = BoincProfile::enum("has_picture=1", "limit 1000");
        } else if ($pic == -1) {
            $profiles = BoincProfile::enum(null, "limit 1000");
        }

        if (count($profiles) == 0) {
            page_head(tra("No profiles"));
            echo tra("No profiles matched your query.");
            page_tail();
            exit();
        }

        shuffle($profiles);
        $userid = $profiles[0]->userid;
        header("Location: ".url_base()."view_profile.php?userid=$userid");
        exit();
    }
}

?>
