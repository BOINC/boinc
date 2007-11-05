<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/credit.inc");
require_once("../inc/email.inc");
require_once("../inc/util.inc");

function parse_project($f) {
    $p->total_credit = 0.0;
    $p->expavg_credit = 0.0;
    while (!feof($f)) {
        $buf = fgets($f);
        if (strstr($buf, "</project>")) break;
        if ($x = parse_element($buf, "<name>")) {
            $p->name = $x;
        }
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
    $url = "http://boinc.netsoft-online.com/get_user.php?cpid=$cpid";
    $f = fopen($url, "r");
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
        <td align=right>".number_format($project->total_credit, 0)."</td>
        <td align=right>".number_format($project->expavg_credit, 0)."</td>
        <td align=right>".date_str($project->create_time)."</td>
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
            echo "<h3>Projects in which you are participating</h3>";
        } else {
            echo "<h3>Projects in which $user->name is participating</h3>";
        }
        start_table();
        row_heading_array(array(
            "Project<br><span class=note>Click for user page</span>", "Total credit", "Average credit", "Since"
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
    row1(tra("Work done"));
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    show_credit($user);
    $config = get_config();
    if (parse_bool($config, "show_results")) {
        row2(tra("Pending credit"), "<a href=\"pending.php\">".tra("View")."</a>");
    }
    row2(tra("Computers on this account"),
        "<a href=\"hosts_user.php\">".tra("View")."</a>"
    );
    row2(tra("Tasks"), "<a href=\"results.php?userid=$user->id\">".tra("View")."</a>");
    $cpid = md5($user->cross_project_id . $user->email_addr);
    $x = "";
    shuffle($cpid_stats_sites);
    foreach ($cpid_stats_sites as $site) {
        $name = $site[0];
        $y = sprintf($site[1], $cpid);
        $x .= "<a href=\"$y\">$name</a><br>";
    }
    $x .= "<br><span class=\"smalltext\">".tra("Cross-project ID").": $cpid</span>\n";
    row2(tra("Cross-project statistics"), $x);
    row2(tra("Stats on your cell phone"), URL_BASE."userw.php?id=$user->id");
    row2(tra("Account number")."<br><span class=\"smalltext\">".tra("Used in URLs")."</span>", $user->id);
}

// show static user info (private)
//
function show_user_info_private($user) {
    if (is_valid_email_addr($user->email_addr)) {
        $email_text = $user->email_addr;
    } else {
        $email_text = "Verification pending";
    }

    row1(tra("Account information"));
    row2(tra("Email address"), $email_text);
    row2(tra("Name"), $user->name);
    if (strlen($user->url)) {
        $x = "http://$user->url";
    } else {
        $x = "none";
    }
    row2(tra("URL"), $x);
    row2(tra("Country"), $user->country);
    row2(tra("Postal code"), $user->postal_code);
    row2(tra("Change"), "<a href=\"edit_email_form.php\">".tra("email address")."</a> | <a href=\"edit_passwd_form.php\">".tra("password")."</a> | <a href=\"edit_user_info_form.php\">".tra("other account info")."</a>");
    row2("", "<a href=\"logout.php?".url_tokens($user->authenticator)."\">".tra("Log out")."</a>");

    row1(tra("Community"));

    if ($user->has_profile) {
        $x = "<a href=\"view_profile.php?userid=$user->id\">".tra("View or edit")."</a> | <a href=\"delete_profile.php\">".tra("Delete")."</a>";
    } else {
        $x = "<a href=\"create_profile.php\">".tra("Create")."</a>";
    }
    row2(tra("Profile"), $x);
    $tot = total_posts($user);
    if ($tot) {
        row2(tra("Message boards"), "<a href=".URL_BASE."/forum_user_posts.php?userid=$user->id>".tra("%1 posts", $tot)."</a>");
    }
    
    row2(tra("Private messages"), pm_notification($user));

    row1(tra("Teams"));
    if ($user->teamid) {
        $team = lookup_team($user->teamid);
        $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>
            | <a href=\"team_quit_form.php\">".tra("Quit team")."</a>";
        if ($team->userid == $user->id) {
            $x .= " | <a href=\"team_manage.php\">".tra("management functions")."</a>";
        }
        row2(tra("Team"), $x);
    } else {
        row2(tra("Team"), tra("None")." | <a href=\"team_search.php\">".tra("find a team")."</a>");
    }
    
    $teams_founded = BoincTeam::enum("userid=$user->id");
    foreach ($teams_founded as $team) {
        if ($team->id != $user->teamid) {
            row2(tra("Founder but not member of"), "<a href=\"team_display.php?teamid=$team->id\">$team->name</a> | <a href=\"team_manage.php?teamid=".$team->id."\">".tra("management functions")."</a>");
        }
    }

    row1("<a name=\"prefs\"></a>".tra("Preferences"));
    row2(
        tra("General preferences")."<br><span class=\"smalltext\">".tra("specify when and how BOINC uses your computer")."</span>",
        "<a href=\"prefs.php?subset=global\">".tra("View or edit")."</a>"
    );
    row2(tra("%1 preferences", PROJECT)."<br><span class=\"smalltext\">".tra("control resource share and customize graphics")."</span>",
        "<a href=\"prefs.php?subset=project\">".tra("View or edit")."</a>"
    );
    row2(tra("Message board preferences")."<br><span class=\"smalltext\">".tra("configure features and appearance of message boards")."</span>",
        "<a href=\"edit_forum_preferences_form.php\">".tra("View or edit")."</a>"
    );
}

// show summary of dynamic and static info (public)
//
function show_user_summary_public($user) {
    row2(PROJECT." member since", date_str($user->create_time));
    row2("Country", $user->country);
    if (strlen($user->url)) {
        row2("URL", "<a href=\"http://$user->url\">http://$user->url</a>");
    }
    show_credit($user);

    if ($user->teamid && ($team = lookup_team($user->teamid))) {
        row2("Team", "<a href=\"".URL_BASE."/team_display.php?teamid=$team->id\">$team->name</a>");
    } else {
        row2("Team", "None");
    }
    if ($user->show_hosts) {
        row2("Computers", "<a href=\"".URL_BASE."/hosts_user.php?userid=$user->id\">View</a>");
    } else {
        row2("Computers", "hidden");
    }
    $tot = total_posts($user);
    if ($tot) {
        row2("Message boards", "<a href=\"".URL_BASE."/forum_user_posts.php?userid=$user->id\">$tot posts</a>");
    }

    if ($user->donated == 1) { 
        if (file_exists("../project/donations.inc")) {
            require_once("../project/donations.inc");
            $x .= DONATION_LINK;
            row2("Donor",$x);
        }
    }
    
    row2("Contact", "<a href=\"forum_pm.php?action=new&amp;userid=".$user->id."\">Send private message</a>");
}

function show_profile_link($user) {
    if ($user->has_profile) {
        row2("Profile", "<a href=\"view_profile.php?userid=$user->id\">View</a>");
    }
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    $config = get_config();
    start_table("width=\"100%\"");
    show_user_info_private($user);
    show_user_stats_private($user);

    // Does this project accept donations? Then put in a project specific
    // function to show user donation information in ../project/donations.inc
    //
    if (parse_bool($config, "donations_accepted")) {
        if (file_exists("../project/donations.inc")) {
            require_once("../project/donations.inc");
            show_user_donations_private($user);
        }
    }
    end_table();
}

function make_user(
    $email_addr, $name, $passwd_hash,
    $country=null, $postal_code=null, $project_prefs=null, $teamid=0
) {
    if (!is_valid_email_addr($email_addr)) return null;
    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    if (!is_valid_country($country)) return null;
    $country = BoincDb::escape_string($country);
    $postal_code = strip_tags(process_user_text($postal_code));

    $ret = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, 1, 1, '$cross_project_id', '$passwd_hash')");

    if ($ret) {
        $db = BoincDb::get();
        $id = $db->insert_id();
        return BoincUser::lookup_id($id);
    } else {
        return null;
    }
}

?>
