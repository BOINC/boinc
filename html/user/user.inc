<?php

// show dynamic user info (private)
//
function show_user_stats_private($user) {
    row1("Your account statistics");
    row2(PROJECT." member since", time_str($user->create_time));
    row2("Total credit", format_credit($user->total_credit));
    row2("Recent average credit", format_credit($user->expavg_credit));
    $team_header = "Team<br>
<font size=-1><a href=team.php>Change</a></font>";
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row2($team_header, "<a href=team_display.php?teamid=$team->id>$team->name</a>");
    } else {
        row2($team_header, "None");
    }
    row2("Computers", "<a href=hosts_user.php>View</a>");
}

// show static user info (private)
//
function show_user_profile_private($user) {
    if (is_valid_email_addr($user->email_addr)) {
        $email_text = $user->email_addr;
    } else {
        $email_text = "Verification pending";
    }

    row1("Your account information");
    row2("Email address<br>
        <font size=-1><a href=edit_email_form.php>Edit</a></font>",
        $email_text
    );
    row2("Name", $user->name);
    row2("URL", "http://$user->url");
    row2("Country", $user->country);
    row2("Postal code", $user->postal_code);
    row2("", "<a href=edit_user_info_form.php>Edit account info</a>");

    row1("Your profile");
    $sql = "SELECT * FROM profile WHERE userid = ".$user->id;
    $result = mysql_query($sql);

    if (mysql_num_rows($result) != 0) {
      row2("", "<a href=view_profile.php?userid=$user->id>View / Edit</a>");
      row2("", "<a href=delete_profile.php>Delete</a>");
    } else {
      row2("", "<a href=create_profile.php>Create</a>");
    }

    row1("Your preferences");
    row2("General", "<a href=prefs.php?subset=global>View / Edit</a>");
    row2(PROJECT, "<a href=prefs.php?subset=project>View / Edit</a>");
}

// show summary of dynamic and static info (public)
function show_user_summary_public($user) {
    $result = mysql_query("SELECT * FROM profile WHERE userid = $user->id");

    row1("Account data for $user->name");
    row2(PROJECT." member since", time_str($user->create_time));
    row2("Country", $user->country);
    if (strlen($user->url)) {
        row2("URL", "<a href=http://$user->url>http://$user->url</a>");
    }
    row2("Total credit", format_credit($user->total_credit));
    row2("Recent average credit", format_credit($user->expavg_credit));

    if (mysql_num_rows($result) != 0) {
        row2("Profile", "<a href=view_profile.php?userid=$user->id>View</a>");
    }

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
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    start_table();
    show_user_profile_private($user);  // NOTE: Will include the actual user profile.
    show_user_stats_private($user);
    end_table();
}

function user_table_start() {
    start_table();
    echo "
        <tr>
        <th>Name</th>
        <th>Recent average credit</th>
        <th>Total credit</th>
        <th>Country</th>
        <th>Participant since</th>
        </tr>
    ";
}

function show_user_row($user) {
    printf(
      "<tr class=row1>
        <td><a href=show_user.php?userid=%d>%s</a></td>
        <td>%s</td>
        <td>%s</td>
        <td>%s</td>
        <td>%s</td>
    </tr>\n", $user->id, $user->name, format_credit($user->expavg_credit),
      format_credit($user->total_credit), $user->country, time_str($user->create_time));
}

?>
