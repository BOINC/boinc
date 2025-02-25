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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/cache.inc");
require_once("../inc/user.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../inc/forum.inc");

define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 640);
define('MAX_IMG_HEIGHT', 480);

define('MAX_DESC_LENGTH', 90);

define('GALLERY_WIDTH', 7);
define('GALLERY_HEIGHT', 4);

function profile_screening() {
    static $val;
    if (!isset($val)) {
        $config = get_config();
        $val = parse_bool($config, "profile_screening");
    }
    return $val;
}

function get_profile($userid) {
    return BoincProfile::lookup("userid = $userid");
}

// TODO: use the following functions instead of hardwired crap everywhere

function profile_image_path($userid) {
    return IMAGE_PATH.$userid.'.jpg';
}

function profile_thumb_path($userid) {
    return IMAGE_PATH.$userid.'_sm.jpg';
}

function profile_image_url($userid) {
    return url_base().IMAGE_URL.$userid.'.jpg';
}

function profile_thumb_url($userid) {
    return url_base().IMAGE_URL.$userid.'_sm.jpg';
}

function profile_user_thumb_url($user) {
    if (!$user->has_profile) return null;
    $profile = BoincProfile::lookup("userid=$user->id");
    if (!$profile->has_picture) return null;
    if (profile_screening() && $profile->verification!=1) return null;
    return profile_thumb_url($user->id);
}

// When passed profile->verification, this function is used to tell the
// user the verification status of their profile.
//
function offensive_profile_warning($verify_flag) {
    if ($verify_flag == 0) {
        return "
            <font size='+2' color='#3c3'>
            ".tra("Your profile will be made visible to other people as soon as it has been approved by the project. This may take up to a few days.")."
            </font>
        ";
    } else if ($verify_flag == -1) {
        return "
            <font size='+2' color='#f33'>
            ".tra("Your profile has been marked as unacceptable. It is not visible to other people. Please change it.")."
            </font>
        ";
    }
    return "";
}

// If the user with id = $userid has uploaded a picture his/herself,
// delete it and its thumbnail.
//
function delete_user_pictures($userid) {
    $path = profile_image_path($userid);
    if (file_exists($path)) {
        unlink($path);
    }
    $path = profile_thumb_path($userid);
    if (file_exists($path)) {
        unlink($path);
    }
}

function delete_profile($user) {
    delete_user_pictures($user->id);
    return BoincProfile::delete_aux("userid=$user->id");
}

function scale_image(
    $image, $origWidth, $origHeight, $targetWidth, $targetHeight
) {

    // If the image is already smaller than the target dimensions,
    // just return it.
    //
    if ($origWidth <= $targetWidth && $origHeight <= $targetHeight) {
        return $image;
    }

    ($origWidth > $origHeight)? $scalar = ($origWidth / $targetWidth) : $scalar = ($origHeight / $targetHeight);

    if ($scalar != 0) {
        $destWidth = $origWidth / $scalar;
        $destHeight = $origHeight / $scalar;
    } else {
        $destWidth = $origWidth;
        $destHeight = $origHeight;
    }

    $gd_info = gd_info();
    $v = $gd_info["GD Version"];
    $v = explode('.', $v);
    $v = (int)$v[0];        // major version
    if ($v >= 2) {
        // If you are using a modern PHP/GD installation that does
        // 'truecolor' images, this is what's needed.
        $newImage = ImageCreateTrueColor($destWidth, $destHeight);
        ImageCopyResampled(
            $newImage, $image, 0, 0, 0, 0, $destWidth,
            $destHeight, $origWidth, $origHeight
        );
    } else {
        // If not, use this block.
        // The image quality is lower but it works using older PHP/GD versions.
        $newImage = ImageCreate($destWidth, $destHeight);
        ImageCopyResized(
            $newImage, $image, 0, 0, 0, 0, $destWidth, $destHeight,
            $origWidth, $origHeight
        );
    }

    return $newImage;
}

