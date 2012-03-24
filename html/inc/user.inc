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

require_once("../inc/credit.inc");
require_once("../inc/email.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/friend.inc");
require_once("../inc/forum_db.inc");
require_once("../inc/notify.inc");

function parse_project($f) {
    $p->total_credit = 0.0;
    $p->expavg_credit = 0.0;
    while (!feof($f)) {
        $buf = fgets($f);
        if (strstr($buf, "</project>")) break;
        if ($x = parse_element($buf, "<name>")) {
            $p->name = $x;
        }
        if ($x = parse_element($buf, "<url>")) {
            $p->url = $x;
        }
        if ($x = parse_element($buf, "<total_credit>")) {
            $p->total_credit = $x;
        }
        if ($x = parse_element($buf, "<expavg_credit>")) {
            $p->expavg_credit = $x;
        }
        if ($x = parse_element($buf, "<id>")) {
            $p->id = $x;
        }
        if ($x = parse_element($buf, "<country>")) {
            $p->country = $x;
        }
        if ($x = parse_element($buf, "<team_id>")) {
            $p->team_id = $x;
        }
        if ($x = parse_element($buf, "<team_name>")) {
            $p->team_name = $x;
        }
        if ($x = parse_element($buf, "<create_time>")) {
            $p->create_time = $x;
        }
    }
    return $p;
}

function parse_user($f, $user) {
    $user->projects = array();
    while (!feof($f)) {
        $buf = fgets($f);
        if (strstr($buf, "</user>")) break;
        if (strstr($buf, "<project>")) {
            $user->projects[] = parse_project($f);
        }
    }
    return $user;
}

function get_other_projects($user) {
    $cpid = md5($user->cross_project_id . $user->email_addr);
    
    $timeout = 3;
    $old_timeout = ini_set('default_socket_timeout', $timeout);
    $url = "http://boinc.netsoft-online.com/get_user.php?cpid=$cpid";
    $f = @fopen($url, "r");
    ini_set('default_socket_timeout', $old_timeout);
    if (!$f) {
        return $user;
    }
    $u = parse_user($f, $user);
    fclose($f);
    return $u;
}

function show_project($project) {
    if ($project->url == "http://www.worldcommunitygrid.org/") {
        $x = $project->name;
    } else {
        $x = "<a href=\"$project->url"."show_user.php?userid=$project->id\">$project->name</a>";
    }
    echo "<tr>
        <td>$x</td>
        <td align=\"right\">".number_format($project->total_credit, 0)."</td>
        <td align=\"right\">".number_format($project->expavg_credit, 0)."</td>
        <td align=\"right\">".date_str($project->create_time)."</td>
        </tr>
    ";
}

function cmp($a, $b) {
    if ($a->expavg_credit == $b->expavg_credit) return 0;
    return ($a->expavg_credit < $b->expavg_credit)? 1 : -1;
}

function show_other_projects($user, $personal) {
    if (count($user->projects) > 1) {
        usort($user->projects, "cmp");
        if ($personal) {
            echo "<h2>".tra("Projects in which you are participating")."</h2>";
        } else {
            echo "<h2>".tra("Projects in which %1 is participating", $user->name)."</h2>";
        }
        start_table();
        row_heading_array(array(
            tra("Project")."<br/><span class=\"note\">".tra("Click for user page")."</span>", tra("Total credit"), tra("Average credit"), tra("Since")
        ));
        foreach($user->projects as $project) {
            show_project($project);
        }
        end_table();
    }
}

function total_posts($user) {
    return BoincPost::count("user=$user->id");
}

function show_credit($user) {
    row2(tra("Total credit"), format_credit_large($user->total_credit));
    row2(tra("Recent average credit"), format_credit($user->expavg_credit));
    project_user_credit($user);
}

