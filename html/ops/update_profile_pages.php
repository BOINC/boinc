#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/uotd.inc");
require_once("../inc/db.inc");
require_once("../inc/profile.inc");

echo date(DATE_RFC822), ": Starting\n";

set_time_limit(0);
ini_set("memory_limit", "1024M");

$debug = true;

function print_debug_msg($text) {
    global $debug;
    if ($debug) echo $text."\n";
}

db_init();

// TODO: convert to new DB interface

$alphabet = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9');


// Builds a summary table of user profiles.
//
//   $members is an array of userIDs;
//   $offset indicates which entry to begin the table with
//   $numToDisplay indicates how many profiles to display in this table
//   $cols indicates how many profile summaries should be written per row
//   $descriptor is an optional file descriptor to write the table to.

function show_user_table($members, $offset, $numToDisplay, $cols) {
    start_table();

    $rows = ceil($numToDisplay / $cols);
    $count = $offset;
    $numMembers = count($members);

    for ($row = 0; $row < $rows; $row++) {
        if ($count >= $numMembers) {
            break;
        }

        echo "<tr>\n";

        for ($col = 0; $col < $cols; $col++) {
            if ($count < $numMembers) {
                $profile = get_profile($members[$count]);
                if (!$profile) {
                    $numMembers--;
                    continue;
                }

                echo "<td class=bordered width=7% height=64><center>";

                $show_picture = $profile->has_picture;
                if (profile_screening() && $profile->verification != 1) {
                    $show_picture = false;
                }
                if ($show_picture) {
                    echo "<a href=\"".secure_url_base()."view_profile.php?userid={$members[$count]}\"><img src=\"".secure_url_base().IMAGE_URL."{$members[$count]}_sm.jpg\"></a>";
                } else {
                    echo "&nbsp;";
                }

                echo "</center></td><td class=bordered width=33% height=64>\n", get_profile_summary($profile), "</td>";
                $count++;
            } else {
                echo "<td width=7% height=64></td><td width=33% height=64></td>";
            }
        }
        echo "</tr>\n";
    }
    end_table();
}

// Generates a standard set of links between associated multi-page documents.
//  All linked files must be of the form "$filename_<page number>.html".

function write_page_links($filename, $currPageNum, $numPages) {
    echo "<p>Page $currPageNum of $numPages</p>";

    $nextPageNum = $currPageNum + 1;
    $prevPageNum = $currPageNum - 1;

    // Make the 'previous' and 'next' page links as appropriate.
    if ($currPageNum > 1) {
        echo "<a href={$filename}_{$prevPageNum}.html>Previous page</a>";

        if ($currPageNum != $numPages) {
            echo " | ";
        }
    }
    if ($currPageNum != $numPages) {
        echo "<a href={$filename}_{$nextPageNum}.html>Next page</a>";
    }

    echo "<p>Jump to Page:\n";

    // Make the individual page links (or a bold non-link for the current page).
    //
    for ($i = 1; $i <= $numPages; $i++) {
        if ($i != $currPageNum) {
            echo "<a href={$filename}_{$i}.html>$i</a>\n";
        } else {
            echo "<b>$i</b>\n";
        }
    }

}

