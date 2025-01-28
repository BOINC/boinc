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

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

// Delete a user if they have no credit, results, or posts
//
function possibly_delete_user($user){
    if ($user->total_credit > 0.0){
        admin_error_page("Cannot delete user: User has credit.");
    }

    // Don't delete user if they have any outstanding Results
    //
    if (BoincResult::count("userid=$user->id")) {
        admin_error_page("Cannot delete user: User has count results in the database.");
    }

    // Don't delete user if they have posted to the forums
    //
    if (BoincPost::count("user=$user->id")) {
        admin_error_page("Cannot delete user: User has forum posts.");
    }

    if ($user->teamid){
        user_quit_team($user);
    }
    delete_user($user);
}

// Process special user settings
//
function handle_special_user($user) {
    global $special_user_bitfield;
    $Nbf = sizeof($special_user_bitfield);
    $bits="";
    for ($i=0; $i<$Nbf; $i++) {
        $key = "special_user_$i";
        if (array_key_exists($key, $_POST) && $_POST[$key]) {
            $bits .= "1";
        } else {
            $bits .= "0";
        }
    }
    $q = "UPDATE forum_preferences SET special_user=\"$bits\" WHERE userid=$user->id";
    _mysql_query($q);
}


// Process a suspension:
//
function handle_suspend($user) {
    global $g_logged_in_user;
    $dt = post_int('suspend_for', true);

    $reason = $_POST['suspend_reason'];
    if ($dt > 0 && empty($reason)) {
        admin_error_page("You must supply a reason for a suspension.
            <p><a href=manage_user.php?userid=$user->id>Try again</a>"
        );
    } else {
        if (is_numeric($dt)) {
            $t = $dt>0 ? time()+$dt : 0;
            $q = "UPDATE forum_preferences SET banished_until=$t WHERE userid=$user->id";
            _mysql_query($q);

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

function show_manage_user_form($user) {
    global $special_user_bitfield;
    $Nbf = sizeof($special_user_bitfield);

    admin_page_head("Management $user->name");

    if (!defined("POST_REPORT_EMAILS")) {
        echo "<p><font color='RED'>
       There is no administrative email address defined for reporting problems
    or abuse in the forums.  Please define POST_REPORT_EMAILS in project.inc
            </font></p>\n";
    }

    echo "<form name='manage_user' action=manage_user.php method='POST'>
        <input type='hidden' name='userid' value='". $user->id."'>
    ";

    start_table();


    row1("<b>User: </b> $user->name <div align='right'>
            <input class=\"btn btn-danger\" name=\"delete_user\" type=\"submit\" value=\"Delete user\">
            </div>"
    );

    show_user_summary_public($user);
    show_profile_link_ops($user);
    row2("Email:", "$user->email_addr");
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
    row1("Special User Status");

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

    echo "</tr><td colspan=$Nbf align='RIGHT'>
        <input name='special_user' type='SUBMIT' value='Update'>
        </td></tr>
    ";
    end_table();
    echo "</form>\n";

    echo "\n\n</td><td valign='TOP'>\n\n";


    // Suspended posting privileges

    echo "<form name='banishment' action=manage_user.php method=\"POST\">
        <input type='hidden' name='userid' value='".$user->id."'>
    ";
    start_table();
    row1("Suspension");

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
    echo "
        <input type='radio' name='suspend_for' value='172800'> 48 hours  <br/>
        <input type='radio' name='suspend_for' value='",86400*7,"'> 1 week  <br/>
        <input type='radio' name='suspend_for' value='",86400*14,"'> 2 weeks  <br/>
    ";

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


    echo "<p align='RIGHT'><input name='suspend_submit' type='SUBMIT' value='Update'></P>\n";
    echo " </td></tr>\n";

    end_table();
    echo "</form>\n";

    echo "</td></tr> </table>\n";

    admin_page_tail();
}

get_logged_in_user();
db_init();

$q = null;

$id = get_int("userid", true);
if (!$id) {
    $id = post_int("userid", true);
}
if (!$id) admin_error_page("No ID given");
$user = BoincUser::lookup_id($id);
if (!$user) admin_error_page("No such user: $id");

BoincForumPrefs::lookup($user);

if (isset($_POST['delete_user'])) {
    possibly_delete_user($user);
    admin_page_head("User deleted");
    echo "
        User $user->name ($user->id) deleted.
        <p>
        <a href=
    ";
    admin_page_tail();
    exit;
}

if (isset($_POST['special_user'])) {
    handle_special_user($user);
    Header("Location: manage_user.php?userid=$user->id");
}
if (isset($_POST['suspend_submit'])) {
    handle_suspend($user);
    Header("Location: manage_user.php?userid=$user->id");
}

show_manage_user_form($user);

?>
