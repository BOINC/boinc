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

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/forum_db.inc");
require_once("../inc/forum.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/countries.inc");
require_once("../inc/credit.inc");
require_once("../inc/team_types.inc");
require_once("../inc/time.inc");
require_once("../inc/stats_sites.inc");

function team_search_form($params) {
    if (!$params) {
        $params = new StdClass;
        $params->keywords = "";
        $params->country = "";
        $params->type = "";
        $params->active = false;
    }
    echo '<form name="form" action="team_search.php">';
    start_table();
    row2('<b>'.tra('Search criteria (use one or more)').'</b>', '');
    row2(
        tra('Key words').'<br><small>'.tra('Find teams with these words in their names or descriptions').'</small>',
        '<input class="form-control" type="text" name="keywords" value="' . htmlspecialchars($params->keywords) . '">');
    row2_init(tra('Country'));
    echo '<select class="form-control" name="country"><option value="" selected>---</option>';
    $country = $params->country;
    if (!$country || $country == 'None') $country = "XXX";
    echo country_select_options($country);
    echo "</select></td></tr>\n";
    row2(tra('Type of team'), team_type_select($params->type, true));
    $checked = $params->active?"checked":"";
    row2(tra('Show only active teams'), "<input type=checkbox name=active $checked>");
    row2("", "<input class=\"btn btn-primary\" type=submit name=submit value=\"".tra('Search')."\">");
    end_table();
    echo '</form>';
}

function foundership_transfer_link($user, $team) {
    $now = time();
    if ($team->ping_user == $user->id) {
        if (transfer_ok($team, $now)) {
            return tra('Requested by you, and founder response deadline has passed.').'
                <br>
                <a href="team_founder_transfer_form.php">'.tra('Complete foundership transfer').'</a>.
            ';
        } else {
            $deadline = date_str(transfer_ok_time($team));
            return '<a href="team_founder_transfer_form.php">'.tra('Requested by you').'</a>; '.tra('founder response deadline is %1', $deadline);
        }
    }
    if (new_transfer_request_ok($team, $now)) {
        if ($team->userid == $user->id) {
            return tra('None');
        } else {
            return '<a href="team_founder_transfer_form.php">'.tra('Initiate request').'</a>';
        }
    }
    return '<a href="team_founder_transfer_form.php">'.tra('Deferred').'</a>';
}