// Generates a string containing:
//   1) the name of the user with ID == $userid,
//      with a link to a view of their profile
//   2) the first MAX_DESC_LENGTH characters from the response1 field
//      of said user's profile.

function get_profile_summary($profile) {
    $user = BoincUser::lookup_id($profile->userid);

    if (!$user || !$profile) {
        error_page(tra("Database error"));
        exit();
    }

    $description = "";

    if (strlen($profile->response1) != 0) {
        $options = new output_options();
        $options->htmlitems = false;
        $temp = output_transform($profile->response1, $options);
        $temp = sanitize_tags($temp);
        $description = "(\"" . sub_sentence($temp, ' ', MAX_DESC_LENGTH, true) . "\")";

    }

    $summary = "<a href=\"".url_base()."view_profile.php?userid=".$profile->userid."\">".$user->name."</a> ".$description;
    return $summary;
}

function check_whether_to_show_profile($user, $logged_in_user) {
    $min_credit = parse_config(get_config(), "<profile_min_credit>");
    if (!$logged_in_user && $min_credit && $user->expavg_credit < $min_credit ) {
        error_page(
           tra("To prevent spam, profiles of users with an average credit of less than %1 are displayed only to logged-in users. We apologize for this inconvenience.", $min_credit)
        );
    }
}

// Displays a user's profile (if they have one);
// $screen_mode is set if we're in the administrative profile-screening page,
// in which case we show everything
// This assumes we're inside a table; it generates table rows
//
function show_profile($user, $logged_in_user, $screen_mode = false) {
    BoincForumPrefs::lookup($user);
    $profile = BoincProfile::lookup("userid = $user->id");
    if (!$profile) {
        row1(tra("No profile exists for that user ID."));
        $user->update("has_profile = 0");
        return;
    }

    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;

    if ($can_edit) {
        echo "<tr><td>";
        show_button("create_profile.php", tra("Edit your profile"));
        echo "</td></tr>
        ";
    }

    // If screening is enabled, only show picture in certain situations
    //
    $show_picture = $profile->has_picture;
    if (profile_screening()) {
        if (!$screen_mode && !$can_edit && $profile->verification!=1) {
            $show_picture = false;
        }
    }
    if ($show_picture) {
        echo "
            <tr><td colspan=\"2\" align=\"center\">
            <img vspace=\"6\" hspace=\"9\" src=\"".profile_image_url($user->id)."\">
            </td></tr>
        ";
    }

    // If the user is viewing their own picture, display its status if it's not
    // yet verified.  This will tell them if other users can't view it yet, or
    // if there is a problem with it and they need to replace it.
    //
    if (profile_screening() && $profile->has_picture && $can_edit && $profile->verification!=1) {
        row1(offensive_profile_warning($profile->verification));
    }

    // Setup text output options based on logged in user forum settings
    //
    BoincForumPrefs::lookup($logged_in_user);
    $options = get_output_options($logged_in_user);
    $options->htmlitems = false;

    if (!empty($profile->response1)) {
        row1(show_profile_heading1());
        row1(output_transform($profile->response1, $options), 2, "foobar");
    }

    if (!empty($profile->response2)) {
        row1(show_profile_heading2());
        row1(output_transform($profile->response2, $options), 2, "foobar");
    }

    if (!$can_edit and !$screen_mode) {
        row1(tra("Your feedback on this profile"));
        row2(
            tra("Recommend this profile for User of the Day:"),
            tra("I %1 like %2 this profile", "<a href=\"profile_rate.php?userid=".$user->id."&vote=recommend\">", "</a>")
        );
        row2(
            tra("Alert administrators to an offensive profile:"),
            tra("I %1 do not like %2 this profile", "<a href=\"profile_rate.php?userid=".$user->id."&vote=reject\">", "</a>")
        );
    }
}


?>
