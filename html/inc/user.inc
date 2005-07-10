<?php

require_once("../inc/credit.inc");
require_once("../inc/email.inc");

function pending_credit($user) {
    $result = mysql_query("select sum(claimed_credit) as total from result where userid=$user->id and (validate_state=0 or validate_state=4)");
    $foobar = mysql_fetch_object($result);
    if (!$foobar) return 0;
    mysql_free_result($result);
    return $foobar->total;
}

function show_credit($user) {
    row2("Total credit", format_credit($user->total_credit));
    row2("Recent average credit", format_credit($user->expavg_credit));
    project_user_credit($user);
}

// show dynamic user info (private)
//
function show_user_stats_private($user) {
    row1("Account statistics");
    row2(PROJECT." member since", date_str($user->create_time));
    show_credit($user);
    row2("Pending credit", "<a href=pending.php>View</a>");
    if ($user->teamid) {
        $team = lookup_team($user->teamid);
        $x = "<a href=team_display.php?teamid=$team->id>$team->name</a>
            | <a href=team_quit_form.php>Quit this team</a>";
        if ($team->userid == $user->id) {
            $x .= " | <a href=team_manage.php>management functions</a>";
        }
        row2("Team", $x);
    } else {
        row2("Team", "None (<a href=team.php>find a team</a>)");
    }
    row2("Computers on this account", "<a href=hosts_user.php>View computers</a>");
    row2("Results", "<a href=results.php?userid=$user->id>View</a>");
    row2("Stats on your cell phone", URL_BASE."userw.php?id=$user->id");
    if ($user->posts) {
        row2("Message board posts", "<a href=forum_user_posts.php?userid=$user->id>$user->posts messages</a>");

    }
}

// show static user info (private)
//
function show_user_info_private($user) {
    if (is_valid_email_addr($user->email_addr)) {
        $email_text = $user->email_addr;
    } else {
        $email_text = "Verification pending";
    }

    row1("Account information");
    row2("Email address<br>
        <font size=-2><a href=edit_email_form.php>Edit</a></font>",
        $email_text
    );
    row2("Name", $user->name);
    if (strlen($user->url)) {
        $x = "http://$user->url";
    } else {
        $x = "none";
    }
    row2("URL", $x);
    row2("Country", $user->country);
    row2("Postal code", $user->postal_code);
    row2("", "<a href=edit_user_info_form.php>Edit account info</a>");

    row1("Profile");
    $sql = "SELECT * FROM profile WHERE userid = ".$user->id;
    $result = mysql_query($sql);

    if (mysql_num_rows($result) != 0) {
        row2("", "<a href=view_profile.php?userid=$user->id>View or edit profile</a>");
        row2("", "<a href=delete_profile.php>Delete profile</a>");
    } else {
        row2("", "<a href=create_profile.php>Create</a>");
    }

    row1("<a name=prefs>Preferences");
    row2(
        "General<br><font size=-2>specify when and how BOINC uses your computer</font>",
        "<a href=prefs.php?subset=global>View or edit general preferences</a>"
    );
    row2(PROJECT."<br><font size=-2>control resource share and customize graphics</font>",
        "<a href=prefs.php?subset=project>View or edit ".PROJECT." preferences</a>"
    );
    row2("Forum<br><font size=-2>configure features and the appearance of the forum</font>",
        "<a href=\"edit_forum_preferences_form.php\">View or edit forum preferences</a>"
    );
}

// show summary of dynamic and static info (public)
//
function show_user_summary_public($user) {
    row2(PROJECT." member since", date_str($user->create_time));
    row2("Country", $user->country);
    if (strlen($user->url)) {
        row2("URL", "<a href=http://$user->url>http://$user->url</a>");
    }
    show_credit($user);

    if ($user->teamid && ($team = lookup_team($user->teamid))) {
        row2("Team", "<a href=".URL_BASE."team_display.php?teamid=$team->id>$team->name</a>");
    } else {
        row2("Team", "None");
    }
    if ($user->show_hosts) {
        row2("Computers", "<a href=".URL_BASE."hosts_user.php?userid=$user->id>View</a>");
    } else {
        row2("Computers", "hidden");
    }
    if ($user->posts) {
        row2("Message board posts", "<a href=".URL_BASE."forum_user_posts.php?userid=$user->id>$user->posts</a>");
    }
}

function show_profile_link($user) {
    if ($user->has_profile) {
        row2("Profile", "<a href=view_profile.php?userid=$user->id>View</a>");
    }
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    start_table("width=100%");
    show_user_info_private($user);
    show_user_stats_private($user);
    row1("Other");
    row2("", "<a href=logout.php>Log out</a>");
    row2("Account number<br><font size=-2>Used in URLs</font>", $user->id);
    $x = $user->cross_project_id . $user->email_addr;
    row2(
        "Cross-project ID<br><font size=-2>Used in <a href=http://boinc.berkeley.edu/db_dump.php>exported accounting data</a></font>",
        md5($x)
    );
    end_table();
}

function user_table_start($sort_by) {
    start_table();
    echo "
        <tr>
        <th>".tr(USER_TABLE_RANK)."</th>
        <th>".tr(USER_TABLE_NAME)."</th>
    ";
    if ($sort_by == "total_credit") {
        echo "
            <th><a href=top_users.php?sort_by=expavg_credit>".tr(USER_TABLE_EXPAVG)."</a></th>
            <th>".tr(USER_TABLE_TOTAL_CREDIT)."</th>
        ";
    } else {
        echo "
            <th>".tr(USER_TABLE_EXPAVG)."</th>
            <th><a href=top_users.php?sort_by=total_credit>".tr(USER_TABLE_TOTAL_CREDIT)."</a></th>
        ";
    }
    echo "
        <th>".tr(USER_TABLE_COUNTRY)."</th>
        <th>".tr(USER_TABLE_PTIME)."</th>
        </tr>
    ";
}

function show_user_row($user, $i) {
    echo "
        <tr class=row1>
        <td>$i</td>
        <td>", user_links($user), "</td>
        <td>", format_credit($user->expavg_credit), "</td>
        <td>", format_credit($user->total_credit), "</td>
        <td>", $user->country, "</td>
        <td>", time_str($user->create_time),"</td>
        </tr>
    ";
}

// decay a user's average credit
//
function user_decay_credit($user) {
    $avg = $user->expavg_credit;
    $avg_time = $user->expavg_time;
    $now = time(0);
    update_average($now, 0, 0, $avg, $avg_time);
    mysql_query("update user set expavg_credit=$avg, expavg_time=$now where id=$user->id");

}
// if the user hasn't received new credit for ndays,
// decay its average and return true
//
function user_inactive_ndays($user, $ndays) {
    $diff = time() - $user->expavg_time;
    if ($diff > $ndays*86400) {
        user_decay_credit($user);
        return true;
    }
    return false;
}

?>
