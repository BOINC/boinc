<?php

require_once("util.inc");
require_once("project_specfici/project.inc");

// TODO: Determine if we can always assume these will be the same number.
define('SMALL_IMG_WIDTH', 64);
define('SMALL_IMG_HEIGHT', 64);

define('MAX_IMG_WIDTH', 800);
define('MAX_IMG_HEIGHT', 600);

define('MAX_DESC_LENGTH', 80);

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
<font size=2><b>NOTE: </b>Please keep your image small (less than 50K bytes)<br></font></td>
</tr>";
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
<font size=2><b>NOTE: </b>Please keep your image small (less than 50K bytes)<br></font>
</td></tr>
");
    }
    
    
}

function show_language_selection() {
    global $profile_info;

    row1("Select Your Primary Language");
    rowify("Selecting a language will help others with the same language preference to find each others' profiles and message board postings."); 
    echo "<tr><td>";
    if ($profile_info) {
        show_select("language", LANGUAGE_FILE, $profile_info['language']);
    } else {
        show_select("language", LANGUAGE_FILE, "English");
    }
    echo "</td></tr>\n";
}

function show_finale() {
    row1("Submit Your Profile");
    rowify("
    <br><input type=\"submit\" value=\"Submit\" name=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type=\"reset\" value=\"Clear\" name=\"clear\">");
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

    if (strlen($response1)==0 &&
        strlen($response2)==0 &&
        $delete_pic != "on" &&
        !is_uploaded_file($_FILES['picture']['tmp_name'])
    ) {
        profile_error_page("Your profile submission was empty.");
        exit();
    }

    // TODO: Having the delete checkbox and pic file form might confuse users.
    // Should figure out a better way to handle this.    Also might want to
    // present some sort of verification dialog (javascript).

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
            ImageJPEG($images[0], IMAGE_PATH . $user->id . '.jpg', 10);
            ImageJPEG($images[1], IMAGE_PATH . $user->id . '_sm.jpg', 10);
    }
    if ($profile_info) {

        $query = 'UPDATE profile SET '
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." language = '$language',"
            ." has_picture = '$hasPicture'"
            ." WHERE userid = '$user->id'";
    } else {

        $query = 'INSERT INTO profile SET '
            ." userid = '$user->id',"
            ." language = '$language',"
            ." response1 = '$response1',"
            ." response2 = '$response2',"
            ." has_picture = '$hasPicture'";
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
        
    
    /*($width > $height)? $scalar = ($width / SMALL_IMG_WIDTH) : $scalar = ($height / SMALL_IMG_HEIGHT);

    $dest_width = $width / $scalar;
    $dest_height = $height / $scalar;
    $horiz_offset = (SMALL_IMG_WIDTH - $dest_width) / 2;
    $vert_offset = (SMALL_IMG_HEIGHT - $dest_height) / 2;

    // TODO: Switch once GD 2.0+ is installed.
    //$smallImage = ImageCreateTrueColor(SMALL_IMG_WIDTH, SMALL_IMG_HEIGHT);
    $smallImage = ImageCreate(SMALL_IMG_WIDTH, SMALL_IMG_HEIGHT);

    // TODO: Switch once GD 2.0+ is installed.
    //ImageCopyResampled($smallImage, $image, $horiz_offset, $vert_offset, 0, 0, $dest_width, $dest_height, $width, $height);
    ImageCopyResized($smallImage, $image, $horiz_offset, $vert_offset, 0, 0, $dest_width, $dest_height, $width, $height);
    */
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

    ($origWidth > $origHeight)? $scalar = ($origWidth / $targetWidth) : $scalar = ($height / $targetHeight);

    $dest_width = $origWidth / $scalar;
    $dest_height = $origHeight / $scalar;
    $horiz_offset = ($targetWidth - $dest_width) / 2;
    $vert_offset = ($targetHeight - $dest_height) / 2;

    // TODO: Switch once GD 2.0+ is installed.
    //$newImage = ImageCreateTrueColor($targetWidth, $targetHeight);
    $newImage = ImageCreate($targetWidth, $targetHeight);

    // TODO: Switch once GD 2.0+ is installed.
    //ImageCopyResampled($newImage, $image, $horiz_offset, $vert_offset, 0, 0, $dest_width, $dest_height, $origWidth, $origHeight);
    ImageCopyResized($newImage, $image, $horiz_offset, $vert_offset, 0, 0, $dest_width, $dest_height, $origWidth, $origHeight);

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

?>