// $team is the team record with a bunch of additional data
// (see team_display.php)
// $user is viewer (not necessarily team founder) or null
//
function display_team_page($team, $user) {
    global $team_name_sites;
    page_head("$team->name");

    echo "<p>";
    start_table();
    row1(tra('Team info'));
    if (strlen($team->description)) {
        row2(tra('Description'), sanitize_html($team->description));
    }
    row2("Created", date_str($team->create_time));
    if (defined("SHOW_NONVALIDATED_TEAMS")) {
        $founder = $team->founder;
        row2("Founder email validated", $founder->email_validated?"Yes":"No (team will not be exported)");
    }
    if (strlen($team->url)) {;
        if (strstr($team->url, "http://")) {
            $x = $team->url;
        } else {
            $x = "http://$team->url";
        }
        row2(tra('Web site'), "<a href=$x>$x</a>");
    }

    if (!NO_STATS) {
        row2(tra('Total credit'), format_credit_large($team->total_credit));
        row2(tra('Recent average credit'), format_credit_large($team->expavg_credit));
        if (function_exists('project_team_credit')) {
            project_team_credit($team);
        }
    }
    show_badges_row(false, $team);
    if (!NO_STATS) {
        $x = "";
        shuffle($team_name_sites);
        foreach ($team_name_sites as $t) {
            $url = $t[0];
            $site_name = $t[1];
            $encoding = $t[2];
            if ($encoding == "hashlc") {
                $key = md5(strtolower($team->name));
            } else if ($encoding == 'hash') {
                $key = md5($team->name);
            } else {
                $key = urlencode($team->name);
            }
            $x .= "<a href=$url".$key.">$site_name</a><br>\n";
        }
        row2(tra('Cross-project stats'), $x);
    }
    row2(tra('Country'), $team->country);
    row2(tra('Type'), team_type_name($team->type));

    if ($team->forum && is_forum_visible_to_user($team->forum, $user)) {
        $f = $team->forum;
        row2('<a href="team_forum.php?teamid='.$team->id.'">'.tra('Message board').'</a>',
            tra('Threads').': '.$f->threads.'<br>'.tra('Posts').': '.$f->posts.'<br>'.tra('Last post').': '.time_diff_str($f->timestamp, time())
        );
    }
    if ($user) {
        if ($user->teamid != $team->id) {
            if ($team->joinable) {
                $tokens = url_tokens($user->authenticator);
                row2("",
                    '<a class="btn btn-success" href="team_join.php?'.$tokens.'&amp;teamid='.$team->id.'">'.tra('Join this team').'</a>
                    <br><p class=\"text-muted\">'.tra('Note: if \'OK to email\' is set in your project preferences, joining a team gives its founder access to your email address.').'</p>'
                );
            } else {
                row2(tra("Not accepting new members"), "");
            }
        }
        if (($user->teamid == $team->id)) {
            if (($user->id == $team->userid)) {
                if ($team->ping_user) {
                    $deadline = date_str(transfer_ok_time($team));
                    row2(tra('Foundership change requested'),
                        '<a href="team_change_founder_form.php?teamid='.$team->id.'">'.tra('Respond by %1', $deadline).'</a>'
                    );
                }
            } else {
                row2(tra('Team foundership change'), foundership_transfer_link($user, $team));
            }
        }
    }
    row1(tra('Members'));
    row2(tra('Founder'),
        $team->founder?user_links($team->founder, BADGE_HEIGHT_MEDIUM):"---"
    );
    if (count($team->admins)) {
        $first = true;
        $x = "";
        foreach ($team->admins as $a) {
            if ($first) {
                $first = false;
            } else {
                $x .= " &middot; ";
            }
            $x .= user_links($a, BADGE_HEIGHT_MEDIUM);
        }
        row2(tra('Admins'), $x);
    }
    $x = "0";
    if (count($team->new_members)) {
        $first = true;
        $x = "";
        foreach ($team->new_members as $id) {
            $u = BoincUser::lookup_id($id);
            if (!$u) continue;
            if ($first) {
                $first = false;
            } else {
                $x .= " &middot; ";
            }
            $x .= user_links($u, BADGE_HEIGHT_MEDIUM);
        }
    }
    row2(tra('New members in last day'), $x);
    row2(tra('Total members'), "$team->nusers (<a href=team_members.php?teamid=$team->id&amp;offset=0&amp;sort_by=expavg_credit>".tra('view')."</a>)");
    if (!NO_STATS) {
        row2(tra('Active members'), "$team->nusers_active (<a href=team_members.php?teamid=$team->id&amp;offset=0&amp;sort_by=expavg_credit>".tra('view')."</a>)");
        row2(tra('Members with credit'), "$team->nusers_worked (<a href=team_members.php?teamid=$team->id&amp;offset=0&amp;sort_by=total_credit>".tra('view')."</a>)");
    }
    end_table();
}