// Generates the html files which comprise the photo gallery.
//   $room: which gallery to generate (user, computer).
//   $width: the width of the table of images.
//   $height: the height of the table of images.
//
function build_picture_pages($width, $height) {
    print_debug_msg("Beginning to build picture pages...");

    // TODO: Add support for a computer image gallery.

    // TODO: Standardize "Last modified" string to a function call (util.inc).

    if (profile_screening()) {
        $profiles = BoincProfile::enum_fields('userid', 'has_picture = 1 AND verification=1', 'order by userid');
    } else {
        $profiles = BoincProfile::enum_fields('userid', 'has_picture = 1', 'order by userid');
    }

    // Build an array of IDs of all users with pictures in their profiles.
    $userIds = array();
    $numIds = 0;
    foreach($profiles as $profile) {
        $user = BoincUser::lookup_id($profile->userid);
        if (!$user) continue; // maybe we should delete the profile if user is non-existent anymore?
        if ($user->name) {
            $userIds[] = $profile->userid;
            $numIds++;
        }
    }

    $msg = "$numIds users have profiles AND uploaded a picture";
    if (profile_screening()) $msg .= " AND were screened by the project";
    print_debug_msg($msg);

// don't randomize; makes things hard for people who sift profiles
//
//    if (count($userIds) > 0) {
//        // Randomize the ordering of users.
//        shuffle($userIds);
//    }

    $numPages = ceil(count($userIds)/($width * $height));

    // Make sure that a page is generated even when no profiles with pictures
    // exist in order to avoid 404 errors from the profile_menu page.

    if ($numPages == 0) {
        $numPages = 1;
    }

    print_debug_msg("Generating $numPages pages");

    $count = 0;

    for ($page = 1; $page <= $numPages; $page++) {
        $filename = PROFILE_PATH . "user_gallery_" . $page . ".html";
        open_output_buffer();

        page_head("Profile gallery: page $page of $numPages", null, false, "../");

        echo "Last updated ", pretty_time_str(time()),
            "\n<p>Browse the user profiles by picture.
            Only user profiles with pictures are listed here.";


        echo "<table class=\"table table-bordered\">\n";

        for ($row = 0; $row < $height; $row++) {
            echo "<tr>";
            for ($col = 0; $col < $width; $col++) {
	            if ($count < $numIds) {
                    echo "<td class=\"bordered\" align=\"center\">
                        <a href=\"".secure_url_base()."view_profile.php?userid=".$userIds[$count]."\"><img src=\"".secure_url_base().IMAGE_URL.$userIds[$count]."_sm.jpg\"></a>
                    </td>";
                    $count++;
                }
            }
            echo "</tr>\n";
            if ($count == $numIds) {
                break;
            }
        }

        echo "</table>\n";

        // Previous and Next links

        write_page_links("user_gallery", $page, $numPages);

        page_tail(false, "../");

        close_output_buffer($filename);
    }

    print_debug_msg("done building picture pages");
}

// Creates pages grouping user profiles by country.  Filenames are of the
// format "profile_country_<country name>_<page number>.html"
// Also creates a summary page listing all countries which have profiled
// members, the number of such members, and links to the created pages for
// each country.

function build_country_pages() {
    print_debug_msg("Beginning to build country pages...");
    $profiles = BoincProfile::enum_fields('userid');
    $numIds = 0;
    $countryMembers = array();

    // Build a multi-dimensional array of countries,
    // each element of which contains an array
    // of the userids who belong to those countries.
    // Format: array[country][index] = userid.

    foreach($profiles as $profile) {
        $user = BoincUser::lookup_id($profile->userid);
        if (!$user) continue; // maybe we should delete the profile if user is non-existent anymore?
        if ($user->country) {
            $countryMembers[$user->country][] = $user->id;
            $numIds++;
        } else {
            $countryMembers['Other'][] = $user->id;
        }
    }

    print_debug_msg("$numIds users have profiles AND non-null country entries.");

    $countries = array_keys($countryMembers);
    sort($countries);

    // Build the pages.
    // TODO: Define a constant for the desired number of rows per page.

    foreach ($countries as $country) {
        $baseFileName =  "profile_country_" . get_legal_filename($country);
        build_profile_pages(
            $countryMembers[$country],
            "User Profiles from $country", 5, 2,
            PROFILE_PATH, $baseFileName
        );
    }

    // Build the summary page linking to the individual country pages.

    build_country_summary_page($countryMembers);

    print_debug_msg("done building country pages");
}

// Creates pages grouping users by the first letter of their names.

