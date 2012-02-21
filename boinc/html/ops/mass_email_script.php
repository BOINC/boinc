#! /usr/bin/env php

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

// mass_email_script [--userid N] [--send] [--idfile X] [--batch N]
//
// send mass email.  Options:
// --userid     send only to the given user
// --send       actually send email
// --show_email show what would be sent rather than sending it
// --explain    show what we're doing
// --idfile     read user ID from the given file; otherwise send to everyone.
//              The IDs must be in increasing order
// --nocurrent  Don't send to current users
// --batch N    Do batches of N (default 1000)
//
// NOTE: a file "email_log" is used for checkpoint/restart.
// It stores a list of user IDs sent to.
// You must create this file (use touch) before starting
// (this is to prevent you from accidentally running this
// in the wrong directory and re-mailing a lot of people)
//
// see http://boinc.berkeley.edu/mass_email.php for info

$cli_only = true;
require_once('../project/project.inc');
require_once('../inc/email.inc');
require_once('../inc/db.inc');
require_once('../inc/util_ops.inc');

db_init();
set_time_limit(0);

$globals->send = false;
$globals->explain = false;
$globals->userid = 0;
$globals->nocurrent = false;
$globals->idfile = null;
$globals->batch = 1000;
$globals->lapsed_interval = 60*86400;

for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--batch") {
        $i++;
        $globals->batch = $argv[$i];
    } elseif ($argv[$i] == "--show_email") {
        $globals->show_email = true;
    } elseif ($argv[$i] == "--explain") {
        $globals->explain = true;
    } elseif ($argv[$i] == "--send") {
        $globals->send = true;
    } elseif ($argv[$i] == "--idfile") {
        $i++;
        $globals->idfile = $argv[$i];
    } elseif ($argv[$i] == "--userid") {
        $i++;
        $globals->userid = $argv[$i];
    } else {
        echo "unrecognized option $argv[$i]\n";
        echo "usage: mass_email_script.php [--userid N] [--show_mail] [--explain] [--send]\n";
        exit (1);
    }
}

$mass_email_log = 'email_log';

// File names for the various mail types.
// Change these here if you like.

$email_failed_html = 'newsletter/failed_html';
$email_failed_text = 'newsletter/failed_text';
$email_failed_subject = 'newsletter/failed_subject';
$email_lapsed_html = 'newsletter/lapsed_html';
$email_lapsed_text = 'newsletter/lapsed_text';
$email_lapsed_subject = 'newsletter/lapsed_subject';
$email_current_html = 'newsletter/current_html';
$email_current_text = 'newsletter/current_text';
$email_current_subject = 'newsletter/current_subject';

function read_files(&$item) {
    $item['html'] = file_get_contents($item['html_file']);
    if (!$item['html']) {
        $x = $item['html_file'];
        echo "file missing: $x\n";
        exit();
    }
    $item['text'] = file_get_contents($item['text_file']);
    if (!$item['text']) {
        $x = $item['text_file'];
        echo "file missing: $x\n";
        exit();
    }
    $item['subject'] = file_get_contents($item['subject']);
    if (!$item['subject']) {
        $x = $item['subject'];
        echo "file missing: $x\n";
        exit();
    }
}

function read_email_files() {
    global $globals;
    global $email_failed_html;
    global $email_failed_text;
    global $email_failed_subject;
    global $email_lapsed_html;
    global $email_lapsed_text;
    global $email_lapsed_subject;
    global $email_current_html;
    global $email_current_text;
    global $email_current_subject;

    $failed['html_file'] = $email_failed_html;
    $failed['text_file'] = $email_failed_text;
    $failed['subject'] = $email_failed_subject;
    $lapsed['html_file'] = $email_lapsed_html;
    $lapsed['text_file'] = $email_lapsed_text;
    $lapsed['subject'] = $email_lapsed_subject;
    $current['html_file'] = $email_current_html;
    $current['text_file'] = $email_current_text;
    $current['subject'] = $email_current_subject;
    read_files($failed);
    read_files($lapsed);
    if (!$globals->nocurrent) {
        read_files($current);
    }
    $email_files['failed'] = $failed;
    $email_files['lapsed'] = $lapsed;
    $email_files['current'] = $current;
    return $email_files;
}

function last_rpc_time($user) {
    $x = 0;
    $result = mysql_query("select rpc_time from host where userid=$user->id");
    while ($host = mysql_fetch_object($result)) {
        if ($host->rpc_time > $x) $x = $host->rpc_time;
    }
    mysql_free_result($result);
    return $x;
}