require_once("../inc/stats_sites.inc");
// show dynamic user info (private)
//
function show_user_stats_private($user) {
    global $cpid_stats_sites;
    row1(tra("Computing and credit"));
    show_credit($user);
    $config = get_config();
    if (parse_bool($config, "show_results")) {
        row2(tra("Pending credit"), "<a href=\"pending.php\">".tra("View")."</a>");
    }
    row2(tra("Computers on this account"), "<a href=\"hosts_user.php\">".tra("View")."</a>");
    row2(tra("Tasks"), "<a href=\"results.php?userid=$user->id\">".tra("View")."</a>");
    $cpid = md5($user->cross_project_id . $user->email_addr);
    $x = "";
    shuffle($cpid_stats_sites);
    foreach ($cpid_stats_sites as $site) {
        $name = $site[0];
        $y = sprintf($site[1], $cpid);
        $x .= "<a href=\"$y\">$name</a><br/>\n";
    }
    $x .= "<br/><span class=\"smalltext\">".tra("Cross-project ID").": $cpid</span>\n";
    row2(tra("Cross-project statistics"), $x);
    $x = '<a href="cert1.php">'.tra("Account").'</a>';
    if ($user->teamid) {
        $x .= ' | <a href="cert_team.php">'.tra("Team").'</a>';
    }
    $x .= ' | <a href="cert_all.php">'.tra("Cross-project").'</a>';
    row2(tra("Certificate"), $x);
    row2(tra("Stats on your cell phone"), URL_BASE."userw.php?id=$user->id");
}

function notify_description($notify) {
    switch ($notify->type) {
    case NOTIFY_FRIEND_REQ:
        return friend_notify_req_web_line($notify);
    case NOTIFY_FRIEND_ACCEPT:
        return friend_notify_accept_web_line($notify);
    case NOTIFY_PM:
        return pm_web_line($notify);
    case NOTIFY_SUBSCRIBED_POST:
        return subscribed_post_web_line($notify);
    }
    return tra("Unknown notification type: %1", $notify->type);
}

function weak_auth($user) {
    $x = md5($user->authenticator.$user->passwd_hash);
    return "{$user->id}_$x";
}

// show static user info (private)
//
function show_user_info_private($user) {
    $email_text = $user->email_addr;
    if (defined("SHOW_NONVALIDATED_EMAIL_ADDR") && !$user->email_validated) {
        $email_text .= " (<a href=validate_email_addr.php>must be validated</a>)";
    }
    row1(tra("Account information"));
    row2(tra("Name"), $user->name);
    row2(tra("Email address"), $email_text);
    if (strlen($user->url)) {
        $x = "<a href=http://$user->url>http://$user->url</a>";
        row2(tra("URL"), $x);
    }
    row2(tra("Country"), $user->country);
    row2(tra("Postal code"), $user->postal_code);
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    $url_tokens = url_tokens($user->authenticator);
    row2(tra("Change"), "<a href=\"edit_email_form.php\">".tra("email address")."</a>
        | <a href=\"".SECURE_URL_BASE."/edit_passwd_form.php\">".tra("password")."</a>
        | <a href=\"edit_user_info_form.php?$url_tokens\">".tra("other account info")."</a>"
    );
    row2(tra("User ID")."<br/><span class=\"note\">".tra("Used in community functions")."</span>", $user->id);
    if (!no_computing()) {
        row2(tra("Weak account key")."<br/><span class=\"note\">".tra("Provides %1limited access%2 to your account", "<a href=\"weak_auth.php\">", "</a> ")."</span>", weak_auth($user));
    }

    row1("<a name=\"prefs\"></a>".tra("Preferences"));
    if (!no_computing()) {
        row2(
            tra("When and how BOINC uses your computer"),
            "<a href=\"prefs.php?subset=global\">".tra("Computing preferences")."</a>"
        );
    }
    row2(tra("Message boards and private messages"),
        "<a href=\"edit_forum_preferences_form.php\">".tra("Community preferences")."</a>"
    );
    if (!no_computing()) {
        row2(tra("Preferences for this project"),
            "<a href=\"prefs.php?subset=project\">".tra("%1 preferences", PROJECT)."</a>"
        );
    }
}