function build_alpha_pages() {
    print_debug_msg("Beginning to build alphabetical pages...");
    global $alphabet;

    $profiles = BoincProfile::enum_fields('userid');

    $numIds = 0;
    $members = array();

    foreach($profiles as $profile) {
        $user = BoincUser::lookup_id($profile->userid);
        if (!$user) continue; // maybe we should delete the profile if user is non-existent anymore?
        if ($user->name) {
            $name = ltrim($user->name);
            $members[strtoupper($name[0])][] = $user->id;
            $numIds++;
        }
    }

    print_debug_msg("$numIds users have profiles AND names.");

    $letters = array_keys($members);

    foreach ($letters as $letter) {
        // NOTE: Array indexing is case sensitive.
        $filePath = PROFILE_PATH;
        if (in_array($letter, $alphabet)) {
            build_profile_pages(
                $members[$letter],
                "User Profiles - Names beginning with $letter",
                5, 2, $filePath,
                "profile_$letter"
            );
        } else {
            build_profile_pages(
                $members[$letter],
                "User Profiles - Names beginning with other characters",
                5, 2, $filePath,
                "profile_other"
            );
        }
        $letters_used[$letter] = 1;
    }

    build_alpha_summary_page($letters_used);
    print_debug_msg("done building alphabetical pages");
}

// A generalized function to produce some number of pages summarizing a
// set of user profiles.

function build_profile_pages(
    $members, $title, $rowsPerPage, $colsPerPage, $dir, $base_filename
) {
    $numPerPage = $rowsPerPage * $colsPerPage;
    $numPages = ceil(count($members) / $numPerPage);

    for ($page = 1; $page <= $numPages; $page++) {
        $filename = $dir . $base_filename . "_" . $page . ".html";
        open_output_buffer();

        $pagetitle = $title.": Page $page of $numPages";
        page_head($pagetitle, null, null, "../");

        echo "Last updated ", pretty_time_str(time()), "<p>\n";

        $offset = (($page-1) * $rowsPerPage * $colsPerPage);

        show_user_table($members, $offset, $numPerPage, $colsPerPage);

        write_page_links($base_filename, $page, $numPages);

        page_tail(false, "../");

        close_output_buffer($filename);
    }

}

function build_country_summary_page($countryMembers) {
    print_debug_msg("Beginning to build country summary page...");
    $countries = array_keys($countryMembers);

    $filename = PROFILE_PATH . "profile_country.html";
    open_output_buffer();

    page_head("User Profiles by Country", null, null, "../");
    echo "Last updated " . pretty_time_str(time()) . "<p>";

    start_table();
    row_heading_array(array("Country", "Profiles"));

    foreach ($countries as $country) {
        $numMembers = count($countryMembers[$country]);
        $name = get_legal_filename($country);

        echo "<tr>\n<td><a href=\"profile_country_",
            "{$name}_1.html\">$country</a></td><td>$numMembers</td></td>\n";
    }

    end_table();
    page_tail(false, "../");

    close_output_buffer($filename);
    print_debug_msg("done building country summary page");
}

function build_alpha_summary_page($characters_used) {
    print_debug_msg("Beginning to build alphabetical summary pages...");
    global $alphabet;

    $filename = PROFILE_PATH."profile_alpha.html";
    open_output_buffer();

    foreach ($alphabet as $character) {
        if (isset($characters_used[$character])) {
            echo "<a href=\"".secure_url_base().PROFILE_URL."profile_{$character}_1.html\">$character</a>&nbsp;";
            unset($characters_used[$character]);
        } else {
            echo "$character ";
        }
    }

    // Link to the 'Other' page if necessary.
    if (!empty($characters_used)) {
        echo "<a href=\"".secure_url_base().PROFILE_URL."profile_other_1.html\">Other</a>&nbsp;";
    }
    close_output_buffer($filename);
}

$caching = true;

if (@$argv[1]=="-d") $debug=true;

build_country_pages();
build_alpha_pages();
build_picture_pages(GALLERY_WIDTH, GALLERY_HEIGHT);

echo date(DATE_RFC822), ": Finished\n";
?>