function replace($user, $template) {
    $pat = array(
        '/<name\/>/',
        '/<create_time\/>/',
        '/<total_credit\/>/',
        '/<opt_out_url\/>/',
        '/<lapsed_interval\/>/',
    );
    $rep = array(
        $user->name,
        gmdate('d F Y', $user->create_time),
        number_format($user->total_credit, 0),
        opt_out_url($user),
        floor((time() - $user->last_rpc_time) / 86400),
    );
    return preg_replace($pat, $rep, $template);
}

function mail_type($user, $email_file) {
    global $globals;

    $html = replace($user, $email_file['html']);
    $text = replace($user, $email_file['text']);
    if ($globals->show_email) {
        echo "\nSending to $user->email_addr:\n";
        echo "------- SUBJECT ----------\n";
        echo $email_file['subject'];
        echo "\n------- HTML ----------\n";
        echo $html;
        echo "\n------- TEXT ----------\n";
        echo $text;
    }
    if ($globals->send) {
        if (is_valid_email_addr($user->email_addr)) {
            send_email(
                $user,
                $email_file['subject'],
                $text,
                $html
            );
        } else {
            if ($globals->explain) {
                echo "invalid e-mail address\n";
            }
        }
    }
}

function handle_user($user) {
    global $email_files;
    global $globals;

    $user->last_rpc_time = last_rpc_time($user);
    $lapsed = time() - $user->last_rpc_time > $globals->lapsed_interval;

    if ($user->total_credit == 0) {
        mail_type($user, $email_files['failed']);
        if ($globals->explain) {
            echo "sending failed email to $user->email_addr\n";
        }
    } else if ($lapsed) {
        mail_type($user, $email_files['lapsed']);
        if ($globals->explain) {
            echo "sending lapsed email to $user->email_addr\n";
        }
    } else {
        if (!$globals->nocurrent) {
            mail_type($user, $email_files['current']);
            if ($globals->explain) {
                echo "sending current email to $user->email_addr\n";
            }
        }
    }
}

function do_batch($startid, $n, $log) {
    $result = mysql_query(
        "select * from user where id>$startid order by id limit $n"
    );
    while ($user = mysql_fetch_object($result)) {
        handle_user($user);
        $startid = $user->id;
        fputs($log, $user->id . "\n");
        fflush($log);
    }
    mysql_free_result($result);
    return $startid;
}

function do_one($thisid, $log) {
    $result = mysql_query(
        "select * from user where id=$thisid"
    );
    $user = mysql_fetch_object($result);
    if ($user) {
        handle_user($user);
        fputs($log, $user->id . "\n");
        fflush($log);
    }
    mysql_free_result($result);
    return $startid;
}

function read_log() {
    global $mass_email_log;
    $f = fopen($mass_email_log, 'r');
    if (!$f) {
        echo "$mass_email_log not found - create empty file and run again\n";
        exit();
    }
    $startid = 0;
    while (fscanf($f, '%d', &$startid)) {
        // read to the last entry in the file
    }
    fclose($f);
    return $startid;
}

function main() {
    global $globals;
    global $id_file;
    global $mass_email_log;
    $startid = read_log();
    $f = fopen($mass_email_log, 'a');
    if ($id_file == "") {
        while (1) {
            $new_startid = do_batch($startid, $globals->batch, $f);
            if ($new_startid == $startid) break;
            $startid = $new_startid;
        }
    } else {
        $fid = fopen($id_file, 'r'); 
        if (!$fid) {
            echo  $id_file . ' not found - create ID list and run again\n';
            exit();
        }
        $thisid = 0;
        while (fscanf($fid, '%d', &$thisid)) {
            if ($thisid > $startid) {
                do_one($thisid, $f);
            }
        }
        fclose($fid);
    }
    echo 'All done!' . "\n";
}

if (!$USE_PHPMAILER) {
    echo "You must use PHPMailer.\n";
    exit();
}

$email_files = read_email_files();

if ($globals->userid) {
    $user = lookup_user_id($globals->userid);
    if (!$user) {
        echo "no such user\n";
    } else {
        $user->last_rpc_time = last_rpc_time($user);
        mail_type($user, $email_files['failed']);
        mail_type($user, $email_files['lapsed']);
        if (!$globals->nocurrent) {
            mail_type($user, $email_files['current']);
        }
    }
} else {
    main();
}

?>
