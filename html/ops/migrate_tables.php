#!/usr/local/bin/php
<?php

// TODO: Find out what to insert for team create_times, since that value
// isn't stored in the SETI@Home database.  Should we just set them all to
// the date of migration?

//define('SETI_IMAGE_PATH', '/disks/kosh/a/inet_services/www/share/htdocs/SetiAtHome/images/user_profile/');
//define('BOINC_IMAGE_PATH','/disks/koloth/a/inet_services/www/share/projects/AstroPulse_Beta/html_user/user_profile/images/');

// NOTE: These are only test values!  The commented values above are closer
// to the real thing.

define('SETI_IMAGE_PATH', '/disks/kodos/a/inet_services/boinc_www/share/projects/tah/html_ops/');
define('BOINC_IMAGE_PATH', '/disks/kodos/a/inet_services/boinc_www/share/projects/tah/html_user/user_profile/images/');

define('USER_FILE', 'test_user_flat');
define('TEAM_FILE', 'test_teams_flat');
define('FEEDBACK_FILE', 'test_feedback_flat');

mysql_connect();
mysql_select_db('boincadm_tah');

$user_field_map = array('seti_id', NULL, 'email_addr', 'name', NULL, 'create_time', NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'country', 'postal_code', NULL, NULL, NULL, 'teamid', NULL, 'url', NULL, NULL);
$team_field_map = array('seti_id', 'userid', 'name', 'name_lc', 'description', 'url', NULL, NULL, NULL, 'nusers', 'type', 'name_html');
$feedback_field_map = array(NULL, 'userid', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'has_picture', 'response1', 'response2', NULL, NULL, NULL, NULL, NULL);

migrate_tables();

function migrate_tables() {
    global $user_field_map;
    global $team_field_map;
    global $feedback_field_map;

    $num = 1;

    print "Migrating user table...\n";

    // User table
    $fd = fopen(USER_FILE, 'r');
    if (!$fd) {
        echo "Error opening '", USER_FILE, "' - exiting.\n";
        exit();
    }

    while ($user_data = get_row($fd, $user_field_map, '|')) {
        $authenticator = random_string();
        print "\nUser Record #$num\n----------\n";
        $query = "INSERT INTO user SET authenticator = '" . $authenticator . "', ";
        $first_field = true;

        for ($i = 0; $i < count($user_field_map); $i++) {
            if (!is_null($user_field_map[$i]) && !is_null($user_data) && !empty($user_data[$i])) {

                if ($user_field_map[$i] == 'email_addr') {
                    $user_data[$i] = munge_email_addr($user_data[$i], $authenticator);
                } else if ($user_field_map[$i] == 'create_time') {
                    $user_data[$i] = jd_to_unix($user_data[$i]);
                }

                print $user_field_map[$i] . ": " . addslashes(trim($user_data[$i])) . "\n";

                if (!$first_field) {
                    $query = $query . ", ";
                } else {
                    $first_field = false;
                }
                $query = $query . $user_field_map[$i] . " = '" . addslashes(trim($user_data[$i])) . "'";
            }
        }

        echo "QUERY: $query\n";
        mysql_query($query);
        $num++;
    }
    fclose($fd);
    $num = 1;

    // Feedback table.

    // TODO: Make sure we can assume that there will be a user corresponding
    // to every feeback entry.  (Can't see why this wouldn't be the case.)

    $fd = fopen(FEEDBACK_FILE, 'r');
    if (!$fd) {
        echo "Error opening '", FEEDBACK_FILE,"' - exiting.\n";
        exit();
    }

    while ($feedback_data = get_row($fd, $feedback_field_map, '|')) {
        print "\nFeedback Record #$num\n----------\n";
        $query = "INSERT INTO profile SET ";
        $first_field = true;
        $new_userid = NULL;

        for ($i = 0; $i < count($feedback_field_map); $i++) {
            if (!is_null($feedback_field_map[$i]) && !is_null($feedback_data) && !empty($feedback_data[$i])) {

                // We know by the above test that the has_picture field is
                // not empty in this case.

                if ($feedback_field_map[$i] == 'userid') {
                    $result = mysql_query("SELECT * FROM user WHERE seti_id = " . $feedback_data[$i]);
                    if ($result) {
                        $row = mysql_fetch_object($result);
                    } else {
                        print "DB error.\n";
                        exit();
                    }
                    $feedback_data[$i] = $row->id;
                    $new_userid = $row->id;
                } else if ($feedback_field_map[$i] == 'has_picture') {

                    // Because the userid field comes before the picture
                    // field in the feedback file, we know that $new_userid
                    // will have been set by the time we get here.
                    move_user_pic($feedback_data[$i], $new_userid);
                    $feedback_data[$i] = '1';
                }

                print $feedback_field_map[$i] . ": " . addslashes(clean_newlines(trim($feedback_data[$i]))) . "\n";

                if (!$first_field) {
                    $query = $query . ", ";
                } else {
                    $first_field = false;
                }
                $query = $query . $feedback_field_map[$i] . " = '" . addslashes(clean_newlines(trim($feedback_data[$i]))) . "'";
            }
        }

        echo "QUERY: $query\n";
        mysql_query($query);
        $num++;
    }
    fclose($fd);
    $num = 1;

    print "Migrating team table...\n";

    // Team table
    $fd = fopen(TEAM_FILE, 'r');
    if (!$fd) {
        echo "Error opening '", TEAM_FILE, "' - exiting.\n";
        exit();
    }

    while ($team_data = get_row($fd, $team_field_map, '|')) {
        print "\nTeam Record #$num\n----------\n";
        $query = "INSERT INTO team SET ";
        $first_field = true;

        for ($i = 0; $i < count($team_field_map); $i++) {
            if (!is_null($team_field_map[$i]) && !is_null($team_data) && !empty($team_data[$i])) {

                // Translate old userID -> new userID
                if ($team_field_map[$i] == 'userid') {

                    $result = mysql_query("SELECT * FROM user WHERE seti_id = " . $team_data[$i]);
                    if ($result) {
                        $row = mysql_fetch_object($result);
                    } else {
                        print "DB error.\n";
                        exit();
                    }

                    $team_data[$i] = $row->id;
                }

                print $team_field_map[$i] . ": " . addslashes(clean_newlines(trim($team_data[$i]))) . "\n";

                if (!$first_field) {
                    $query = $query . ", ";
                } else {
                    $first_field = false;
                }
                $query = $query . $team_field_map[$i] . " = '" . addslashes(clean_newlines(trim($team_data[$i]))) . "'";
            }
        }
        echo "QUERY: $query\n";
        mysql_query($query);
        $num++;
    }
    fclose($fd);

    // Wrapping up.
    print "Re-linking team IDs and sending notification emails...\n";

    $result = mysql_query("SELECT * FROM user WHERE seti_id IS NOT NULL");

    while ($user = mysql_fetch_object($result)) {
        // Relink team ID.
        if (!is_null($user->teamid)) {
            $result2 = mysql_query("SELECT * FROM team WHERE seti_id = " . $user->teamid);
            $team = mysql_fetch_object($result2);
            $result2 = mysql_query("UPDATE user SET teamid = " . $team->id . " WHERE id = " . $user->id);
        }

        // Send out email.
        split_munged_email_addr($user->email_addr, $user->authenticator, $address);

        $body = "Your SETI@Home account has been transferred to BOINC.  Your authenticator is\n\n" . $user->authenticator;
        //mail($address, "Your SETI@Home BOINC account is ready!", $body);

    }

    print "Migration Complete.\n";

}