function show_community_private($user) {
    row1(tra("Community"));

    if ($user->has_profile) {
        $x = "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a> | <a href=\"delete_profile.php\">".tra("Delete")."</a>";
    } else {
        $x = "<a href=\"create_profile.php\">".tra("Create")."</a>";
    }
    row2(tra("Profile"), $x);
    $tot = total_posts($user);
    if ($tot) {
        row2(tra("Message boards"), "<a href=\"".URL_BASE."forum_user_posts.php?userid=$user->id\">".tra("%1 posts", $tot)."</a>");
    }
    
    row2(tra("Private messages"), pm_notification($user).pm_email_remind($user));

    $notifies = BoincNotify::enum("userid=$user->id");
    if (count($notifies)) {
        $x = "";
        foreach ($notifies as $notify) {
            $x .= "&bull; ".notify_description($notify)."<br>";
        }
        $x .= "<a href=\"".notify_rss_url($user)."\"><img vspace=\"4\" border=\"0\" src=\"img/rss_icon.gif\" alt=\"RSS\" /></a>";
        row2(tra("Notifications"), $x);
    }

    if (!defined('NO_TEAMS')) {
        if ($user->teamid) {
            $team = lookup_team($user->teamid);
            $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>
                | <a href=\"team_quit_form.php\">".tra("Quit team")."</a>";
            if (is_team_admin($user, $team)) {
                $x .= " | <a href=\"team_manage.php?teamid=$user->teamid\">".tra("Administer")."</a>";
            }

            // if there's a foundership request, notify the founder
            //
            if ($user->id==$team->userid && $team->ping_user >0) {
                $x .= "<span style=\"color:red;\">".tra("(foundership change request pending)")."</span>";
            }
            row2(tra("Member of team"), $x);
        } else {
            row2(tra("Team"), tra("None")." | <a href=\"team_search.php\">".tra("find a team")."</a>");
        }
        
        $teams_founded = BoincTeam::enum("userid=$user->id");
        foreach ($teams_founded as $team) {
            if ($team->id != $user->teamid) {
                $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>";
                $x .= "| <a href=\"team_manage.php?teamid=".$team->id."\">".tra("Administer")."</a>";
                if ($team->ping_user > 0) {
                    $x .= "<span style=\"color:red;\">".tra("(foundership change request pending)")."</span>";
                }
                row2(tra("Founder but not member of"), $x);
            }
        }
    }

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    $x = "<a href=\"user_search.php\">".tra("Find friends")."</a><br/>\n";
    $n = count($friends);
    if ($n) {
        foreach($friends as $friend) {
            $fuser = BoincUser::lookup_id($friend->user_dest);
            $x .= friend_links($fuser);
        }
        row2(tra("Friends")." ($n)", $x);
    } else {
        row2(tra("Friends"), $x);
    }
}

// show summary of dynamic and static info (public)
//
function show_user_summary_public($user) {
    global $g_logged_in_user;
    row2(tra("User ID"), $user->id);
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    row2(tra("Country"), $user->country);
    // don't show URL if user has no recent credit (spam suppression)
    //
    if (strlen($user->url) && $user->expavg_credit > 1) {
        row2(tra("URL"), "<a href=\"http://$user->url\">http://$user->url</a>");
    }
    if (!no_computing()) {
        show_credit($user);

        if ($user->show_hosts) {
            row2(tra("Computers"), "<a href=\"".URL_BASE."hosts_user.php?userid=$user->id\">".tra("View")."</a>");
        } else {
            row2(tra("Computers"), tra("hidden"));
        }
    }
    if ($user->donated == 1) { 
        if (file_exists("../project/donations.inc")) {
            require_once("../project/donations.inc");
            $x .= DONATION_LINK;
            row2(tra("Donor"), $x);
        }
    }
}

// Returns a cacheable community links data object
// @param user The user to produce a community links object for

function get_community_links_object($user){
    $cache_object->post_count = total_posts($user);
    $cache_object->user = $user;
    $cache_object->team = lookup_team($user->teamid);

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    $n = count($friends);
    if ($n) {
        $x = null;
        foreach($friends as $friend) {
            $fuser = BoincUser::lookup_id($friend->user_dest);
            $cache_object->friends[] = $fuser;
        }
    }
    return $cache_object;
}

