<?php

require_once("../inc/db.inc");
require_once("../inc/user.inc");
require_once("../inc/profile.inc");
require_once("../inc/util.inc");
require_once("../inc/image.inc");
require_once("../inc/forum.inc");


db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);

$avatar_url = mysql_escape_string($HTTP_POST_VARS["avatar_url"]);
if (substr($avatar_url,0,4)!="http") $avatar_url="http://".$avatar_url;
$avatar_type = intval($HTTP_POST_VARS["avatar_select"]);
$newfile=IMAGE_PATH.$user->id."_avatar.jpg";
if ($avatar_type<0 or $avatar_type>3) $avatar_type=0;
if ($avatar_type==0){
    if (file_exists($newfile)){
        unset($newfile);      //Delete the file on the server if the user
                              //decides not to use an avatar
                              // - or should it be kept?
    }
    $avatar_url="";
} elseif ($avatar_type==2){
    if ($_FILES['picture']['tmp_name']!=""){
            $file=$_FILES['picture']['tmp_name'];
        $size = getImageSize($file);
//        print_r($size);flush();
        if ($size[2]!=2 and $size[2]!=3){
            //Not the right kind of file
            Echo "Error: Not the right kind of file, only PNG and JPEG are supported.";
            exit();
        }
        $width = $size[0];
        $height = $size[1];
        $image2 = intelligently_scale_image($file, 100, 100);
        ImageJPEG($image2, $newfile);
    }
    if (file_exists($newfile)){
        $avatar_url=IMAGE_URL.$user->id."_avatar.jpg"; //$newfile;
    } else {
        //User didn't upload a compatible file or it went lost on the server
        $avatar_url="";
    }
}

$image_as_link = ($HTTP_POST_VARS["forum_images_as_links"]!="");
$link_externally = ($HTTP_POST_VARS["forum_link_externally"]!="");
$hide_avatars = ($HTTP_POST_VARS["forum_hide_avatars"]!="");
$hide_signatures = ($HTTP_POST_VARS["forum_hide_signatures"]!="");
$jump_to_unread = ($HTTP_POST_VARS["forum_jump_to_unread"]!="");
$ignore_sticky_posts = ($HTTP_POST_VARS["forum_ignore_sticky_posts"]!="");
$low_rating_threshold = intval($HTTP_POST_VARS["forum_low_rating_threshold"]);
$high_rating_threshold = intval($HTTP_POST_VARS["forum_high_rating_threshold"]);
$add_user_to_filter = ($HTTP_POST_VARS["add_user_to_filter"]!="");

$no_signature_by_default=($HTTP_POST_VARS["signature_enable"]=="");
$signature = sanitize_html(stripslashes($HTTP_POST_VARS["signature"]));
if (strlen($signature)>250) {
    echo "You signature was too long, please keep it less than 250 chars";
    exit();
}
$signature = mysql_escape_string($signature); 

$forum_sort = $HTTP_POST_VARS["forum_sort"];
$thread_sort = $HTTP_POST_VARS["thread_sort"];
$faq_sort = $HTTP_POST_VARS["faq_sort"];
$answer_sort = $HTTP_POST_VARS["answer_sort"];
$forum_sorting=mysql_escape_string(implode("|",array($forum_sort,$thread_sort,$faq_sort,$answer_sort)));
$has_prefs=mysql_query("select * from forum_preferences where userid='".$user->id."'");

$ignorelist = $user->ignorelist;
if ($add_user_to_filter){					//see if we should add any users to the ignorelist
    $user_to_add = trim($HTTP_POST_VARS["forum_filter_user"]);
    if ($user_to_add!="" and $user_to_add==strval(intval($user_to_add))){
	$ignorelist.="|".$user_to_add;
    }
}

$ignored_users = explode("|",$ignorelist);			//split the list into an array
$ignored_users = array_unique($ignored_users);			//a user can only be on the list once
natsort($ignored_users);					//sort the list by userid in natural order
$ignored_users=array_values($ignored_users);			//reindex
$real_ignorelist = "";
for ($i=1;$i<sizeof($ignored_users);$i++){
    if ($ignored_users[$i]!="" and $HTTP_POST_VARS["remove".trim($ignored_users[$i])]!=""){
	//this user will be removed
    } else {
	//the user should be in the new list
	$real_ignorelist.="|".$ignored_users[$i];
    }
}

$result = mysql_query(
    "update forum_preferences set 
        avatar_type='".$avatar_type."', 
        avatar='".$avatar_url."', 
        images_as_links='".$image_as_link."', 
        link_popup='".$link_externally."', 
        hide_avatars='".$hide_avatars."', 
        no_signature_by_default='".$no_signature_by_default."', 
        ignore_sticky_posts='".$ignore_sticky_posts."', 
        sorting='".$forum_sorting."',
        signature='$signature',
        jump_to_unread='".$jump_to_unread."',
        hide_signatures='".$hide_signatures."',
        low_rating_threshold='".$low_rating_threshold."',
	ignorelist='".$real_ignorelist."',
        high_rating_threshold='".$high_rating_threshold."'
    where userid=$user->id"
);
if ($result) {
    echo mysql_error();
    Header("Location: edit_forum_preferences_form.php");
} else {
    page_head("Forum preferences update");
    echo "Couldn't update forum preferences.<br>\n";
    echo mysql_error();
    page_tail();
}

?>
