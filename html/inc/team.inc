<?php

require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/countries.inc");
require_once("../inc/credit.inc");
require_once("../inc/team_types.inc");

function display_team_page($team, $offset, $sort_by) {
    $n = 20;

    // there aren't indices to support sorting by credit.
    // set the following variable to disable sorted output.
    // (though since caching is generally used this shouldn't needed)
    //
    $nosort = false;

    if ($sort_by == "total_credit") {
        $sort_clause = "total_credit desc";
    } else {
        $sort_clause = "expavg_credit desc";
    }
    
    page_head("$team->name");

    echo sanitize_html($team->name_html);
    start_table();
    row1("Team info");
    if (strlen($team->description)) {
        row2("Description", sanitize_html($team->description));
    }
    if (strlen($team->url)) {;
        if (strstr($team->url, "http://")) {
            $x = $team->url;
        } else {
            $x = "http://$team->url";
        }
        row2("Web site", "<a href=$x>$x</a>");
    }
    row2("Members (with credit)", $team->nusers);
    
    $cache_args = "teamid=".$team->id."&founderid=".$team->userid;
    $cached_data = get_cached_data(TEAM_PAGE_TTL, $cache_args);
    if ($cached_data){
        $founder = unserialize($cached_data);
    } else {
        $founder = lookup_user_id($team->userid);
        set_cache_data(serialize($founder),$cache_args);
    }
    
    $user = get_logged_in_user(false);

    row2("Total credit", format_credit($team->total_credit));
    row2("Recent average credit", format_credit($team->expavg_credit));
    row2("Founder", user_links($founder));
    row2("Country", $team->country);
    row2("Type", team_type_name($team->type));
    if ($user->teamid != $team->id) {
        row2("Join this team", "<a href='team_join_form.php?id=$team->id'>Join</a>");
    }
    if (($user->teamid == $team->id) && ($user->id != $founder->id)) {
        row2("Request team founder transfer", "<a href=\"team_founder_transfer_form.php\">Initiate transfer</a>");
    }
    echo "</table>";
    echo "<p>";
    start_table();
    row1("Active members (with credit)", 4);
    echo "<tr>
        <th>Name</th>
    ";
    if ($nosort) {
        echo "
            <th>Total credit</th>
            <th>Recent average credit</th>
        ";
    } else {
        if ($sort_by == "total_credit") {
            echo "<th>Total credit</th>";
        } else {
            echo "<th><a href=team_display.php?teamid=$team->id&sort_by=total_credit&offset=$offset>Total credit</a></th>";
        }
        if ($sort_by == "expavg_credit") {
            echo "<th>Recent average credit</th>";
        } else {
            echo "<th><a href=team_display.php?teamid=$team->id&sort_by=expavg_credit&offset=$offset>Recent average credit</a></th>";
        }
    }

    echo "
        <th>Country</th>
        </tr>
    ";

    if ($nosort) {
        $result = mysql_query("select * from user where teamid=$team->id and total_credit>0 limit $offset,$n");
    } else {
        $result = mysql_query("select * from user where teamid=$team->id and total_credit>0 order by $sort_clause limit $offset,$n");
    }

    $j = $offset + 1;
    while ($user = mysql_fetch_object($result)) {
        $user_total_credit = format_credit($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        $x = user_links($user);
        echo "<tr class=row1>
            <td align=left>$j) $x
            <td align=center>$user_total_credit</td>
            <td align=center>$user_expavg_credit</td>
            <td align=center>$user->country</td>
            </tr>
        ";
        $j++;
    }
    mysql_free_result($result);
    echo "</table>";

    if ($offset > 0) {
        $new_offset = $offset - $n;
        echo "<a href=team_display.php?teamid=$team->id&sort_by=$sort_by&offset=$new_offset>Last $n</a> | ";
    }
    if ($j == $offset + $n + 1) {
        $new_offset = $offset + $n;
        echo "<a href=team_display.php?teamid=$team->id&sort_by=$sort_by&offset=$new_offset>Next $n</a>";
    }
}

// requires that the team exist
function require_team($team) {
    if (!$team) {
        error_page("No such team.");
    }
}

// requires that the user is logged in as the founder of
// the team trying to be edited
//
function require_founder_login($user, $team) {
    require_team($team);
    if ($user->id != $team->userid) {
        error_page("Only a team's founder may edit a team.");
    }
}

