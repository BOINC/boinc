<?php

require_once("util.inc");
require_once("project_specific/project.inc");
require_once("sanitize_html.inc");

// TODO: Determine if we can always assume these will be the same number.
define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 800);
define('MAX_IMG_HEIGHT', 600);

define('MAX_DESC_LENGTH', 90);

define('GALLERY_WIDTH', 7);
define('GALLERY_HEIGHT', 4);

$user = NULL;
$profile_info = NULL;

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
        process_results();
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
you would like to use instead. (No animated GIFs, please!)    If you would rather not have a picture
with your profile, click the \"Delete\" checkbox.    If you like your current picture, there is no need
to do anything.<p>
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
    row1("Submit Your Profile");
    rowify("<input type=\"checkbox\" name=\"hide_email\">Please do NOT show my email address in my profile.<p>
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

function process_results() {
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
//   $pathMod is an optional path which is prepended to the image path.  Useful
//     when the image path is not relative to the directory from which this
//     function is called.

function show_user_table($members, $offset, $numToDisplay, $cols, $descriptor=null, $pathMod=null) {

  // TODO: Would be nice if we could open a stream to stdout to avoid
  // all the redundant $descriptor checks.  Once the server is running
  // PHP 5+, might want to try switching over to fprintf(...).
    
  if ($descriptor) {
    fwrite($descriptor, "<table border=1 cellpadding=5>\n");
  } else {
    echo "<table border=1 cellpadding=5>";
  }
  
  $rows = ceil($numToDisplay / $cols);
  $count = $offset;
  $numMembers = count($members);
  
  for ($row = 0; $row < $rows; $row++) {
    if ($count >= $numMembers) {
      break;
    }
    
    if ($descriptor) {
      fwrite($descriptor, "<tr>\n"); 
    } else {
      echo "<tr>";
    }
    
    // Formatting is a table with two columns of user summaries.
    for ($col = 0; $col < $cols; $col++) {
      if ($count < $numMembers) {
	if ($descriptor) {
	  fwrite($descriptor, "<td width=7% height=64><center>");
	} else {
	  echo "<td width=7% height=64><center>";
	}
	
	// Only link an image if one exists.
	if (file_exists($pathMod . IMAGE_PATH . $members[$count] . '_sm.jpg')) {
	  if ($descriptor) {
	    fwrite($descriptor, "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $members[$count] . "\"><img src=\"" . '../' . IMAGE_PATH . $members[$count] . '_sm.jpg' . "\"></a>");
	  } else {
	    echo "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $members[$count] . "\"><img src=\"" . IMAGE_PATH . $members[$count] . '_sm.jpg' . "\"></a>";
	  }
	  
	} else {
	  if ($descriptor) {
	    fwrite($descriptor, "&nbsp;");
	  } else {
	    echo "&nbsp;";
	  }
	}
	if ($descriptor) {
	  fwrite($descriptor, "</center></td><td width=33% height=64>");
	  fwrite($descriptor, get_profile_summary($members[$count]));
	  fwrite($descriptor, "</td>");
	} else {
	  echo "</center></td><td width=33% height=64>\n", get_profile_summary($members[$count]), "</td>";
	}
	
	$count++;
      }
      else {
	// Empty entry
	if ($descriptor) {
	  fwrite($descriptor, "<td width=7% height=64></td><td width=33% height=64></td>");
	} else {
	  echo "<td width=7% height=64></td><td width=33% height=64></td>";
	}
      }
    }
    if ($descriptor) {
      fwrite($descriptor, "</tr>\n");
    } else {
      echo "</tr>";
    }
  }
  if ($descriptor) {
    fwrite($descriptor, "</table>\n");
  } else {
    echo "</table>";
  }
}

// Generates a string containing: 
//   1) the name of the user with ID == $userid, with a link to a view of their profile; 
//   2) the first MAX_DESC_LENGTH characters from the response1 field of said user's profile.

function get_profile_summary($userid) {
  $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
  $result2 = mysql_query("SELECT name FROM user WHERE id = $userid");

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

    $description = "(\"" . substr(sanitize_html($temp), 0, MAX_DESC_LENGTH);
    if (strlen($row['response1']) >= MAX_DESC_LENGTH) {
      $description = $description . "...";
    }
    $description = $description . "\")";
  }

  $summary = "<a href=\"" . URL_BASE . "view_profile.php?userid=" . $userid . "\">" . $row2['name'] . "</a> " . $description;
  return $summary;
}

?>
