<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/cache.inc");

// TODO: Determine if we can always assume these will be the same number.
define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 800);
define('MAX_IMG_HEIGHT', 600);

define('MAX_DESC_LENGTH', 90);

define('GALLERY_WIDTH', 7);
define('GALLERY_HEIGHT', 4);

function get_profile($userid) {
    $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
    if (!$result) {
        return NULL;
    }
    $profile = mysql_fetch_object($result);
    mysql_free_result($result);
    return $profile;
}

function show_profile_creation_page($user) {
    // If the user already has a profile,
    // fill in the fields with their current values.
    //
    $profile = get_profile($user->id);
    if ($_POST['submit']) {
        process_create_results($user, $profile);
        exit();
    }

    if ($profile) {
        page_head("Edit your Profile");
    } else {
        page_head("Create a Profile");
    }

    echo "
        <form action=", $_SERVER['PHP_SELF'], " method=\"POST\", ENCTYPE=\"multipart/form-data\">
    ";
    start_table_noborder();
    show_description($user, $profile);
    show_questions($profile);
    show_picture_option($profile);
    show_finale();
    end_table();
    echo "</form>";
    page_tail();
}

function show_description($user, $profile) {

    if ($profile) {
        row1("Edit your profile");
    } else {
        row1("Create a profile");
    }
    rowify("
Your <b>profile</b> lets you share your opinions and background
with the ".PROJECT." community.
<br><a href=html.php><font size=-2>You may use HTML tags</font></a>
");
    rowify("<br>");
}

function show_questions($profile) {
    row1(show_profile_heading1());
    rowify(show_profile_question1());
    rowify("<br>");
    show_textarea("response1", $profile->response1);
    rowify("<br>");
    row1( show_profile_heading2());
    rowify( show_profile_question2());
    rowify("<br>");
    show_textarea("response2", $profile->response2);
    rowify("<br>");
    show_language_selection($profile);
    rowify("<br>");
}

function show_textarea($name, $text) {
    rowify("<textarea name=\"$name\" cols=80 rows=20>" . $text . "</textarea>");
}

function show_picture_option($profile) {

    row1("Your Picture");

    if ($profile->has_picture) {
        echo "
<tr><td colspan=2>
<table border=0 cellpadding=5
<tr>
<td><a href=\"" . IMAGE_URL . $profile->userid . '.jpg' . "\"><img src=\"" . IMAGE_URL . $profile->userid . '_sm.jpg' . "\"></a><p>
<input type=\"checkbox\" name=\"delete_pic\">Delete
</td>
<td>You have previously uploaded a picture of yourself
to accompany your profile, shown at left.
If you would like to replace it,
please click the \"Browse\" button and select the JPEG or PNG file
you would like to use instead.
If you would rather not have a picture with your profile,
click the \"Delete\" checkbox;
your picture will be deleted as soon as you submit this form.
If you're happy with your current picture,
there is no need to do anything.<p>
<input name=\"picture\" type=\"file\"><br>
<font size=2>
<b>NOTE: </b>Please keep your image small (less than 50K bytes)<br></font></td></tr>";
        rowify("<br>");
        end_table();
        echo "</td></tr>";
    }
    else {
        rowify("
Do you have a picture of yourself on your computer?
If you would like us to include it with your profile,
please click the \"Browse\" button and select the JPEG or PNG file
you want to send. (No animated GIFs, please!)<br>
<input name=\"picture\" type=\"file\"><br>
<font size=2><b>NOTE: </b>Please keep your image small (less than 50K bytes)<br></font></td></tr>");
        rowify("<br>");
    }
}

function show_language_selection($profile) {
    row1("Select Your Primary Language");
    rowify("Selecting a language will help others with the same language preference to find each others' profiles and message board postings.");
    echo "<tr><td>";
    if (strlen($profile->language)) {
        show_combo_box("language", LANGUAGE_FILE, $profile->language);
    } else {
        show_combo_box("language", LANGUAGE_FILE, "English");
    }
    echo "</td></tr>\n";
}

function show_finale() {
    row1("Submit Your Profile");
    rowify("<p>
        <input type=submit value=Submit name=submit>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        <input type=reset value=Clear name=clear>"
    );
}

// If the user with id = $userid has uploaded a picture his/herself,
// delete it and its thumbnail.
//
function delete_user_pictures($userid) {
    $filename1 = IMAGE_PATH . $userid . '.jpg';
    $filename2 = IMAGE_PATH . $userid . '_sm.jpg';

    if (file_exists($filename1)) {
        unlink($filename1);
    }
    if (file_exists($filename2)) {
        unlink($filename2);
    }
}

function process_create_results($user, $profile) {
    $response1 = $_POST['response1'];
    $response2 = $_POST['response2'];
    $language = $_POST['language'];
    $delete_pic = $_POST['delete_pic'];

    if (strlen($response1)==0 &&
        strlen($response2)==0 &&
        $delete_pic != "on" &&
        !is_uploaded_file($_FILES['picture']['tmp_name'])
    ) {
        profile_error_page("Your profile submission was empty.");
        exit();
    }

    if ($delete_pic == "on") {
        delete_user_pictures($profile->userid);
        $profile->has_picture = false;
    }

    $profile ? $hasPicture = $profile->has_picture: $hasPicture = false;

    if (is_uploaded_file($_FILES['picture']['tmp_name'])) {
        $hasPicture = true;

        // echo "<br>Name: " . $_FILES['picture']['name'];
        // echo "<br>Type: " . $_FILES['picture']['type'];
        // echo "<br>Size: " . $_FILES['picture']['size'];
        // echo "<br>Temp name: " . $_FILES['picture']['tmp_name'];

        $images = getImages($_FILES['picture']['tmp_name']);

        // Write the original image file to disk.
        // TODO: define a constant for image quality.
        ImageJPEG($images[0], IMAGE_PATH . $user->id . '.jpg');
        ImageJPEG($images[1], IMAGE_PATH . $user->id . '_sm.jpg');
    }
    $response1 = sanitize_html($response1);
    $response2 = sanitize_html($response2);
    if ($profile) {
        $query = 'UPDATE profile SET '
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." language = '$language',"
            ." has_picture = '$hasPicture'"
            ." WHERE userid = '$user->id'";
        $result = mysql_query($query);
        if (!$result) {
            profile_error_page("Couldn't update profile: database error");
            exit();
        }
    } else {
        $query = 'INSERT INTO profile SET '
            ." userid = '$user->id',"
            ." language = '$language',"
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." has_picture = '$hasPicture',"
            ." verification=0";
        $result = mysql_query($query);
        if (!$result) {
            profile_error_page("Couldn't create profile: database error");
            exit();
        }
        mysql_query("update user set has_profile=1 where id=$user->id");
    }

    show_result_page($user);
}

// Returns an array containing:
//     [0]: The original image refered to by $fileName if its dimensions are
//                less than MAX_IMG_WIDTH x MAX_IMG_HEIGHT, or a version scaled to
//                those dimensions if it was too large.
//     [1]: A scaled version of the above.

function getImages($fileName) {
    $size = getImageSize($fileName);

    // Determine if the filetype uploaded is supported.
    // TODO: Change these to constants.
    switch($size[2]) {
        case '2':    // JPEG
            $image = imageCreateFromJPEG($fileName);
            break;
        case '3':    // PNG
            $image = imageCreateFromPNG($fileName);
            break;
        default:
            profile_error_page("The format of your uploaded image is not supported by our system.");
            exit();
    }

    $width = $size[0];
    $height = $size[1];

    $smallImage = scale_image($image, $width, $height, SMALL_IMG_WIDTH, SMALL_IMG_HEIGHT);

    if ($width > MAX_IMG_WIDTH || $height > MAX_IMG_HEIGHT) {
        $image = scale_image($image, $width, $height, MAX_IMG_WIDTH, MAX_IMG_HEIGHT);
    }

    /*
    echo "<br><br>Image type: $size[2]";
    echo "<br>Original width: $width";
    echo "<br>Original height: $height";
    echo "<br>Scalar: $scalar";
    echo "<br>Dest width: " . ($width / $scalar);
    echo "<br>Dest height: " . ($height / $scalar);
    echo "<br>Horizontal offset: $horiz_offset";
    echo "<br>Vertical offset: $vert_offset";
    echo "<br><br><a href=\"images/user_profile/test.jpg\">View result</a>";
    */

    return array($image, $smallImage);
}

function scale_image($image, $origWidth, $origHeight, $targetWidth, $targetHeight) {

    // If the image is already smaller than the target dimensions, just return it.
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

if (1) {
    // If you are using a modern PHP/GD installation that does 'truecolor' images, this is what's needed.
    $newImage = ImageCreateTrueColor($destWidth, $destHeight);
    ImageCopyResampled($newImage, $image, 0, 0, 0, 0, $destWidth, $destHeight, $origWidth, $origHeight);
} else {
    // If not, use this block.  The image quality is lower but it works using older PHP/GD versions.
    $newImage = ImageCreate($destWidth, $destHeight);
    ImageCopyResized($newImage, $image, 0, 0, 0, 0, $destWidth, $destHeight, $origWidth, $origHeight);
}

    return $newImage;
}

function show_result_page($user) {
    page_head("Profile Saved");

    echo "
<h2>Congratulations!</h2><p>
Your profile was successfully entered into our database.<br><br>
<a href=view_profile.php?userid=$user->id>View your profile</a><br>
";

    page_tail();
}

// Builds a summary table of user profiles.
//
//   $members is an array of userIDs;
//   $offset indicates which entry to begin the table with
//   $numToDisplay indicates how many profiles to display in this table
//   $cols indicates how many profile summaries should be written per row
//   $descriptor is an optional file descriptor to write the table to.

function show_user_table($members, $offset, $numToDisplay, $cols) {
    echo "<table class=bordered border=1 cellpadding=5>\n";

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

                // Only link an image if the user has uploaded one.;
                if ($profile->has_picture) {
                    echo "<a href=\"", URL_BASE, "view_profile.php?userid={$members[$count]}\"><img src=\"", URL_BASE, IMAGE_URL, "{$members[$count]}_sm.jpg\"></a>";

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
    echo "</table>\n";

}

// Generates a string containing:
//   1) the name of the user with ID == $userid, with a link to a view of their profile
//   2) the first MAX_DESC_LENGTH characters from the response1 field of said user's profile.

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

    $summary = "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $profile->userid . "\">" . $user->name . "</a> " . $description;
    return $summary;
}

// Displays a user's profile (if they have one);

function show_profile($userid, $verify_mode = FALSE) {
    $user = get_user_from_id($userid);

    if (!$user) {
        profile_error_page("No user exists for that ID, or there was a database error.<p>");
        exit();
    }
    $profile = get_profile($userid);
    if (!$profile) {
        echo "No user profile exists for that user ID.";
        exit();
    }
    if (!$verify_mode) {
        $logged_in_user = get_logged_in_user(false);
        if (!$logged_in_user || ($user->id != $logged_in_user->id)) {
            $caching = true;
            $cache_args = "userid=$userid";
            start_cache(USER_PROFILE_TTL,$cache_args);
        }
    }

    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;
    if (!$verify_mode) {
        page_head("Profile: ".$user->name);
    }

    start_table();

    if ($can_edit) {
        row1("<a href=create_profile.php>Edit your profile</a>");
    }
    

    if ($profile->has_picture) {
        echo "
            <tr><td colspan=2 align=center>
            <img vspace=6 hspace=9 src=\"" , URL_BASE, IMAGE_URL , $user->id , '.jpg' . "\">
            </td></tr>
        ";
    }

    row2("Country", $user->country);
    row2("Language", $profile->language);
    row2("Total Credit", $user->total_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row2("Team", "<a href=team_display.php?teamid=$team->id>$team->name</a><br>");
    }
    row2("Date Registered", date_str($user->create_time));

    row1(show_profile_heading1());
    row1($profile->response1, 2, "foobar");
    row1(show_profile_heading2());
    row1($profile->response2, 2, "foobar");
    if (!$can_edit and !$verify_mode) {
        row1("Your feedback on this profile");
        row2(
            "Recommend this profile for User of the Day:",
            "I <a href=profile_rate.php?userid=$userid&vote=recommend>like</a> this profile"
        );
        row2(
            "Alert administrators to an offensive profile:",
            "I <a href=profile_rate.php?userid=$userid&vote=reject>don't like</a> this profile"
        );
    }

    end_table();
    if (!$verify_mode) {
        page_tail();
    } else {
        echo "<hr>";
    }
    if ($caching) end_cache(USER_PROFILE_TTL,$cache_args);
}
?>
