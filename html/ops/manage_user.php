<?php
/***********************************************************************\
 * Manage user settings
 *  
 * Displays user settings, allows one to control special user status
 * and forum suspension (banishment).   Put this in html/ops
 *
\***********************************************************************/

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/forum.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/profile.inc");
require_once("../project/project.inc");

// Stuff for user roles and wiki interface.
// (Should work even if you comment these out)
/**
require_once("../include/roles.php");
require_once("../include/mediawiki.php");
**/


db_init();

$logged_in_user = get_logged_in_user(true);
$logged_in_user= getForumPreferences($logged_in_user);

if( function_exists('is_Administrator') ){ // only (now) on Pirates@Home
    if( !is_Administrator($logged_in_user) ){ 
        error_page("You must be a project administrator to use this page.");
    }    
 }

$id = get_int("userid", true);
if(!isset($id) ) $id = post_int("userid", true);

if( !$id || !is_numeric($id) || $id <= 0 ) {
    error_page("Must specify a userid");
}


// Look up the user

$user=lookup_user_id($id);
if( !$user ) {
    error_page("Cannot find user $id");
}

$self = $_SERVER['PHP_SELF'];
$Nbf = sizeof($special_user_bitfield);


/**
 * Process special user settings
 */

if( isset($_POST['special_user']) ){
    $bits="";
    for($i=0;$i<$Nbf;$i++) {
        $bits .= $_POST['special_user_'.$i] ? "1" : "0" ;
    }
    $q = "UPDATE forum_preferences SET special_user=\"$bits\" WHERE userid=$id";
    mysql_query($q);
}


/**
 * Process a suspension:
 */

