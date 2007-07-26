#!/usr/bin/env php

<?php

// fetch a list of "BOINC-wide teams" and create or update them

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

// set the following to 1 to print queries but not do anything

$dry_run = 0;

function lookup_team_seti_id($id) {
    $result = mysql_query("select * from team where seti_id=$id");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
        return $team;
    }
    return null;
}

function parse_team($f) {
    while ($s = fgets($f)) {
        if (strstr($s, '</team>')) {
            return $t;
        }
        else if (strstr($s, '<name>')) $t->name = parse_element($s, '<name>');
        else if (strstr($s, '<url>')) $t->url = parse_element($s, '<url>');
        else if (strstr($s, '<type>')) $t->type = parse_element($s, '<type>');
        else if (strstr($s, '<name_html>')) $t->name_html = parse_element($s, '<name_html>');
        else if (strstr($s, '<description>')) {
            while ($s = fgets($f)) {
                if (strstr($s, '</description>')) break;
                $t->description .= $s;
            }
        }
        else if (strstr($s, '<country>')) $t->country = parse_element($s, '<country>');
        else if (strstr($s, '<id>')) $t->id = parse_element($s, '<id>');
        else if (strstr($s, '<user_email_munged>')) {
            $user_email_munged = parse_element($s, '<user_email_munged>');
            $t->user_email = str_rot13($user_email_munged);
        }
        else if (strstr($s, '<user_name>')) $t->user_name = parse_element($s, '<user_name>');
        else if (strstr($s, '<user_country>')) $t->user_country = parse_element($s, '<user_country>');
        else if (strstr($s, '<user_postal_code>')) $t->user_postal_code = parse_element($s, '<user_postal_code>');
        else if (strstr($s, '<user_url>')) $t->user_url = parse_element($s, '<user_url>');
    }
    return null;
}

function valid_team($t) {
    if (!$t->id) return false;
    if (!$t->name) return false;
    if (!$t->user_email) return false;
    if (!$t->user_name) return false;
    return true;
}

function update_team($t, $team, $user) {
    global $dry_run;
    if (
        trim($t->url) == $team->url
        && $t->type == $team->type
        && trim($t->name_html) == $team->name_html
        && trim($t->description) == $team->description
        && $t->country == $team->country
        && $t->id == $team->seti_id
    ) {
        echo "   no changes\n";
        return;
    }
    echo "   updating\n";
    $url = process_user_text($t->url);
    $name_html = process_user_text($t->name_html);
    $description = process_user_text($t->description);
    $country = process_user_text($t->country);
    $query = "update team set url='$url', type=$t->type, name_html='$name_html', description='$description', country='$country', seti_id=$t->id where id=$team->id";
    if ($dry_run) {
        echo "   $query\n";
        return;
    }
    $retval = mysql_query($query);
    if (!$retval) {
        echo "   update failed: $query\n";
        exit;
    }
}

function insert_case($t, $user) {
    global $dry_run;
    if ($dry_run) {
        if (!$user) echo "   making user $t->user_email\n";
        echo "   making team $t->name\n";
        return;
    }
    if (!$user) {
        echo "   making user $t->user_email\n";
        $user = make_user($t->user_email, $t->user_name, random_string());
        if (!$user) {
            echo "   Can't make user $t->user_email\n";
            echo mysql_error();
            exit;
        }
    }
    echo "   making team $t->name\n";
    $team = make_team(
        $user->id, $t->name, $t->url, $t->type, $t->name_html,
        $t->description, $t->country
    );
    if (!$team) {
        echo "   Can't make team $t->id\n";
        echo mysql_error();
        echo "\n";
        exit;
    }
    mysql_query("update team set seti_id=$t->id where id=$team->id");
    mysql_query("update user set teamid=$team->id where id=$user->id");
}

// There are several cases for a given record:
// (note: "ID" means the ID coming from BOINC, stored locally in seti_id)
// insert case:
//      There's no team with given name; create one,
//      and create the user if needed
// update1 case:
//      There's a team with the given name and the given ID
//      and its founder has the right email address.
//      Update its parameters if any are different.
// update2 case:
//      There's a team with the given name and seti_id=0,
//      and its founder has the right email address.
//      Update its parameters if any are different,
//      and set its seti_id.
//      This handles the case where the team founder created the team
//      before this new system was run.
// conflict case:
//      There's a team with the given name,
//      and either it has the wrong ID
//      or its founder has a different email address.
//      Don't change anything.

// These semantics mean that:
// -    A BOINC team can't change its name via this mechanism.
//      This avoids pathological cases, e.g. if two teams swapped names,
//      the updates would always fail.
//      If a BOINC team wants to change its name,
//      it must do it manually everywhere.
// -    If a BOINC team changes its founder (or the founder changes email)
//      they'll have to make this change manually on all projects.
//      (this is better than a security vulnerability)
// -    This mechanism can't be used to update the founder's
//      account parameters on all projects

function handle_team($f) {
    $t = parse_team($f);
    if (!$t) {
        echo "Failed to parse team\n";
        return;
    }
    //print_r($t);
    //return;
    if (!valid_team($t)) {
        echo "Invalid team\n";
        return;
    }

    echo "Processing $t->name $t->user_email\n";
    $user = lookup_user_email_addr($t->user_email);
    $team = lookup_team_name($t->name);
    if ($team) {
        if (!$user) {
            echo "   team exists but user $t->user_email doesn't\n";
            return;
        }
        if ($user->id != $team->userid) {
            echo "   team exists but is owned by a different user\n";
            return;
        }
        if ($team->seti_id) {
            if ($team->seti_id == $t->id) {
                echo "   case 1\n";
                update_team($t, $team, $user);      // update1 case
            } else {
                echo "   team exists but has wrong seti_id\n";
            }
        } else {
            $team2 = lookup_team_seti_id($t->id);
            if ($team2) {
                // update1 case
                echo "   case 2\n";
                update_team($t, $team2, $user);
            } else {
                // update2 case
                echo "   case 3\n";
                update_team($t, $team, $user);
            }
        }
    } else {
        $team = lookup_team_seti_id($t->id);
        if ($team) {
            echo "   A team with same ID but different name exists;\n";
            echo "   Please report this to $t->user_email;\n";
        } else {
            echo "   Adding team\n";
            insert_case($t, $user);
        }
    }
}

function main() {
    $f = fopen("http://boinc.berkeley.edu/boinc_teams.xml", "r");
    if (!$f) {
        echo "Can't get times file\n";
        exit;
    }
    while ($s = fgets($f)) {
        if (strstr($s, '<team>')) {
            handle_team($f);
        }
    }
}

db_init();
main();

?>
