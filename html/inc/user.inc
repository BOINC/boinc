<?php

function pending_credit($user) {
    $result = mysql_query("select sum(claimed_credit) as total from result where userid=$user->id and validate_state=0");
    $foobar = mysql_fetch_object($result);
    if (!$foobar) return 0;
    mysql_free_result($result);
    return $foobar->total;
}

// show dynamic user info (private)
//
function show_user_stats_private($user) {
    row1("Account statistics");
    row2(PROJECT." member since", time_str($user->create_time));
    row2("Total credit", format_credit($user->total_credit));
    row2("Pending credit", "<a href=pending.php>View</a>");
    row2("Recent average credit", format_credit($user->expavg_credit));
    $team_header = "Team<br><font size=-2><a href=team.php>Change</a></font>";
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row2($team_header, "<a href=team_display.php?teamid=$team->id>$team->name</a>");
    } else {
        row2($team_header, "None");
    }
    row2("Computers", "<a href=hosts_user.php>View</a>");
    row2("Results", "<a href=results.php?userid=$user->id>View</a>");
    if ($user->posts) {
        row2("Message board posts", "<a href=forum/user_posts.php?userid=$user->id>$user->posts messages</a>");

    }
}

// show static user info (private)
//
function show_user_profile_private($user) {
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
    row2("URL", "http://$user->url");
    row2("Country", $user->country);
    row2("Postal code", $user->postal_code);
    if ($user->signature) {
        $x = "<pre>".htmlspecialchars($user->signature)."</pre>";
        row2("Forums signature", $x);
    }
    row2("", "<a href=edit_user_info_form.php>Edit account info</a>");

    row1("Profile");
    $sql = "SELECT * FROM profile WHERE userid = ".$user->id;
    $result = mysql_query($sql);

    if (mysql_num_rows($result) != 0) {
      row2("", "<a href=view_profile.php?userid=$user->id>View / Edit</a>");
      row2("", "<a href=delete_profile.php>Delete</a>");
    } else {
      row2("", "<a href=create_profile.php>Create</a>");
    }

    row1("Preferences");
    row2("General", "<a href=prefs.php?subset=global>View / Edit</a>");
    row2(PROJECT, "<a href=prefs.php?subset=project>View / Edit</a>");
}

// show summary of dynamic and static info (public)
function show_user_summary_public($user) {
    row1("Account data for $user->name");
    row2(PROJECT." member since", time_str($user->create_time));
    row2("Country", $user->country);
    if (strlen($user->url)) {
        row2("URL", "<a href=http://$user->url>http://$user->url</a>");
    }
    row2("Total credit", format_credit($user->total_credit));
    row2("Recent average credit", format_credit($user->expavg_credit));

    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row2("Team", "<a href=team_display.php?teamid=$team->id>$team->name</a>");
    } else {
        row2("Team", "None");
    }
    if ($user->show_hosts) {
        row2("Computers", "<a href=hosts_user.php?userid=$user->id>View</a>");
    } else {
        row2("Computers", "hidden");
    }
    if ($user->posts) {
        row2("Message board posts", "<a href=forum/user_posts.php?userid=$user->id>$user->posts</a>");
    }
    if ($user->has_profile) {
        row2("Profile", "<a href=view_profile.php?userid=$user->id>view</a>");
    }
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    start_table("width=100%");
    show_user_profile_private($user);  // NOTE: Will include the actual user profile.
    show_user_stats_private($user);
    end_table();
}

function user_table_start() {
    start_table();
    echo "
        <tr>
        <th>Rank</th>
        <th>Name</th>
        <th><a href=top_users.php?sort_by=expavg_credit>Recent average credit</a></th>
        <th><a href=top_users.php?sort_by=total_credit>Total credit</a></th>
        <th>Country</th>
        <th>Participant since</th>
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

?>