function display_team_members($team, $offset, $sort_by) {
    $n = 20;

    $admins = BoincTeamAdmin::enum("teamid=$team->id");

    // there aren't indices to support sorting by credit.
    // set the following variable to disable sorted output.
    // (though since caching is generally used this shouldn't be needed)
    //
    $nosort = false;

    if ($sort_by == "total_credit") {
        $sort_clause = "total_credit desc";
    } else {
        $sort_clause = "expavg_credit desc";
    }

    start_table();
    $x = array();
    $a = array();
    $x[] = tra('Name');
    $a[] = "";
    if (!NO_STATS) {
        if ($nosort) {
            $x[] = tra('Total credit');
            $x[] = tra('Recent average credit');
        } else {
            if ($sort_by == "total_credit") {
                $x[] = tra('Total credit');
            } else {
                $x[] = "<href=team_members.php?teamid=$team->id&amp;sort_by=total_credit&amp;offset=$offset>".tra('Total credit')."</a>";
            }
            if ($sort_by == "expavg_credit") {
                $x[] = tra('Recent average credit');
            } else {
                $x[] = "<href=team_members.php?teamid=$team->id&amp;sort_by=expavg_credit&amp;offset=$offset>".tra('Recent average credit').'</a>';
            }
        }
        $a[] = ALIGN_RIGHT;
        $a[] = ALIGN_RIGHT;
    }

    $x[] = tra('Country');
    $a[] = "";
    row_heading_array($x, $a);

    $cache_args = "teamid=".$team->id."&mosort=".$nosort."&order=".$sort_clause."&limit=".$offset."_".$n;
    $users = unserialize(get_cached_data(TEAM_PAGE_TTL, $cache_args));
    if (!$users) {
        if ($nosort) {
            $users = BoincUser::enum("teamid=$team->id limit $offset,$n");
        } else {
            $users = BoincUser::enum("teamid=$team->id order by $sort_clause limit $offset,$n");
        }
        set_cached_data(TEAM_PAGE_TTL, serialize($users), $cache_args);
    }

    $j = $offset + 1;
    foreach ($users as $user) {
        $user_total_credit = format_credit_large($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        $x = user_links($user, BADGE_HEIGHT_MEDIUM);
        if ($user->id == $team->userid) {
            $x .= ' ['.tra('Founder').']';
        } else if (is_team_admin_aux($user, $admins)) {
            $x .= ' ['.tra('Admin').']';
        }
        echo "<tr class=row1>
            <td align=left>$j) $x
        </td>";
        if (!NO_STATS) {
            echo "
                <td align=right>$user_total_credit</td>
                <td align=right>$user_expavg_credit</td>
            ";
        }
        echo "
            <td>$user->country</td>
            </tr>
        ";
        $j++;
    }
    echo "</table>";

    if ($offset > 0) {
        $new_offset = $offset - $n;
        echo "<a href=team_members.php?teamid=$team->id&amp;sort_by=$sort_by&amp;offset=$new_offset>".tra('Previous %1', $n)."</a> &middot; ";
    }
    if ($j == $offset + $n + 1) {
        $new_offset = $offset + $n;
        echo "<a href=team_members.php?teamid=$team->id&amp;sort_by=$sort_by&amp;offset=$new_offset>".tra('Next %1', $n)."</a>";
    }
}

// check that the team exists
//
function require_team($team) {
    if (!$team) {
        error_page(tra('No such team.'));
    }
}

function is_team_founder($user, $team) {
    return $user->id == $team->userid;
}

// check that the user is founder of the team
//
function require_founder_login($user, $team) {
    require_team($team);
    if ($user->id != $team->userid) {
        error_page(tra('This operation requires foundership.'));
    }
}

function is_team_admin($user, $team) {
    if (!$user) return false;
    if ($user->id == $team->userid) return true;
    $admin = BoincTeamAdmin::lookup($team->id, $user->id);
    if ($admin) return true;
    return false;
}

// use this when you're displaying a long list of users
// and don't want to do a lookup for each one
//
function is_team_admin_aux($user, $admins) {
    foreach ($admins as $a) {
        if ($a->userid == $user->id) return true;
    }
    return false;
}

function require_admin($user, $team) {
    if (!is_team_admin($user, $team)) {
        error_page(tra('This operation requires team admin privileges'));
    }
}

// return list of ID of user who joined team in last day
//
function new_member_list($teamid) {
    $new_members = array();
    $yesterday = time() - 86400;
    $deltas = BoincTeamDelta::enum("teamid=$teamid and timestamp>$yesterday and joining=1");
    foreach ($deltas as $delta) {
        $u = BoincUser::lookup_id($delta->userid);
        if ($u->teamid == $teamid) {
            $new_members[] = $u->id;  // they might have later quit
        }
    }
    return array_unique($new_members);
}

function admin_list($teamid) {
    $u = array();
    $admins = BoincTeamAdmin::enum("teamid=$teamid");
    foreach ($admins as $admin) {
        $user = BoincUser::lookup_id($admin->userid);
        if (!$user) continue;
        $u[] = $user;
    }
    return $u;
}

