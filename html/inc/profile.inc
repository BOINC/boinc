<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/cache.inc");
require_once("../inc/user.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../inc/forum.inc");
require_once("../inc/recaptchalib.php");

define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 800);
define('MAX_IMG_HEIGHT', 600);

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
    return URL_BASE.IMAGE_URL.$userid.'.jpg';
}

function profile_thumb_url($userid) {
    return URL_BASE.IMAGE_URL.$userid.'_sm.jpg';
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
            <font size='+2' color='#33cc33'>
            Your profile will be made visible to other people<br>
            as soon as it has been approved by the project.<br>
            This may take up to a few days.<br>
            </font>
        ";
    } else if ($verify_flag == -1) {
        return "
            <font size='+2' color='#ff3333'>
            Your profile has been marked as unacceptable.<br>
            It is not visible to other people. Please change it.<br>
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
    $newGD = (strstr($gd_info["GD Version"], "2.0")!="");
    if ($newGD) {
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
    $user = get_user_from_id($profile->userid);

    if (!$user || !$profile) {
        echo "Database error!";  // Change this to a standard error page.
        exit();
    }

    $description = "";

    if (strlen($profile->response1) != 0) {
        $temp = $profile->response1;
        $description = "(\"" . sub_sentence(strip_tags($temp), ' ', MAX_DESC_LENGTH, true) . "\")";

    }

    $summary = "<a href=\"".URL_BASE."view_profile.php?userid=".$profile->userid."\">".$user->name."</a> ".$description;
    return $summary;
}

// Displays a user's profile (if they have one);
// $screen_mode is set if we're in the administrative profile-screening page,
// in which case we show everything

function show_profile($user, $logged_in_user, $screen_mode = false) {
    BoincForumPrefs::lookup($user);
    if (is_banished($user)) {
        error_page("User is banished");
    }
    $profile = get_profile($user->id);
    if (!$profile) {
        error_page("No user profile exists for that user ID.");
    }

    $can_edit = isset($logged_in_user) && $logged_in_user && $user->id == $logged_in_user->id;

    if ($can_edit) {
        row1("<a href=create_profile.php>Edit your profile</a>");
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
    if (!$screen_mode) {
        $logged_in_user = get_logged_in_user(false);
        BoincForumPrefs::lookup($logged_in_user);
        $options = get_transform_settings_from_user($logged_in_user);
    }
        
    row1(show_profile_heading1());
    row1(output_transform($profile->response1,$options), 2, "foobar");
    row1(show_profile_heading2());
    row1(output_transform($profile->response2,$options), 2, "foobar");

    if (!$can_edit and !$screen_mode) {
        row1("Your feedback on this profile");
        row2(
            "Recommend this profile for User of the Day:",
            "I <a href=profile_rate.php?userid=$user->id&vote=recommend>like</a> this profile"
        );
        row2(
            "Alert administrators to an offensive profile:",
            "I <a href=profile_rate.php?userid=$user->id&vote=reject>don't like</a> this profile"
        );
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