function community_links($clo, $logged_in_user){
    $user = $clo->user;
    $team = $clo->team;
    $friends = $clo->friends;
    $tot = $clo->post_count;
    
    if (!defined('NO_TEAMS')) {
        if ($user->teamid && $team) {
            row2(tra("Team"), "<a href=\"".URL_BASE."team_display.php?teamid=$team->id\">$team->name</a>");
        } else {
            row2(tra("Team"), tra("None"));
        }
    }
    if ($tot) {
        row2(tra("Message boards"), "<a href=\"".URL_BASE."forum_user_posts.php?userid=$user->id\">".tra("%1 posts", $tot)."</a>");
    }
    if ($logged_in_user && $logged_in_user->id != $user->id) {
        row2(tra("Contact"), "<a href=\"pm.php?action=new&userid=".$user->id."\">".tra("Send private message")."</a>");
        $friend = BoincFriend::lookup($logged_in_user->id, $user->id);
        if ($friend && $friend->reciprocated) {
            row2(tra("This person is a friend"), 
                "<a href=\"friend.php?action=cancel_confirm&userid=$user->id\">".tra("Cancel friendship")."</a>"
            );
        } else if ($friend) {
            row2(tra("Friends"),  "<a href=\"friend.php?action=add&userid=$user->id\">".tra("Request pending")."</a>");
        } else {
            row2(tra("Friends"),  "<a href=\"friend.php?action=add&userid=$user->id\">".tra("Add as friend")."</a>");
        }
    }
    
    if ($friends) {
        $x = null;
        foreach($friends as $friend) {
            $x .= friend_links($friend);
        }
        row2(tra("Friends")." (".sizeof($friends).")", $x);
    }
}

function show_profile_link($user) {
    if ($user->has_profile) {
        row2(tra("Profile"), "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a>");
    }
}

function show_account_private($user) {
    start_table_noborder();
    echo "<tr><td valign=top>";
    start_table();
    show_user_info_private($user);
    if (!no_computing()) {
        show_user_stats_private($user);
    }

    if (file_exists("../project/donations.inc")) {
        require_once("../project/donations.inc");
        if (function_exists('show_user_donations_private')) {
            show_user_donations_private($user);
        }
    }
    end_table();
    show_other_projects($user, true);
    project_user_page_private($user);
    //echo "<a href=delete_account.php>Delete account</a>\n";
    echo "</td><td valign=top>";
    start_table();
    show_community_private($user);
    end_table();

    echo "</td></tr></table>";
}

function is_banned_email_addr($email_addr) {
    global $banned_email_domains;
    if (isset($banned_email_domains)) {
        foreach($banned_email_domains as $d) {
            $x = strstr($email_addr, $d);
            if ($x == $d) return true;
        }
    }
    return false;
}

function is_valid_user_name($name, &$reason) {
    if (trim($name) !== name) {
        $reason = tra("user name cannot have leading or trailing white space");
        return false;
    }
    if (strlen($name) == 0) {
        $reason = tra("user name must be nonempty");
        return false;
    }
    if (sanitize_tags($name) !== name)) {
        $reason = tra("user name may not contain HTML tags");
        return false;
    }
    return true;
}

// the following DB-escapes its args
//
function make_user(
    $email_addr, $name, $passwd_hash,
    $country=null, $postal_code=null, $project_prefs=null, $teamid=0
) {
    if (!is_valid_email_addr($email_addr)) return null;
    if (is_banned_email_addr($email_addr)) return null;

    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    if (!is_valid_country($country)) return null;

    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $passwd_hash = BoincDb::escape_string($passwd_hash);

    $country = BoincDb::escape_string($country);
    $postal_code = sanitize_tags(BoincDb::escape_string($postal_code));

    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, 1, 1, '$cross_project_id', '$passwd_hash')");

    if ($uid) {
        return BoincUser::lookup_id($uid);
    } else {
        return null;
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