function clean_newlines($string) {
    return ereg_replace('\\\0d\\\0a', "\n", $string);
}

/** Flat file traversal functions
 */


function get_row($descriptor, $keys, $delim) {
    $data = NULL;
    $empty = true;

    for ($i=0; $i < count($keys); $i++) {
        $field = get_next_field($descriptor, $delim);
        if (!is_null($keys[$i])) {
            $data[$i] = $field;
            if (!empty($field)) $empty = false;
        }
    }

    if ($empty) {
        return NULL;
    }

    return $data;
}

function get_next_field($descriptor, $delim) {
    $data = NULL;
    $pos = 0;
    while (($char = fgetc($descriptor)) !== FALSE) {
        if ($char == $delim) {
            if (!($pos > 0 && $data[($pos-1)] == '\\')) {
                return $data;
            }
        }
        $data = $data . $char;
        $pos++;
    }
    return NULL;
}
/** Functions copied over from util.inc
 */

function random_string() {
    return md5(uniqid(rand()));
}

function munge_email_addr($email, $string) {
    return "@".$email."_".$string;
}

// if email_addr is of the form @X_Y, split out the X and return true.
// otherwise return false
//
function split_munged_email_addr($addr, $string, &$email) {
    if (substr($addr, 0, 1) != "@") return false;
    $x = strrchr($addr, "_");
    if (!$x) return false;
    $y = substr($x, 1);
    if ($y != $string) return false;
    $email = substr($addr, 1, strlen($addr)-strlen($x)-1);
    return true;
}

function jd_to_unix($raw_jd) {
    $jd = floor($raw_jd);
    $jtime = $raw_jd - $jd;

    $total_secs = 86400 * ($raw_jd - $jd);

    $hours = floor($total_secs / 3600);
    $minutes = floor(($total_secs - (3600 * $hours)) / 60);
    $seconds = floor(($total_secs - (3600 * $hours) - (60 * $minutes)));

// Calculate Gregorian date
    $l = $jd + 68569;
    $n = floor(( 4 * $l ) / 146097);
    $l = $l - floor((146097 * $n + 3 ) / 4);
    $i = floor(( 4000 * ( $l + 1 ) ) / 1461001);
    $l = floor($l - ( 1461 * $i ) / 4 + 31);
    $j = floor(( 80 * $l ) / 2447);
    $d = floor($l - ( 2447 * $j ) / 80);
    $l = floor($j / 11);
    $m = $j + 2 - ( 12 * $l );
    $y = 100 * ( $n - 49 ) + $i + $l;

    return mktime($hours, $minutes, $seconds, $m, $d, $y);
}

function move_user_pic($img_name, $userid) {
    $img_name = trim($img_name);

    $filename = SETI_IMAGE_PATH . $img_name;
    $filename_sm = SETI_IMAGE_PATH . 'sm_' . $img_name;

    if (file_exists($filename)) {
        $dotpos = strrpos($img_name, ".");
        $name = substr($img_name, 0, $dotpos);
        $ext = strrchr($img_name, ".");

        if ($ext == '.jpg' || $ext == '.jpeg') {
            shell_exec("cp $filename " . BOINC_IMAGE_PATH . $userid . ".jpg");
            shell_exec("cp $filename_sm " . BOINC_IMAGE_PATH . $userid . "_sm.jpg");
            return true;
        } else if ($ext == '.png' || $ext == '.gif') {
            shell_exec("convert $filename jpg:" . BOINC_IMAGE_PATH . $userid . ".jpg");
            shell_exec("convert $filename_sm jpg:" . BOINC_IMAGE_PATH . $userid . "_sm.jpg");
            return true;
        } else {
            print "Failed to move image $filename";
            return false;
        }
    }
    print "Image $filename does not exist!\n";
    return false;
}