if( isset($_POST['suspend_submit']) ){
    $dt = post_int('suspend_for',true);
    if( user_has_permission('moderator') ){
        $reason = $_POST['suspend_reason'];
        if( $dt > 0 && empty($reason)  ) {
            error_page("You must supply a reason for a suspension.
                <p><a href='$self?userid=$user->id'>Try again</a>");
        } else {
            if( is_numeric($dt) ) {
                $t = time()+$dt;
                $q = "UPDATE forum_preferences SET banished_until=$t WHERE userid=$id";
                mysql_query($q);

                /* put a timestamp in wiki to trigger re-validation of credentials */

                if( function_exists('touch_wiki_user') ){
                    touch_wiki_user($user);  
                }

                /* Send suspension e-mail to user and administrators */

                if($dt>0){
                    $subject = PROJECT." posting privileges suspended for ". $user->name;
                    $body = "
Forum posting privileges for the " .PROJECT. " user \"".$user->name."\"
have been suspended for " .time_diff($dt). " by ".$logged_in_user->name.". 
The reason given was:
  
   $reason
   
The suspension will end at " .time_str($t)."\n";
                }
                else {
                    $subject = PROJECT." user ". $user->name. " unsuspended";
                    $body = "
Forum posting privileges for the " .PROJECT. " user \"".$user->name."\"
have been restored by ".$logged_in_user->name."\n";
                    if($reason) $body.="The reason given was:\n\n   $reason\n";
                }

                send_email($user, $subject, $body);

                $emails = explode(",", POST_REPORT_EMAILS);
                foreach ($emails as $email) {
                    $admin->email_addr = $email;
                    send_email($admin, $subject, $body);
                }

            }//
        }
    }
}// suspend_submit



// Now update from whatever was set

$user=getForumPreferences($user);


/********************************
 * Output:
 */

admin_page_head("User Management: $user->name");
echo "\n<link rel='stylesheet' type=text/css href='". URL_BASE. "new_forum.css'>\n";
echo "\n<link rel='stylesheet' type=text/css href='" .URL_BASE. "arrgh.css'>\n";

if (!defined("POST_REPORT_EMAILS")) {
  echo "<p><font color='RED'>
   There is no addministrative e-mail address defined for reporting problems
or abuse in the forums.  Please define POST_REPORT_EMAILS in project.inc
        </font></p>\n";
 }

echo "<form name='manage_user' action='$self' method='POST'>
        <input type='hidden' name='userid' value='$id'>  \n";

start_table();
row1("<b>User: </b> ".$user->name
     . "<div align='right'>
        <input name='manage_user' type='submit' value='Update'></div>");
show_user_summary_public($user);
show_profile_link($user);
project_user_summary($user);
//row2("E-mail:", $user->email_addr);
end_table();
project_user_page_private($user);

echo "</form>\n";


/**********************
 * Special User status:
 */

echo "\n\n<P>
   <table width='100%'><tr>
   <td width='50%' valign='TOP'> \n";

echo "<form name='special_user' action='$self' method=\"POST\">
        <input type='hidden' name='userid' value='$id'>  \n";

start_table();
row1("Special User Status: $user->name", $Nbf );

echo "<tr>\n";
for($i=0;$i<$Nbf;$i++) {
    $bit = substr($user->special_user, $i, 1);
    echo "<tr><td><input type='checkbox'' name='special_user_".$i."' value='1'";
    if ($bit == 1) {
        echo " checked='checked'";
    }
    echo ">". $special_user_bitfield[$i] ."</td></tr>\n";
}
echo "</tr>";

echo "</tr><td colspan=$Nbf align='RIGHT'>
        <input name='special_user' type='SUBMIT' value='Apply'>
        </td></tr>\n";
end_table();
echo "</form>\n";

echo "\n\n</td><td valign='TOP'>\n\n";



/**********************
 * Suspended posting privileges
 */

//function suspend_user_form($user) {

echo "<form name='banishment' action='$self' method=\"POST\">
        <input type='hidden' name='userid' value='$user->id'>  \n";
start_table();
row1("Suspension: $user->name");

if( $user->banished_until ) {
    $dt = $user->banished_until - time();
    if( $dt > 0 ) {
        $x = " Suspended until " . time_str($user->banished_until)
            ."<br/> (Expires in " . time_diff($dt) .")" ;
    }
    else {
        $x = " last suspended " . time_str($user->banished_until);
    }
    row1($x);
}

echo "<tr><td>
Suspend user for:
 <blockquote>
        <input type='radio' name='suspend_for' value='3600'> 1 hour   <br/>
        <input type='radio' name='suspend_for' value='7200'> 2 hours  <br/>
        <input type='radio' name='suspend_for' value='18000'> 5 hours  <br/>
        <input type='radio' name='suspend_for' value='36000'> 10 hours  <br/>
        <input type='radio' name='suspend_for' value='86400'> 24 hours  <br/>
        <input type='radio' name='suspend_for' value='172800'> 48 hours  <br/>";

if( is_Administrator($logged_in_user) ){ // in case we are only a moderator
    echo "
        <input type='radio' name='suspend_for' value='",86400*7,"'> 1 week  <br/>
        <input type='radio' name='suspend_for' value='",86400*14,"'> 2 weeks  <br/>
";
}


if($dt>0) {
    echo "
        <input type='radio' name='suspend_for' value='-1'>  <b>unsuspend</b>   <br/>";
}
echo "
 </blockquote>

";

echo "<P>Reason (required):\n";
echo "<textarea name='suspend_reason' cols='40' rows='4'></textarea>";
echo "<br><font size='-2' >The reason will be sent to both the user
        and to the project administrators.</font>\n";


echo "<p align='RIGHT'><input name='suspend_submit' type='SUBMIT' value='Apply'></P>\n";
echo " </td></tr>\n";

end_table();
echo "</form>\n";

echo "</td></tr>
        </table>\n";


if($q) {
    echo "<P><font color='grey'>Query: $q </font>";
}



admin_page_tail();

$cvs_version_tracker[]=        //Generated automatically - do not edit
    "\$Id$"; 
?>
