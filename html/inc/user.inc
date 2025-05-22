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
require_once("../inc/ldap.inc");

if (!defined('REMOTE_PROJECTS_TTL')) {
    define('REMOTE_PROJECTS_TTL', 86400);
}

// add an element "projects" to user consisting of array of projects
// they've participated in
//
function get_other_projects($user) {
    $cpid = md5($user->cross_project_id . $user->email_addr);
    $url = "http://boinc.netsoft-online.com/get_user.php?cpid=".$cpid;

    // Check the cache for that URL
    //
    $cacheddata = get_cached_data(REMOTE_PROJECTS_TTL, $url);
    if ($cacheddata) {
        $remote = unserialize($cacheddata);
        if (!$remote) $remote = [];
    } else {
        // Fetch the XML, use curl if fopen() is disallowed
        //
        if (ini_get('allow_url_fopen')) {
            $timeout = 3;
            $old_timeout = ini_set('default_socket_timeout', $timeout);
            $xml_object = null;
            $f = @file_get_contents($url);
            if ($f) {
                $xml_object = @simplexml_load_string($f);
            }
            ini_set('default_socket_timeout', $old_timeout);
            if (!$xml_object) {
                return $user;
            }
        } else {
            $ch = curl_init($url);
            curl_setopt($ch, CURLOPT_HEADER, false);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
            curl_setopt($ch, CURLOPT_MAXREDIRS, 3);
            curl_setopt($ch, CURLOPT_TIMEOUT, 3);
            $rawxml = @curl_exec($ch);
            $xml_object = null;
            if ($rawxml) {
                $xml_object = @simplexml_load_string($rawxml);
            }
            curl_close($ch);
            if (!$xml_object) {
                return $user;
            }
        }

        // auto-cast the project list to an array of stdClass projects
        //
        $remote = @json_decode(json_encode((array)$xml_object))->project;
        if (!$remote) $remote = [];
        if (!is_array($remote)) {
            $remote = [$remote];
        }

        // Cache the results
        set_cached_data(REMOTE_PROJECTS_TTL, serialize($remote), $url);
    }
    $user->projects = $remote;
    return $user;
}

