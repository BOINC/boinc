<?php

require_once("db.inc");
require_once("util.inc");

// TODO: Determine if we can always assume these will be the same number.
define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 800);
define('MAX_IMG_HEIGHT', 600);

define('MAX_DESC_LENGTH', 90);

define('GALLERY_WIDTH', 7);
define('GALLERY_HEIGHT', 4);

define('UOTD_THRESHOLD', 7);

$user = NULL;
$profile_info = NULL;

db_init();

function show_profile_creation_page($usr) {
    global $user;
    global $profile_info;
    $user = $usr;

    // If the user already has a profile, fill in the fields with their current values.
    $result = mysql_query("SELECT * FROM profile WHERE userid = $user->id");
    if ($result) {
        $profile_info = mysql_fetch_array($result, MYSQL_ASSOC);
    }

    setup_form();
    $profile_info?page_head("Edit your Profile"):page_head("Create a Profile");
    start_table_noborder();
    show_description();
    show_questions();
    show_picture_option();
    show_finale();
    end_table();
    close_form();
    page_tail();
}

function setup_form() {
    if ($_POST['submit']) {
        process_create_results();
        exit();
    }

    echo "
<form action=", $_SERVER['PHP_SELF'], " method=\"POST\", ENCTYPE=\"multipart/form-data\">
";
}

function close_form() {
    echo "</form>";
}

