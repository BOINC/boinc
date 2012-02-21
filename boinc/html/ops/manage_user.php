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

// Manage user settings
//  
// Displays user settings, allows one to control special user status
// and forum suspension (banishment).   Put this in html/ops,
// (or could be used by moderators for bans < 24 hrs).


// TODO: use DB abstraction layer

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");
require_once("../inc/forum.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/profile.inc");
require_once("../project/project.inc");

db_init();

$is_admin = true;
$Nbf = sizeof($special_user_bitfield);

// Delete a user (or at least try to)
//
function delete_user($user){
    global $delete_problem;
  
    if (!empty($user->teamid)){
        user_quit_team($user);
        #$delete_problem .= "Removed user from team.<br/>";
    }
    if ($user->has_profile){
        mysql_query("DELETE FROM profile WHERE userid = $user->id");
        delete_user_pictures($user->id);
        mysql_query("UPDATE user SET has_profile=0 WHERE id=$user->id");
        #$delete_problem .= "Deleted profile.<br/>";
    }

    if ($user->total_credit > 0.0){
        $delete_problem .= "Cannot delete user: User has credit.<br/>";
        return false;
    }  

    // Don't delete user if they have any outstanding Results
    //
    $q = "SELECT COUNT(*) AS count FROM result WHERE userid=".$user->id;
    $result = mysql_query($q);
    $c = mysql_fetch_object($result);
    mysql_free_result($result);
    if ($c->count) {
        $delete_problem .= "Cannot delete user: User has ". $c->count.
            " Results in the database.<br/>";
    }

    // Don't delete user if they have posted to the forums
    //
    $q = "SELECT COUNT(*) AS count FROM post WHERE user=".$user->id;
    $result = mysql_query($q);
    $c = mysql_fetch_object($result);
    mysql_free_result($result);
    if ($c->count) {
        $delete_problem .= "Cannot delete user: User has ". $c->count.
            " forum posts.<br/>";
    }
    if ($delete_problem) return false;

    $q = "DELETE FROM user WHERE id=".$user->id;
    $result = mysql_query($q);
    $delete_problem .= "User ".$user->id." deleted.";
    unset($user);

}

$delete_problem="";

// Process user search form

$matches="";

if (isset($_POST['search_submit'])){
    $search_name = post_str('search_text');
    $search_name = BoincDb::escape_string(sanitize_tags($search_name));

    if (!empty($search_name)){ 
        $result = mysql_query("SELECT * FROM user WHERE name='$search_name'");

        if (mysql_num_rows($result)==1) {
            $user = mysql_fetch_object($result);
            mysql_free_result($result);
        } else {
            $q = "SELECT * FROM user WHERE name LIKE '%".$search_name."%'";
            $result = mysql_query($q);
            if (mysql_num_rows($result)==1) {
                $user = mysql_fetch_object($result);
                mysql_free_result($result);
            }
            if (mysql_num_rows($result)>1) { 
                while ($row = mysql_fetch_object($result)){ 
                    if (!empty($matches)) {
                        $matches .= ",  ";
                    }
                    $matches .= $row->name;
                }
                mysql_free_result($result);
            }
        }
    }
}


// Look up the user 

$id = get_int("userid", true);
if (!$id) {
    $id = post_int("userid", true);
}
$user = lookup_user_id($id);

// but clear if page was reset (forcing search form) 

if (isset($_POST['reset_page'])){
    unset($user);
}

// Process special user settings

if (isset($_POST['special_user']) && $user && $is_admin){
    $bits="";
    for ($i=0; $i<$Nbf; $i++) {
        $key = "special_user_$i";
        if (array_key_exists($key, $_POST) && $_POST[$key]) {
            $bits .= "1";
        } else {
            $bits .= "0";
        }
    }
    $q = "UPDATE forum_preferences SET special_user=\"$bits\" WHERE userid=$id";
    mysql_query($q);
}


// Process a suspension:

if (isset($_POST['suspend_submit']) && !empty($user) && $is_admin) {
    $dt = post_int('suspend_for',true);

    if ($is_admin || ($is_mod && $dt < 86400)) {
        $reason = $_POST['suspend_reason'];
        if ($dt > 0 && empty($reason)) {
            error_page("You must supply a reason for a suspension.
                <p><a href=manage_user.php?userid=$user->id>Try again</a>"
            );
        } else {
            if (is_numeric($dt)) {
                $t = time()+$dt;
                $q = "UPDATE forum_preferences SET banished_until=$t WHERE userid=$id";
                mysql_query($q);

                // put a timestamp in wiki to trigger re-validation of credentials

                if (function_exists('touch_wiki_user')){
                    touch_wiki_user($user);  
                }

                // Send suspension e-mail to user and administrators

                if ($dt>0) {
                    $subject = PROJECT." posting privileges suspended for ". $user->name;
                    $body = "
Forum posting privileges for the " .PROJECT. " user \"".$user->name."\"
have been suspended for " .time_diff($dt). " by ".$g_logged_in_user->name.". 
The reason given was:
  
   $reason
   
The suspension will end at " .time_str($t)."\n";
                } else {
                    $subject = PROJECT." user ". $user->name. " unsuspended";
                    $body = "
Forum posting privileges for the " .PROJECT. " user \"".$user->name."\"
have been restored by ".$g_logged_in_user->name."\n";
                    if ($reason) {
                        $body.="The reason given was:\n\n   $reason\n";
                    }
                }

                send_email($user, $subject, $body);

                $emails = explode(",", POST_REPORT_EMAILS);
                foreach ($emails as $email) {
                    $admin->email_addr = $email;
                    send_email($admin, $subject, $body);
                }
            }
        }
    }
}


