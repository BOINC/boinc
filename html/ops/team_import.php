<?php

// fetch a list of "BOINC-wide teams" and create/update them
// Note: these are identified by the seti_id field,
// allowing you to change the names

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

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

// if team belongs to given user, update it
//
function update_team($t, $team, $user) {
    if (!$user) {
        echo "   No user\n";
        return;
    }
    if ($user.id != $team.id) {
        echo "   owned by a different user\n";
        return;
    }
    if (
        $t->url == $team->url
        && $t->type == $team->type
        && $t->name_html == $team->name_html
        && $t->description == $team->description
        && $t->country == $team->country
        && $t->id == $team->seti_id
    ) {
        echo "   no changes\n";
        return;
    }
    echo "   updating\n";
    $query = "update team set url='$t->url', type=$t->type, name_html='$t->name_html', description='$t->description', country='$t->country', seti_id=$t->id";
    $retval = mysql_query($query);
    if (!$retval) {
        echo "   update failed: $query\n";
        exit;
    }
}

function insert_case($t, $user) {
    if (!$user) {
        echo "   making user\n";
        $user = make_user($t->user_email, $t->user_name, random_string());
        if (!$user) {
            echo "   Can't make user $t->user_email\n";
            echo mysql_error();
            exit;
        }
    }
    echo "   making team\n";
    $team = make_team(
        $user->id, $t->name, $t->url, $t->type, $t->name_html,
        $t->description, $t->country
    );
    if (!$team) {
        echo "   Can't make team $t->id\n";
        echo mysql_error();
        exit;
    }
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
    $user = lookup_user_email_addr($t->user_email);

    echo "Processing $t->name\n";
    $team = lookup_team_name($t->name);
    if ($team) {
        if ($team->seti_id) {
            if ($team->seti_id == $t->id) {
                update_team($t, $team, $user);      // update1 case
            }
        } else {
            $team2 = lookup_team_seti_id($t->id);
            if ($team2) {
                // update1 case
                update_team($t, $team2, $user);
            } else {
                // update2 case
                update_team($t, $team, $user);
            }
        }
    } else {
        $team = lookup_team_seti_id($t->id);
        if ($team) {
            echo "   A team with same ID but different name exists;\n";
            echo "   Please report this to $t->user_email;\n";
        } else {
            echo "   Inserting team\n";
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