function team_table_start($sort_by, $type_url) {
    $x = array();
    $x[] = tra('Rank');
    $x[] = tra('Name');
    $x[] = tra('Members');
    $a = array("", "", ALIGN_RIGHT);
    if (!NO_STATS) {
        if ($sort_by == "total_credit") {
            $x[] = "<a href=top_teams.php?sort_by=expavg_credit".$type_url.">".tra('Recent average credit')."</a>";
            $x[] = tra('Total credit');
        } else {
            $x[] = tra('Recent average credit');
            $x[] = "<a href=top_teams.php?sort_by=total_credit".$type_url.">".tra('Total credit')."</a>";
        }
        $a[] = ALIGN_RIGHT;
        $a[] = ALIGN_RIGHT;
    }
    $x[] = tra('Country');
    $x[] = tra("Type");
    $a[] = "";
    $a[] = "";

    row_heading_array($x, $a);
}

function team_links($team) {
    $b = badges_string(false, $team, BADGE_HEIGHT_MEDIUM);
    return "<a href=team_display.php?teamid=$team->id>$team->name</a> $b";
}

function show_team_row($team, $i) {
    $team_expavg_credit = format_credit_large($team->expavg_credit);
    $team_total_credit = format_credit_large($team->total_credit);
    echo "<tr>
        <td>$i</td>
        <td>".team_links($team)."</td>
        <td align=right>".$team->nusers."</td>
    ";
    if (!NO_STATS) {
        echo "
            <td align=right>$team_expavg_credit</td>
            <td align=right>$team_total_credit</td>
        ";
    }
    echo "
        <td>$team->country</td>
        <td>".team_type_name($team->type)."</td>
        </tr>
    ";
}

function user_join_team($team, $user) {
    user_quit_team($user);
    $res = $user->update("teamid=$team->id");
    if ($res) {
        $now = time();
        BoincTeamDelta::insert("(userid, teamid, timestamp, joining, total_credit) values ($user->id, $team->id, $now, 1, $user->total_credit)");
        return true;
    }
    return false;
}

function user_quit_team($user) {
    if (!$user->teamid) return;
    $user->update("teamid=0");
    $team = BoincTeam::lookup_id($user->teamid);
    if ($team && $team->ping_user==$user->id) {
        $team->update("ping_user=-ping_user");
    }
    BoincTeamAdmin::delete("teamid=$user->teamid and userid=$user->id");
    $now = time();
    BoincTeamDelta::insert("(userid, teamid, timestamp, joining, total_credit) values ($user->id, $user->teamid, $now, 0, $user->total_credit)");
}

function user_erase_team_owner($user) {
    if ($user->teamid) {
        $team = BoincTeam::lookup_id($user->teamid);
        if ($team && $team->userid == $user->id) {
            $team->update("userid=0");
        }
    }
}

function user_erase_team_delta($user) {
    BoincTeamDelta::delete_for_user($user->id);
}

function team_edit_form($team, $label, $url) {
    global $team_types;
    echo "<form method=post action=$url>\n";
    if ($team) {
        echo "<input type=hidden name=teamid value=$team->id>\n";
        if ($team->seti_id) {
            echo "<p class=\"text-danger\">".tra("WARNING: this is a BOINC-wide team. If you make changes here, they will soon be overwritten. Edit the %1 BOINC-wide team %2 instead.", "<a href=https://boinc.berkeley.edu/teams/>", "</a>")
            ."</p>";
        }
    }
    echo '
        <p>
        '.tra('%1 Privacy note %2: if you create a team, your project preferences (resource share, graphics preferences) will be visible to the public.', '<b>', '</b>').'
        <p>
    ';
    start_table();
    row2(tra('Team name, text version').'
        <br><p class=\"text-muted\">'.tra('Don\'t use HTML tags.').'</p>',
        '<input class="form-control" name="name" type="text" size="50" value="'.($team?$team->name:"").'">'
    );
    row2(tra('Team name, HTML version').'
        <br><p class=\"text-muted\">
        '.tra('You may use %1 limited HTML tags %2.', '<a href="html.php" target="_new">', '</a>').'
        '.tra('If you don\'t know HTML, leave this box blank.').'</p>',
        '<input class="form-control" name="name_html" type="text" size="50" value="'.str_replace('"',"'",($team?$team->name_html:"")).'">'
    );
    row2(tra('URL of team web page, if any').':<br><font size=-2>('.tra('without "http://"').')
        '.tra('This URL will be linked to from the team\'s page on this site.'),
        '<input class="form-control" type="text" name="url" size="60" value="'.($team?$team->url:"").'">'
    );
    row2(tra('Description of team').':
        <br><p class=\"text-muted\">
        '.tra('You may use %1 limited HTML tags %2.', '<a href="html.php" target="_new">', '</a>').'
        </p>',
        '<textarea class="form-control" name="description" rows=10>'.($team?$team->description:"").'</textarea>'
    );

    row2(tra('Type of team').':', team_type_select($team?$team->type:null));

    row2_init(tra('Country'));
    echo '<select class="form-control" name="country">';
    echo country_select_options($team?$team->country:null);
    echo "</select></td></tr>\n";

    $x = (!$team || $team->joinable)?"checked":"";
    row2(tra("Accept new members?"), "<input type=checkbox name=joinable $x>");
    // Check if we're using reCaptcha to prevent spam accounts
    //
    if (!$team && recaptcha_public_key()) {
        row2(
            "",
            boinc_recaptcha_get_html(recaptcha_public_key())
        );
    }
    row2("",
        sprintf(
            '<input class="btn" %s type=submit name=new value="%s">',
            button_style(),
            $label
        )
    );
    end_table();
    echo "</form>\n";
}