// Process a delete request.  Empty user will trigger search form.
//
if (isset($_POST['delete_user']) && !empty($user)) {
    delete_user($user);
}


// Now update from whatever might have been set above

if (!empty($user)) {
    BoincForumPrefs::lookup($user);
}

// Output:

admin_page_head("User Management: $user->name");

echo "<h2>User Management</h2>\n";

if (!defined("POST_REPORT_EMAILS")) {
    echo "<p><font color='RED'>
   There is no addministrative e-mail address defined for reporting problems
or abuse in the forums.  Please define POST_REPORT_EMAILS in project.inc
        </font></p>\n";
}

echo "<form name='manage_user' action=manage_user.php method='POST'>
    <input type='hidden' name='userid' value='". $user->id."'>
";

start_table();

if (empty($user->id)) {
    if (!empty($search_name)) {
        echo "No match found. ";
        if (!empty($matches)) {
            echo " Partial matches are: <blockquote> $matches </blockquote>\n";
        }
    }
    echo " Enter user name:
        <blockquote>
         <input type='text' name='search_text' >
         <input type='submit' name='search_submit' value='Search'>
        </form>
    ";
    admin_page_tail();
    exit();
}

row1("<b>User: </b> ".$user->name. "<br/>
      Id# ". $user->id 
     . "<div align='right'>
        <input name='reset_page'  type='submit' value='Reset'>
        <input name='manage_user' type='submit' value='Update'><br>
        <input name=\"delete_user\" type=\"submit\" value=\"Delete user\">
        </div>"
);

if ($delete_problem) {
    echo "<font color='RED'>$delete_problem</font><br/>\n";
}

show_user_summary_public($user);
show_profile_link_ops($user);
if ($is_admin) {
    row2("E-mail:", "$user->email_addr");
}
project_user_summary($user);
end_table();
project_user_page_private($user);

echo "</form>\n";


// Special User status:

echo "\n\n<P>
   <table width='100%'><tr>
   <td width='50%' valign='TOP'> \n";

echo "<form name='special_user' action=manage_user.php method=\"POST\">
    <input type='hidden' name='userid' value='".$user->id."'>
";

start_table();
row1("Special User Status: $user->name", $Nbf );

echo "<tr>\n";
for ($i=0; $i<$Nbf; $i++) {
    $bit = substr($user->prefs->special_user, $i, 1);
    echo "<tr><td><input type='checkbox'' name='special_user_".$i."' value='1'";
    if ($bit == 1) {
        echo " checked='checked'";
    }
    echo ">". $special_user_bitfield[$i] ."</td></tr>\n";
}
echo "</tr>";

if ($is_admin) {
    echo "</tr><td colspan=$Nbf align='RIGHT'>
        <input name='special_user' type='SUBMIT' value='Apply'>
        </td></tr>
    ";
}
end_table();
echo "</form>\n";

echo "\n\n</td><td valign='TOP'>\n\n";


// Suspended posting privileges

echo "<form name='banishment' action=manage_user.php method=\"POST\">
    <input type='hidden' name='userid' value='".$user->id."'>
";
start_table();
row1("Suspension: $user->name");

if ($user->prefs->banished_until) {
    $dt = $user->prefs->banished_until - time();
    if ($dt > 0) {
        $x = " Suspended until " . time_str($user->prefs->banished_until)
            ."<br/> (Expires in " . time_diff($dt) .")" ;
    } else {
        $x = " last suspended " . time_str($user->prefs->banished_until);
    }
    row1($x);
} else {
    $dt = 0;
}

echo "<tr><td>
Suspend user for:
 <blockquote>
        <input type='radio' name='suspend_for' value='3600'> 1 hour   <br/>
        <input type='radio' name='suspend_for' value='7200'> 2 hours  <br/>
        <input type='radio' name='suspend_for' value='18000'> 6 hours  <br/>
        <input type='radio' name='suspend_for' value='36000'> 12 hours  <br/>
        <input type='radio' name='suspend_for' value='86400'> 24 hours  <br/>
";
if ($is_admin) {       // in case we are only a moderator
    echo "
        <input type='radio' name='suspend_for' value='172800'> 48 hours  <br/>
        <input type='radio' name='suspend_for' value='",86400*7,"'> 1 week  <br/>
        <input type='radio' name='suspend_for' value='",86400*14,"'> 2 weeks  <br/>
";
}


if ($dt>0) {
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

echo "</td></tr> </table>\n";


if ($q) {
    echo "<P><font color='grey'>Query: $q </font>";
 }

admin_page_tail();

$cvs_version_tracker[]=        //Generated automatically - do not edit
    "\$Id$"; 
?>