function show_project($project) {
    if ($project->url == "http://www.worldcommunitygrid.org/") {
        $x = $project->name;
    } else {
        $x = sprintf(
            '<a href="%sshow_user.php?userid=%d">%s</a>',
            $project->url,
            $project->id,
            $project->name
        );
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
    return ($a->expavg_credit < $b->expavg_credit) ? 1 : -1;
}

function show_other_projects($user, $personal) {
    if (!isset($user->projects)) return;
    if (count($user->projects) < 2) return;

    usort($user->projects, "cmp");
    if ($personal) {
        $t  = tra("Projects in which you are participating");
    } else {
        $t = tra("Projects in which %1 is participating", $user->name);
    }
    panel(
        $t,
        function() use ($user) {
            show_other_projects_aux($user);
        }
    );
}

function show_other_projects_aux($user) {
    start_table('table-striped');
    row_heading_array(
        array(
            tra("Project")."<br/><small>".tra("Click for user page")."</small>",
            tra("Total credit"),
            tra("Average credit"),
            tra("Since")
        ),
        array("", ALIGN_RIGHT, ALIGN_RIGHT, ALIGN_RIGHT)
    );
    foreach ($user->projects as $project) {
        show_project($project);
    }
    end_table();
}

function total_posts($user) {
    return BoincPost::count("user=$user->id");
}

function show_credit($user) {
    row2(tra("Total credit"), format_credit_large($user->total_credit));
    row2(tra("Recent average credit"), format_credit($user->expavg_credit));
    if (function_exists("project_user_credit")) {
        project_user_credit($user);
    }
}

require_once("../inc/stats_sites.inc");
// show dynamic user info (private)
//
function show_user_stats_private($user) {
    global $cpid_stats_sites;

    if (!NO_STATS) {
        show_credit($user);
    }

    if (!NO_HOSTS) {
        row2(tra("Computers on this account"), "<a href=\"hosts_user.php\">".tra("View")."</a>");
    }
    if (!NO_COMPUTING) {
        row2(tra("Tasks"), "<a href=\"results.php?userid=$user->id\">".tra("View")."</a>");
    }

    if (!NO_STATS) {
        $cpid = md5($user->cross_project_id . $user->email_addr);
        $x = "";
        shuffle($cpid_stats_sites);
        foreach ($cpid_stats_sites as $site) {
            $name = $site[0];
            $y = sprintf($site[1], $cpid);
            $x .= "<a href=\"$y\">$name</a><br/>\n";
        }
        $x .= "<br/><small>".tra("Cross-project ID").": $cpid</small>\n";
        row2(tra("Cross-project statistics"), $x);
        $x = sprintf('<a href="%s">%s</a>', cert_filename(), tra("Account"));
        if ($user->teamid) {
            $x .= ' &middot; <a href="cert_team.php">'.tra("Team").'</a>';
        }
        $x .= ' &middot; <a href="cert_all.php">'.tra("Cross-project").'</a>';
        row2(tra("Certificate"), $x);
    }
}

function notify_description($notify) {
    switch ($notify->type) {
    case NOTIFY_FRIEND_REQ:
        return friend_notify_req_web_line($notify);
    case NOTIFY_FRIEND_ACCEPT:
        return friend_notify_accept_web_line($notify);
    case NOTIFY_PM:
        return pm_web_line($notify);
    case NOTIFY_SUBSCRIBED_THREAD:
        return subscribed_thread_web_line($notify);
    case NOTIFY_SUBSCRIBED_FORUM:
        return subscribed_forum_web_line($notify);
    }
    return null;
}

// a string that can be used to authenticate some operations,
// but can't be used to log in to the account
// (e.g. can't be used to change email addr or passwd)
//
// this is a function of
// - authenticator (never changes)
// - user ID (never changes)
// - password
// - email addr
//
function weak_auth($user) {
    $x = md5($user->authenticator.$user->passwd_hash);
    return "{$user->id}_$x";
}

// originally user URLs were assumed to be http://,
// and this prefix wasn't stored.
// Now the prefix can be http:// or https://.
// This function takes a user URL in any form and converts
// it to a canonical form, with the protocol prefix.
//
function normalize_user_url($url) {
    $url = sanitize_user_url($url);
    if (!$url) return '';
    $x = strtolower($url);
    if (substr($x, 0, 7) == 'http://') {
        return 'http://'.substr($url, 7);
    }
    if (substr($x, 0, 8) == 'https://') {
        return 'https://'.substr($url, 8);
    }
    return 'http://'.$url;
}

// show static user info (private)
//
function show_user_info_private($user) {
    row2(tra("Name"), $user->name);
    if (LDAP_HOST && is_ldap_email($user->email_addr)) {
        row2("LDAP ID", ldap_email_to_uid($user->email_addr));
    } else {
        $email_text = $user->email_addr;
        if (defined("SHOW_NONVALIDATED_EMAIL_ADDR") && !$user->email_validated) {
            $email_text .= " (<a href=validate_email_addr.php>must be validated</a>)";
        }
        row2(tra("Email address"), $email_text);
    }
    if (USER_URL && $user->url) {
        $u = normalize_user_url($user->url);
        row2(
            tra("URL"),
            $u?sprintf('<a href="%s">%s</a>', $u, $u):tra('Invalid URL')
        );
    }
    if (USER_COUNTRY) {
        row2(tra("Country"), $user->country);
    }
    if (POSTAL_CODE) {
        row2(tra("Postal code"), $user->postal_code);
    }
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    $url_tokens = url_tokens($user->authenticator);
    if (LDAP_HOST && is_ldap_email($user->email_addr)) {
        // LDAP accounts can't change email or password
        //
        row2(tra("Change"),
            "<a href=\"edit_user_info_form.php?$url_tokens\">Account info</a>"
        );
    } else {
        $delete_account_str = "";
        $config = get_config();
        if (parse_bool($config, "enable_delete_account")) {
            $delete_account_str = " &middot; <a href=\"delete_account_request.php\">".tra("delete account")."</a>";
        }

        row2(tra("Change"),
            "<a href=\"edit_email_form.php\">".tra("email address")."</a>
            &middot; <a href=\"".secure_url_base()."/edit_passwd_form.php\">".tra("password")."</a>
            &middot; <a href=\"edit_user_info_form.php?$url_tokens\">".tra("other account info")."</a>"
            .$delete_account_str
        );
    }
    if (!UNIQUE_USER_NAME) {
        row2(tra("User ID")."<br/><p class=\"small\">".tra("Used in community functions")."</p>", $user->id);
    }
    if (!NO_COMPUTING) {
        row2(
            tra("Account keys"),
            "<a href=\"weak_auth.php\">".tra("View")."</a>"
        );

        require_once("../inc/account_ownership.inc");
        if (file_exists($account_ownership_private_key_file_path)) {
          // If the server has keys configured show the account ownership form
          row2(
              tra("Account Ownership"),
              "<a href=\"account_ownership.php?$url_tokens\">Generate ownership proof</a>"
          );
        }

    }
}

function show_preference_links() {
    if (!NO_GLOBAL_PREFS) {
        row2(
            tra("When and how BOINC uses your computer"),
            "<a href=\"prefs.php?subset=global\">".tra("Computing preferences")."</a>"
        );
    }
    row2(tra("Message boards and private messages"),
        "<a href=\"edit_forum_preferences_form.php\">".tra("Community preferences")."</a>"
    );
    if (!NO_COMPUTING) {
        row2(tra("Preferences for this project"),
            "<a href=\"prefs.php?subset=project\">".tra("%1 preferences", PROJECT)."</a>"
        );
    }
    if (parse_bool(get_config(), 'keyword_sched')) {
        row2(
            tra('What types of jobs to run'),
            sprintf('<a href=kw_prefs.php>%s</a>',
                tra('Science and location preferences')
            )
        );
    }
}

// return string describing a friend:
// their name, and profile picture if it exists
//
function friend_links($user) {
    $x = sprintf(
        '<a href="%s%s?userid=%d" style="%s">%s</a>',
        url_base(),
        SHOW_USER_PAGE,
        $user->id,
        'vertical-align:top',
        $user->name
    );
    if ($user->has_profile) {
        $profile = BoincProfile::lookup_fields("has_picture", "userid=$user->id");
        if ($profile && $profile->has_picture) {
            $img_url = profile_thumb_url($user->id);
        } else {
            $img_url = url_base()."img/head_20.png";
        }
        $alt = tra("Profile");
        $x .= sprintf(
            '<a href="%sview_profile.php?userid=%d"><img title="%s" src="%s" alt="%s"></a><br>',
            url_base(),
            $user->id,
            tra("View the profile of %1", $user->name),
            $img_url,
            tra("Profile")
        );
    }
    if (function_exists("project_user_links")) {
        $x .= project_user_links($user);
    }
    return $x;
}

// show user name, with links to profile if present.
// if $badge_height is > 0, show badges
// if $name_limit, limit name to N chars
//
function user_links($user, $badge_height=0, $name_limit=0) {
    if (!$user) {
        error_log("user_links(): null arg\n");
        return;
    }
    BoincForumPrefs::lookup($user);
    $x = "";
    if ($user->has_profile) {
        $img_url = url_base()."img/head_20.png";
        $x .= sprintf(
            ' <a href="%s%s?userid=%d"><img title="View the profile of %s" src="%s" alt="Profile"></a>',
            url_base(),
            'view_profile.php',
            $user->id,
            $user->name,
            $img_url
        );
    }
    $name = $user->name;
    if ($name_limit && strlen($name) > $name_limit) {
        $name = substr($name, 0, $name_limit)."...";
    }
    $x .= sprintf(
        '<a href="%s%s?userid=%d">%s</a>',
        url_base(),
        SHOW_USER_PAGE,
        $user->id,
        $name
    );
    if (function_exists("project_user_links")){
        $x .= project_user_links($user);
    }
    if ($badge_height) {
        $x .= badges_string(true, $user, $badge_height);
    }
    return $name_limit?"<nobr>$x</nobr>":$x;
}

// show community links for the logged in user
//
function show_community_private($user) {
    show_badges_row(true, $user);
    if (!DISABLE_PROFILES) {
        if ($user->has_profile) {
            $x = "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a> &middot; <a href=\"delete_profile.php\">".tra("Delete")."</a>";
        } else {
            $x = "<a href=\"create_profile.php\">".tra("Create")."</a>";
        }
        row2(tra("Profile"), $x);
    }
    if (!DISABLE_FORUMS) {
        $tot = total_posts($user);
        if ($tot) {
            if (is_banished($user)) {
                row2(tra("Message boards"), tra('(banished)'));
            } else {
                row2(
                    tra("Message boards"),
                    sprintf(
                        '<a href="%s/forum_user_posts.php?userid=%d">%s</a>',
                        url_base(), $user->id, tra('%1 posts', $tot)
                    )
                );
            }
        }
    }

    row2(tra("Private messages"), pm_notification($user).pm_email_remind($user));

    $notifies = BoincNotify::enum("userid=$user->id");
    if (count($notifies)) {
        $x = "";
        foreach ($notifies as $notify) {
            $y = notify_description($notify);
            if ($y) {
                $x .= "&bull; $y<br>";
            } else {
                $notify->delete();
            }
        }
        $x .= "<a href=\"".notify_rss_url($user)."\"><img vspace=\"4\" border=\"0\" src=\"img/rss_icon.gif\" alt=\"RSS\" /></a>";
        row2(tra("Notifications"), $x);
    }

    if (!DISABLE_TEAMS) {
        if ($user->teamid && ($team = BoincTeam::lookup_id($user->teamid))) {
            $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>
                &middot; <a href=\"team_quit_form.php\">".tra("Quit team")."</a>";
            if (is_team_admin($user, $team)) {
                $x .= " &middot; <a href=\"team_manage.php?teamid=$user->teamid\">".tra("Administer")."</a>";
            }

            // if there's a foundership request, notify the founder
            //
            if ($user->id==$team->userid && $team->ping_user >0) {
                $x .= "<p class=\"text-danger\">".tra("(foundership change request pending)")."</p>";
            }
            row2(tra("Member of team"), $x);
        } else {
            row2(tra("Team"), tra("None")." &middot; <a href=\"team_search.php\">".tra("find a team")."</a>");
        }

        $teams_founded = BoincTeam::enum("userid=$user->id");
        foreach ($teams_founded as $team) {
            if ($team->id != $user->teamid) {
                $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>";
                $x .= " | <a href=\"team_manage.php?teamid=".$team->id."\">".tra("Administer")."</a>";
                if ($team->ping_user > 0) {
                    $x .= "<p class=\"text-danger\">".tra("(foundership change request pending)")."</span>";
                }
                row2(tra("Founder but not member of"), $x);
            }
        }
    }

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    $x = [];
    if ($friends) {
        foreach($friends as $friend) {
            $fuser = BoincUser::lookup_id($friend->user_dest);
            if (!$fuser) continue;
            $x[] = friend_links($fuser);
        }
        row2(tra("Friends"), implode('<br>', $x));
    } else {
        row2(tra("Friends"), '---');
    }

    if (is_admin($user)) {
        row2('Special users', '<a href=user_permissions.php>Manage</a>');
    }
}

// show summary of dynamic and static info (public)
//
function show_user_summary_public($user) {
    global $g_logged_in_user;
    if (!UNIQUE_USER_NAME) {
        row2(tra("User ID"), $user->id);
    }
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    if (USER_COUNTRY) {
        row2(tra("Country"), $user->country);
    }
    if (USER_URL && $user->url) {
        // don't show URL if user has no recent credit (spam suppression)
        //
        if (!NO_COMPUTING || $user->expavg_credit > 1) {
            $u = normalize_user_url($user->url);
            row2(tra("URL"), sprintf('<a href="%s">%s</a>', $u, $u));
        }
    }
    if (!NO_COMPUTING) {
        show_credit($user);

        if ($user->show_hosts) {
            row2(tra("Computers"), "<a href=\"".url_base()."hosts_user.php?userid=$user->id\">".tra("View")."</a>");
        } else {
            row2(tra("Computers"), tra("hidden"));
        }
    }
    if (function_exists("project_user_summary_public")) {
        project_user_summary_public($user);
    }
}

// return an object with data to show the user's community links
//
function get_community_links_object($user){
    $cache_object = new StdClass;
    $cache_object->post_count = total_posts($user);
    $cache_object->user = $user;
    $cache_object->team = BoincTeam::lookup_id($user->teamid);
    $cache_object->friends = array();

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    foreach($friends as $friend) {
        $fuser = BoincUser::lookup_id($friend->user_dest);
        if (!$fuser) continue;
        $cache_object->friends[] = $fuser;
    }
    return $cache_object;
}

// show community links of another user (described by $clo)
//
function community_links($clo, $logged_in_user){
    $user = $clo->user;
    if (!$user) {
        error_log("community_links(): null user\n");
        return;
    }
    $team = $clo->team;
    $friends = $clo->friends;
    $tot = $clo->post_count;

    if (!DISABLE_TEAMS) {
        if ($user->teamid && $team) {
            row2(tra("Team"), "<a href=\"".url_base()."team_display.php?teamid=$team->id\">$team->name</a>");
        } else {
            row2(tra("Team"), '&mdash;');
        }
    }
    if (!DISABLE_FORUMS) {
        if ($tot) {
            $link = sprintf(
                '<a href="%s/forum_user_posts.php?userid=%d">%s</a>',
                url_base(), $user->id, tra("%1 posts", $tot)
            );
            if (is_banished($user)) {
                if (is_moderator($logged_in_user)) {
                    row2(tra("Message boards"), $link.' (banished)');
                } else {
                    row2(tra("Message boards"), tra('(banished)'));
                }
            } else {
                row2(tra("Message boards"), $link);
            }
        }
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
        $x = [];
        foreach($friends as $friend) {
            $x[] = friend_links($friend);
        }
        row2(tra('Friends'), implode('<br>', $x));
    }
}

function show_profile_link($user) {
    if ($user->has_profile) {
        row2(tra("Profile"), "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a>");
    }
}

function show_account_private($user) {
    grid(
        false,
        function() use ($user) {
            panel(
                tra("Account information"),
                function() use ($user) {
                    start_table();
                    show_user_info_private($user);
                    end_table();
                }
            );
            if (!NO_COMPUTING || !NO_STATS || !NO_HOSTS) {
                panel(
                    tra("Computing"),
                    function() use($user) {
                        start_table();
                        show_user_stats_private($user);
                        end_table();
                    }
                );
            }
            if (function_exists('show_user_donations_private')) {
                show_user_donations_private($user);
            }
            if (!NO_COMPUTING) {
                show_other_projects($user, true);
            }
            if (function_exists("project_user_page_private")) {
                project_user_page_private($user);
            }
        },
        function() use ($user) {
            panel(
                tra("Community"),
                function() use ($user) {
                    start_table();
                    show_community_private($user);
                    end_table();
                }
            );
            panel(
                tra("Preferences"),
                function() use($user) {
                    start_table();
                    show_preference_links();
                    end_table();
                }
            );
        }
    );
}



?>