// decay a team's average credit
//
function team_decay_credit($team) {
    $avg = $team->expavg_credit;
    $avg_time = $team->expavg_time;
    $now = time();
    update_average($now, 0, 0, $avg, $avg_time);
    $team->update("expavg_credit=$avg, expavg_time=$now");

}
// if the team hasn't received new credit for ndays,
// decay its average and return true
//
function team_inactive_ndays($team, $ndays) {
    $diff = time() - $team->expavg_time;
    if ($diff > $ndays*86400) {
        team_decay_credit($team);
        return true;
    }
    return false;
}

function team_count_members($teamid) {
    return BoincUser::count("teamid=$teamid");
}

// These functions determine the rules for foundership transfer, namely:
// - A transfer request is allowed if either:
//   - there is no active request, and it's been at least 60 days
//     since the last request (this protects the founder from
//     being bombarded with frequest requests)
//   - there's an active request older than 90 days
//     (this lets a 2nd requester eventually get a chance)
// - Suppose someone (X) requests foundership at time T.
//   An email is sent to the founder (Y).
//   The request is "active" (ping_user is set to X's ID)
// - If Y declines the change, an email is sent to X,
//   and the request is cleared.
// - If Y accepts the change, an email is sent to X
//   and the request is cleared.
// - After T + 60 days, X can become founder
// - After T + 90 days, new requests are allowed even if there's
//   an active request, i.e. after the 60 days elapse X has another
//   30 days to assume foundership before someone elase can request it
//
function new_transfer_request_ok($team, $now) {
    if ($team->ping_user <= 0) {
        if ($team->ping_time < $now - 60 * 86400) {
            return true;
        }
        return false;
    }
    if ($team->ping_time < $now - 90 * 86400) {
        return true;
    }
    return false;
}

// the time at which we can actually change foundership
// if the founder hasn't responded
//
function transfer_ok_time($team) {
    return $team->ping_time + 60*86400;
}

function transfer_ok($team, $now) {
    if ($now > transfer_ok_time($team)) return true;
    return false;
}

// Make a team; args are untrusted, so cleanse and validate them
//
function make_team(
    $userid, $name, $url, $type, $name_html, $description, $country
) {
    $name = BoincDb::escape_string(sanitize_tags($name));
    if (strlen($name) == 0) return null;
    $name_lc = strtolower($name);
    $url = BoincDb::escape_string(sanitize_tags($url));
    if (strstr($url, "http://")) {
        $url = substr($url, 7);
    }
    $name_html = BoincDb::escape_string($name_html);
    $description = BoincDb::escape_string($description);
    if (!is_valid_country($country)) {
        $country = tra('None');
    }
    $country = BoincDb::escape_string($country);  // for Cote d'Ivoire

    $clause = sprintf(
        "(userid, create_time, name, name_lc, url, type, name_html, description, country, nusers, expavg_time) values(%d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d, unix_timestamp())",
        $userid,
        time(),
        $name,
        $name_lc,
        $url,
        $type,
        $name_html,
        $description,
        $country,
        0
    );
    $id = BoincTeam::insert($clause);
    if ($id) {
        return BoincTeam::lookup_id($id);
    } else {
        return null;
    }
}


?>