function team_table_start($sort_by,$type_url) {
    echo "<tr>
        <th>".tr(USER_TABLE_RANK)."</th>
        <th>".tr(USER_TABLE_NAME)."</th>
        <th>".tr(TEAM_TABLE_MEMBERS)."</th>
    ";
    if ($sort_by == "total_credit") {
        echo "
            <th><a href=top_teams.php?sort_by=expavg_credit".$type_url.">".tr(EXPAVG_CREDIT)."</a></th>
            <th>".tr(TOTAL_CREDIT)."</th>
        ";
    } else {
        echo "
            <th>".tr(EXPAVG_CREDIT)."</th>
            <th><a href=top_teams.php?sort_by=total_credit".$type_url.">".tr(TOTAL_CREDIT)."</a></th>
        ";
    }
    echo "
        <th>".tr(USER_TABLE_COUNTRY)."</th>
        </tr>
    ";
}

function show_team_row($team, $i) {
    $team_expavg_credit = format_credit($team->expavg_credit);
    $team_total_credit = format_credit($team->total_credit);
    echo"<tr class=row1>
        <td>$i</td>
        <td><a href=team_display.php?teamid=$team->id>$team->name</a></td>
        <td>".$team->nusers."</td>
        <td>$team_expavg_credit</td>
        <td>$team_total_credit</td>
        <td>$team->country</td>
        </tr>\n";
}

function user_join_team($team, $user) {
    $old_teamid = $user->teamid;
    $res = mysql_query("update user set teamid=$team->id where id=$user->id");
    if ($res) return true;
    return false;
}

function user_quit_team($user) {
    $team = lookup_team($user->teamid);
    if (!$team) return;
    mysql_query("update user set teamid=0 where id=$user->id");
    mysql_query("UPDATE team SET ping_user=0 WHERE id=".$user->teamid." AND ping_user=".$user->id);
}

function team_edit_form($team, $label, $url) {
    global $team_types;
    echo "<form method=post action=$url>\n";
    if ($team) {
        echo "<input type=hidden name=teamid value=$team->id>\n";
    }
    echo "
        <p>
        <b>Privacy note</b>: if you create a team, your project preferences
        (resource share, graphics preferences)
        will be visible to the public.
        <p>
    ";
    start_table();
    row2( "Team name, text version
        <br><font size=-2>
        Don't use any HTML tags.
        This name will be used in the searchable team list.",
        "<input name=name type=text size=50 value='$team->name'>"
    );
    row2("Team name, HTML version
        <br><font size=-2>
        You may include HTML formatting, link, and image tags.
        If you don't know HTML, just leave this box blank.",
        "<input name=name_html type=text size=50 value=\"".str_replace('"',"'",$team->name_html)."\">"
    );
    row2("URL of team web page, if any:<br><font size=-2>(without \"http://\")
        This URL will be linked to from the team's page on this site.",
        "<input name=url size=60 value='$team->url'>"
    );
    row2("Description of team:",
        "<textarea name=description cols=60 rows=10>$team->description</textarea>"
    );

    row2("Type of team:", team_type_select($team));

    row2_init("Country",
        "<select name=country>"
    );
    print_country_select($team->country);
    
    echo "</select></b></td></tr>\n";
    row2("",
        "<input type=submit name=new value='$label'>"
    );
    end_table();
    echo "</form>\n";
}

// decay a team's average credit
//
function team_decay_credit($team) {
    $avg = $team->expavg_credit;
    $avg_time = $team->expavg_time;
    $now = time(0);
    update_average($now, 0, 0, $avg, $avg_time);
    mysql_query("update team set expavg_credit=$avg, expavg_time=$now where id=$team->id");

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

function team_count_nusers($teamid) {
    $res = mysql_query("select count(*) as total from user where teamid=$teamid and total_credit>0");
    $cnt = mysql_fetch_object($res);
    if ($cnt) {
        return $cnt->total;
    } else {
        return null;
    }
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
    if ($team->ping_user == 0) {
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
    $name = boinc_htmlentities(process_user_text(strip_tags($name)));
    if (strlen($name) == 0) return null;
    $name_lc = strtolower($name);
    $url = process_user_text(strip_tags($url));
    if (strstr($url, "http://")) {
        $url = substr($url, 7);
    }
    $name_html = process_user_text($name_html);
    $description = process_user_text($description);
    $country = process_user_text($country);
    if (!is_valid_country($country)) {
            $country = 'None';
    }
    $country = boinc_real_escape_string($country);  // for Cote d'Ivoire

    $query = sprintf(
        "insert into team (userid, create_time, name, name_lc, url, type, name_html, description, country, nusers, expavg_time) values(%d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d, unix_timestamp())",
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
    $result = mysql_query($query);
    if ($result) {
        $teamid = mysql_insert_id();
        return lookup_team($teamid);
    } else {
        return null;
    }
}

?>
