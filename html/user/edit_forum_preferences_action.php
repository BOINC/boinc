<?php
/**
 * This page commits the chanegs made in edit_forum_preferences_form.php to
 * the database.
 **/


require_once("../inc/forum.inc");
require_once("../inc/image.inc"); // Avatar scaling
require_once("../inc/forum_std.inc");

db_init();
$user = re_get_logged_in_user();

// If the user has requested a total reset of preferences:
$dbhandler = $mainFactory->getDatabaseHandler();
if (post_str("action", true)=="reset"){
    $post_count = $user->getPostcount();
    $special_user = $user->getSpecialUser();
    $dbhandler->deleteUserPrefs($user);
    $user->resetPrefs();
    $user->setPostcount($post_count);	// Recreate postcount
    $user->setSpecialUser($special_user); // And recreate special user bitfield
    Header("Location: edit_forum_preferences_form.php");    
    exit;
}

$avatar_type = post_int("avatar_select");
$newfile=IMAGE_PATH.$user->getID()."_avatar.jpg";

// Update the user avatar
if ($avatar_type<0 or $avatar_type>3) $avatar_type=0;
if ($avatar_type==0){
    if (file_exists($newfile)){
        unlink($newfile);      //Delete the file on the server if the user
                              //decides not to use an avatar
                              // - or should it be kept?
    }
    $avatar_url="";
} elseif ($avatar_type==2){
    if ($_FILES['picture']['tmp_name']!=""){
        $file=$_FILES['picture']['tmp_name'];
        $size = getImageSize($file);
        if ($size[2]!=2 and $size[2]!=3){
            //Not the right kind of file
            error_page("Error: Not the right kind of file, only PNG and JPEG are supported.");
        }
        $width = $size[0];
        $height = $size[1];
        $image2 = intelligently_scale_image($file, 100, 100);
        ImageJPEG($image2, $newfile);
    }
    if (file_exists($newfile)){
        $avatar_url=IMAGE_URL.$user->getID()."_avatar.jpg"; //$newfile;
    } else {
        //User didn't upload a compatible file or it went lost on the server
        $avatar_url="";
    }
}
$user->setAvatar($avatar_url);

// Update some simple prefs that are either on or off
$images_as_links = ($HTTP_POST_VARS["forum_images_as_links"]!="");
$link_externally = ($HTTP_POST_VARS["forum_link_externally"]!="");
$hide_avatars = ($HTTP_POST_VARS["forum_hide_avatars"]!="");
$hide_signatures = ($HTTP_POST_VARS["forum_hide_signatures"]!="");
$jump_to_unread = ($HTTP_POST_VARS["forum_jump_to_unread"]!="");
$ignore_sticky_posts = ($HTTP_POST_VARS["forum_ignore_sticky_posts"]!="");
$signature_by_default = ($HTTP_POST_VARS["signature_enable"]!="");
$user->setImagesAsLinks($images_as_links);
$user->setLinkPopup($link_externally);
$user->setHideAvatars($hide_avatars);
$user->setHideSignatures($hide_signatures);
$user->setJumpToUnread($jump_to_unread);
$user->setIgnoreStickyPosts($ignore_sticky_posts);
$user->setSignatureByDefault($signature_by_default);

// Update the rating thresholds for display of posts
$low_rating_threshold = post_int("forum_low_rating_threshold");
$high_rating_threshold = post_int("forum_high_rating_threshold");
$user->setLowRatingThreshold($low_rating_threshold);
$user->setHighRatingThreshold($high_rating_threshold);

// Update the signature for this user
$signature = sanitize_html(stripslashes($HTTP_POST_VARS["signature"]));
if (strlen($signature)>250) {
    error_page("Your signature was too long, please keep it less than 250 chars");
}
$user->setSignature($signature);

// Sorting styles for different forum areas
$forum_sort = post_int("forum_sort");
$thread_sort = post_int("thread_sort");
$user->setForumSortStyle($forum_sort);
$user->setThreadSortStyle($thread_sort);

// Add users to the ignore list if any users are defined
$add_user_to_filter = ($HTTP_POST_VARS["add_user_to_filter"]!="");
if ($add_user_to_filter){
    $user_to_add = trim($HTTP_POST_VARS["forum_filter_user"]);
    if ($user_to_add!="" and $user_to_add==strval(intval($user_to_add))){
	$user->addIgnoredUser(newUser($user_to_add));
    }
}

// Or remove some from the ignore list
$ignored_users = $user->getIgnorelist();
for ($i=0;$i<sizeof($ignored_users);$i++){
    if ($HTTP_POST_VARS["remove".trim($ignored_users[$i])]!=""){
	//this user will be removed and no longer ignored
	$user->removeIgnoredUser(newUser($ignored_users[$i]));
    }
}
// Update preferences for the "Display only the Y last posts if there are more than X posts in the thread" feature
$minimum_wrap_postcount = post_int("forum_minimum_wrap_postcount");
$display_wrap_postcount = post_int("forum_display_wrap_postcount");
if ($minimum_wrap_postcount<0) $minimum_wrap_postcount=0;
if ($display_wrap_postcount>$minimum_wrap_postcount) $display_wrap_postcount=round($minimum_wrap_postcount/2);
if ($display_wrap_postcount<5) $display_wrap_postcount=5;
$user->setMinimumWrapPostcount($minimum_wrap_postcount);
$user->setDisplayWrapPostcount($display_wrap_postcount);


// If we get down here everything went ok so let's redirect the user to the setup page again
// so that they can view their new preferences in action in the previews.
Header("Location: edit_forum_preferences_form.php");

?>