function show_description() {
    global $profile_info;
    global $user;

    $profile_info?row1("Edit User Profile: " . $user->name):row1("Create a User Profile");
    rowify("
By creating a <b>user profile</b> you can share your opinions and background
with the entire ".PROJECT." community.
");
    rowify("<br>");
}

function show_questions() {
    show_profile_heading1();
    show_profile_question1();
    rowify("<br>");
    show_textarea('response1');
    rowify("<br>");
    show_profile_heading2();
    show_profile_question2();
    rowify("<br>");
    show_textarea('response2');
    rowify("<br>");
    show_language_selection();
    rowify("<br>");
}

function show_textarea($name) {
    global $profile_info;

    rowify("<textarea name=\"$name\" cols=80 rows=20>" . $profile_info[$name] . "</textarea>");
}

function show_picture_option() {
    global $profile_info;

    row1("Your Picture");

    if ($profile_info['has_picture']) {
        echo "
<tr><td colspan=2>
<table border=0 cellpadding=5
<tr>
<td><a href=\"" . IMAGE_PATH . $profile_info['userid'] . '.jpg' . "\"><img src=\"" . IMAGE_PATH . $profile_info['userid'] . '_sm.jpg' . "\"></a><p>
<input type=\"checkbox\" name=\"delete_pic\">Delete
</td>
<td>You have previously uploaded a picture of yourself to accompany your profile, shown at left.
If you would like to replace it, please click the \"Browse\" button and select the JPEG or PNG file
you would like to use instead.  If you would rather not have a picture
with your profile, click the \"Delete\" checkbox; your picture will be deleted as soon as you submit
this form.  If you're happy with your current picture, there is no need to do anything.<p>
<input name=\"picture\" type=\"file\"><br>
<font size=2><b>NOTE: </b>Please keep your image small (less than 50K bytes)<br></font></td></tr>";
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

function show_language_selection() {
    global $profile_info;

    row1("Select Your Primary Language");
    rowify("Selecting a language will help others with the same language preference to find each others' profiles and message board postings.");
    echo "<tr><td>";
    if ($profile_info) {
        show_combo_box("language", LANGUAGE_FILE, $profile_info['language']);
    } else {
        show_combo_box("language", LANGUAGE_FILE, "English");
    }
    echo "</td></tr>\n";
}

function show_finale() {
    global $profile_info;

    row1("Submit Your Profile");
    $email_check = "<input type=\"checkbox\" name=\"hide_email\"";
    if ($profile_info['hide_email']) {
        $email_check = $email_check . " checked";
    }
    $email_check = $email_check . ">Please do NOT show my email address in my profile.";
    rowify($email_check . "<p>
            <input type=\"submit\" value=\"Submit\" name=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type=\"reset\" value=\"Clear\" name=\"clear\">");
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

function process_create_results() {
    global $user;
    global $profile_info;

    $response1 = $_POST['response1'];
    $response2 = $_POST['response2'];
    $language = $_POST['language'];
    $delete_pic = $_POST['delete_pic'];
    $hide_email = ($_POST['hide_email'] == "on");

    if (strlen($response1)==0 &&
        strlen($response2)==0 &&
        $delete_pic != "on" &&
        $hide_email != "on" &&

        !is_uploaded_file($_FILES['picture']['tmp_name'])
    ) {
        profile_error_page("Your profile submission was empty.");
        exit();
    }

    if ($delete_pic == "on") {

        delete_user_pictures($profile_info['userid']);

        $profile_info['has_picture'] = false;
    }

    $profile_info ? $hasPicture = $profile_info['has_picture']: $hasPicture = false;

    if (is_uploaded_file($_FILES['picture']['tmp_name'])) {
        $hasPicture = true;

            /*
            echo "<br>Name: " . $_FILES['picture']['name'];
            echo "<br>Type: " . $_FILES['picture']['type'];
            echo "<br>Size: " . $_FILES['picture']['size'];
            echo "<br>Temp name: " . $_FILES['picture']['tmp_name'];
            */

            $images = getImages($_FILES['picture']['tmp_name']);

            // Write the original image file to disk.
            // TODO: define a constant for image quality.
            ImageJPEG($images[0], IMAGE_PATH . $user->id . '.jpg');
            ImageJPEG($images[1], IMAGE_PATH . $user->id . '_sm.jpg');
    }
    if ($profile_info) {

        $query = 'UPDATE profile SET '
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." language = '$language',"
            ." has_picture = '$hasPicture',"
            ." hide_email = '$hide_email'"
            ." WHERE userid = '$user->id'";
    } else {

        $query = 'INSERT INTO profile SET '
            ." userid = '$user->id',"
            ." language = '$language',"
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." has_picture = '$hasPicture',"
            ." hide_email = '$hide_email'";
    }

    $result = mysql_query($query);

    if (!$result) {
        profile_error_page("Couldn't create profile: database error!");
        exit();
    }

    show_result_page();
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

    $newImage = ImageCreateTrueColor($destWidth, $destHeight);

    ImageCopyResampled($newImage, $image, 0, 0, 0, 0, $destWidth, $destHeight, $origWidth, $origHeight);

    return $newImage;
}

function show_result_page() {
    global $user;

    page_head("Profile Saved");

    echo "
<h1>Congratulations!</h1><p>
Your profile was successfully entered into our database.<br><br>
<a href=view_profile.php?userid=$user->id>View your profile</a><br>
";

    page_tail();
}

// Builds a summary table of user profiles, writing it to $descriptor if it is
// available, or echoing if it is not.
//
//   $members is an array of userIDs;
//   $offset indicates which entry to begin the table with
//   $numToDisplay indicates how many profiles to display in this table
//   $cols indicates how many profile summaries should be written per row
//   $descriptor is an optional file descriptor to write the table to.

function show_user_table($members, $offset, $numToDisplay, $cols, $descriptor=null) {

    write_fd($descriptor, "<table class=bordered border=1 cellpadding=5>\n");

    $rows = ceil($numToDisplay / $cols);
    $count = $offset;
    $numMembers = count($members);

    for ($row = 0; $row < $rows; $row++) {
        if ($count >= $numMembers) {
            break;
        }

        write_fd($descriptor, "<tr>\n");

        for ($col = 0; $col < $cols; $col++) {
            if ($count < $numMembers) {
                write_fd($descriptor, "<td class=bordered width=7% height=64><center>");

                // Only link an image if the user has uploaded one.;
                $sql = "SELECT * FROM profile WHERE userid = " . $members[$count];
                $result = mysql_query($sql);
                $profile = mysql_fetch_assoc($result);

                if ($profile['has_picture']) {
                    write_fd($descriptor, "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $members[$count] . "\"><img src=\"" . URL_BASE . IMAGE_PATH . $members[$count] . '_sm.jpg' . "\"></a>");

                } else {
                    write_fd($descriptor, "&nbsp;");
                }

                write_fd($descriptor, "</center></td><td class=bordered width=33% height=64>\n". get_profile_summary($profile). "</td>");
                $count++;
            } else {
                write_fd($descriptor, "<td width=7% height=64></td><td width=33% height=64></td>");
            }
        }
        write_fd($descriptor, "</tr>\n");
    }
    write_fd($descriptor, "</table>\n");

}

// Generates a string containing:
//   1) the name of the user with ID == $userid, with a link to a view of their profile
//   2) the first MAX_DESC_LENGTH characters from the response1 field of said user's profile.

function get_profile_summary($profile) {
  $sql = "SELECT * FROM profile WHERE userid = " . $profile['userid'];
  $result = mysql_query($sql);
  $sql = "SELECT name FROM user WHERE id = " . $profile['userid'];
  $result2 = mysql_query($sql);

  if (!$result || !$result2) {
    echo "Database error!";  // Change this to a standard error page.
    exit();
  }

  $row = mysql_fetch_assoc($result);
  $row2 = mysql_fetch_assoc($result2);

  mysql_free_result($result);
  mysql_free_result($result2);

  $description = "";

  if (strlen($row['response1']) != 0) {
    $temp = $row['response1'];
    $description = "(\"" . sub_sentence(strip_tags($temp), ' ', MAX_DESC_LENGTH, true) . "\")";

  }

  $summary = "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $profile['userid'] . "\">" . $row2['name'] . "</a> " . $description;
  return $summary;
}

// Displays a user's profile (if they have one);

function show_profile($userid, $verify_mode=false) {
    if (!$userid) {
        profile_error_page("No user ID was specified.<p>");
        exit();
    }

    $user = get_user_from_id($userid);

    if (!$user) {
        profile_error_page("No user exists for that ID, or there was a database error.<p>");
        exit();
    }

    $result = mysql_query("SELECT * FROM profile WHERE userid = $user->id");

    if (mysql_num_rows($result) > 0) {
        $profile_info = mysql_fetch_array($result, MYSQL_ASSOC);
    } else {
    // TODO: Standardize this to a global error page.;
        echo "No user profile exists for that user ID.";
        exit();
    }
    if (!$verify_mode) {
        $logged_in_user = get_logged_in_user(false);  // (false) since anyone can look at profiles.;
    }
    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;

    if (!$verify_mode) {
        page_head("User Profile: ".$user->name);
    }

    if ($can_edit) {
        echo "<a href=create_profile.php>[Edit Your Profile]</a>";
    }

    start_table_noborder();
    echo "<tr><td>";
    show_profile_summary($user, $profile_info, $can_edit, $verify_mode);
    echo "</tr></td>";
    show_profile_heading1();
    echo "<tr><td>", $profile_info['response1'], "<br><br></td></tr>";
    show_profile_heading2();
    echo "<tr><td>", $profile_info['response2'], "</td></tr>";
    end_table();
    if (!$verify_mode) {
        page_tail();
    }
}

function show_profile_summary($user, $profile_info, $can_edit, $verify_mode) {
  echo "
<table border=0 cellpadding = 1 width=100%>\n
<tr><td><h1>$user->name</h1></td><td align=\"center\">";

  if (!$can_edit && !$verify_mode) {
    show_view_buttons($user->id);
  }

  echo "</td></tr>\n<tr><td colspan=\"2\">\n";

  // Only display an image if the user has uploaded one;
  if (!$verify_mode && $profile_info['has_picture']) {
    echo "<a href=\"" , URL_BASE, IMAGE_PATH , $user->id , '.jpg' . "\"><img align=left vspace=6 hspace=9 src=\"" , URL_BASE, IMAGE_PATH , $user->id , '_sm.jpg' . "\"></a>\n";
  } else if ($verify_mode && $profile_info['has_picture']) {
    echo "<a href=\"" , URL_BASE, IMAGE_PATH , $user->id , '.jpg' . "\"><img align=left vspace=6 hspace=9 src=\"" , URL_BASE, IMAGE_PATH , $user->id , '_sm.jpg' . "\"></a>\n";
  }

  echo "
<font size=\"-1\">
<b>Country:</b> ", $user->country, "&nbsp&nbsp<b>Language:</b> ", $profile_info['language'], "<br>";
  if (!$profile_info['hide_email']) {
      echo "<b>Email:</b> <a href=\"mailto:", $user->email_addr, "\">", $user->email_addr, "</a><br>";
  }
  echo "<b>Total Credit:</b> ", $user->total_credit, "<br>";

  if ($user->teamid) {
    $result = mysql_query("select * from team where id = $user->teamid");
    $team = mysql_fetch_object($result);
    echo "<b>Team:</b> <a href=team_display.php?teamid=$team->id>$team->name</a><br>";
  }
  echo "
<b>Date Registered:</b> ", date_str($user->create_time), "
</font>
</td></tr>
</table>
<br>\n";
}

function show_view_buttons($userid) {
  echo "
<form action=view_profile.php?userid=$userid method=\"POST\">
<input type=\"submit\" name=\"recommend\" value=\"RECOMMEND\">
<font size=-1><a href=\"javascript:;\" onClick=\"window.open ('explanation.php?val=recommend','_blank','width=350,height=200,left=50,top=150,menubar=0,directories=0,scrollbars=0,resizable=0,status=0')\">what is recommend?</a></font>
<br>
<input type=\"submit\" name=\"reject\" value=\"VOTE TO REJECT\">
<font size=-1><a href=\"javascript:;\" onClick=\"window.open ('explanation.php?val=reject','_blank','width=350,height=200,left=50,top=150,menubar=0,directories=0,scrollbars=0,resizable=0,status=0')\">what is vote to reject?</a></font>
</form>
";
}

function process_view_results($vote, $userid) {
  if ($vote != "recommend" && $vote != "reject") {
    echo "Invalid vote type.<br>";
    exit();
  }

  $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
  $profile = mysql_fetch_array($result);

  $newValue = $profile[$vote] + 1;
  $newresult = mysql_query("UPDATE profile SET $vote = $newValue WHERE userid = $userid");

  page_head("Vote Recorded");

  start_table_noborder();

  row1("Thank you");

  if ($vote == "recommend") {
    rowify("Your recommendation has been recorded.");
  } else {
    rowify("Your vote to reject has been recorded.");
  }
  end_table();
  echo "<br><a href=\"view_profile.php?userid=", $userid ,"\">Return to profile.</a>";

  page_tail();
}

?>
