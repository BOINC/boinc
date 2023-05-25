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

// remind.php [--lapsed | --failed] [--show_email] [--userid ID]
//
// --lapsed
//      Send emails to lapsed user (see below)
// --failed
//      Send emails to failed user (see below)
// --userid
//      Send both "lapsed" and "failed" emails to the given user,
//      regardless of whether they are due to be sent.
//      The --lapsed and --failed options are ignored.
//      (for testing)
// --show_email
//      Show the text that would be mailed
// --explain
//      Show which users would be sent email and why
// --send
//      Actually send emails (this is an option to encourage
//      you to do thorough testing before using it)
// --count N
//      By default send to all users that qualify, but if count is
//      set, only send to N users at a time
//
// This program sends "reminder" emails to
// - failed users: those who
//   1) were created at least $start_interval seconds ago,
//   2) have zero total credit
//   3) haven't been sent an email in at least $email_interval seconds.
//   These people typically either had a technical glitch,
//   or their prefs didn't allow sending them work,
//   or the app crashed on their host.
//   The email should direct them to a web page that helps
//   them fix the startup problem.
//
//   Set $start_interval according to your project's delay bounds
//   e.g. (1 or 2 weeks).
///  $email_interval should be roughly 1 month -
//   we don't want to bother people too often.
//
// - lapsed users: those who
//   1) have positive total credit,
//   2) haven't done a scheduler RPC within the past
//      $lapsed_interval seconds, and
//   3) haven't been sent an email in at least $email_interval seconds.
//   The email should gently prod them to start running the project again.
//

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/email.inc");

db_init();
set_time_limit(0);

$globals->start_interval = 14*86400;
$globals->email_interval = 200*86400;
$globals->lapsed_interval = 60*86400;
$globals->do_failed = false;
$globals->do_lapsed = false;
$globals->show_email = false;
$globals->send = false;
$globals->explain = false;
$globals->userid = 0;
$globals->count = -1;

for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--failed") {
        $globals->do_failed = true;
    } elseif ($argv[$i] == "--lapsed") {
        $globals->do_lapsed = true;
    } elseif ($argv[$i] == "--show_email") {
        $globals->show_email = true;
    } elseif ($argv[$i] == "--explain") {
        $globals->explain = true;
    } elseif ($argv[$i] == "--send") {
        $globals->send = true;
    } elseif ($argv[$i] == "--userid") {
        $i++;
        $globals->userid = $argv[$i];
    } elseif ($argv[$i] == "--count") {
        $i++;
        $globals->count = $argv[$i];
    } else {
        echo "unrecognized option $argv[$i]\n";
        echo "usage: remind.php [--failed ] [--lapsed] [--userid N] [--show_mail] [--explain] [--send] [--count N]\n";
        exit (1);
    }
}

// File names for the various mail types.
// Change these here if needed.
//
$dir = "remind_email";
$failed_html = "$dir/reminder_failed_html";
$failed_text = "$dir/reminder_failed_text";
$failed_subject = "$dir/reminder_failed_subject";
$lapsed_html = "$dir/reminder_lapsed_html";
$lapsed_text = "$dir/reminder_lapsed_text";
$lapsed_subject = "$dir/reminder_lapsed_subject";

// return time of last scheduler RPC from this user,
// or zero if they're never done one
//
function last_rpc_time($user) {
    $x = 0;
    $result = _mysql_query("select rpc_time from host where userid=$user->id");
    while ($host = _mysql_fetch_object($result)) {
        if ($host->rpc_time > $x) $x = $host->rpc_time;
    }
    _mysql_free_result($result);
    return $x;
}

function read_files(&$item) {
    $item['html'] = @file_get_contents($item['html_file']);
    if (!$item['html']) {
        //$x = $item['html_file'];
        //echo "file missing: $x\n";
        //exit();
    }
    $item['text'] = @file_get_contents($item['text_file']);
    if (!$item['text']) {
        $x = $item['text_file'];
        echo "file missing: $x\n";
        exit();
    }
    $item['subject'] = @file_get_contents($item['subject']);
    if (!$item['subject']) {
        $x = $item['subject'];
        echo "file missing: $x\n";
        exit();
    }
}

function read_email_files() {
    global $failed_html;
    global $failed_text;
    global $failed_subject;
    global $lapsed_html;
    global $lapsed_text;
    global $lapsed_subject;

    $failed['html_file'] = $failed_html;
    $failed['text_file'] = $failed_text;
    $failed['subject'] = $failed_subject;
    $lapsed['html_file'] = $lapsed_html;
    $lapsed['text_file'] = $lapsed_text;
    $lapsed['subject'] = $lapsed_subject;
    read_files($failed);
    read_files($lapsed);
    $email_files['failed'] = $failed;
    $email_files['lapsed'] = $lapsed;
    return $email_files;
}

function replace($user, $template) {
    $pat = array(
        '/<name\/>/',
        '/<email\/>/',
        '/<create_time\/>/',
        '/<total_credit\/>/',
        '/<opt_out_url\/>/',
        '/<user_id\/>/',
        '/<lapsed_interval\/>/',
    );
    $rep = array(
        $user->name,
        $user->email_addr,
        gmdate('d F Y', $user->create_time),
        number_format($user->total_credit, 0),
        opt_out_url($user),
        $user->id,
        floor ((time() - last_rpc_time($user)) / 86400),
    );
    return preg_replace($pat, $rep, $template);
}

function mail_type($user, $type) {
    global $globals;
    global $email_files;

    $email_file = $email_files[$type];
    if ($email_file['html']) {
        $html = replace($user, $email_file['html']);
    } else {
        $html = null;
    }
    $text = replace($user, $email_file['text']);
    if ($globals->show_email) {
        echo "------- SUBJECT ----------\n";
        echo $email_file['subject'];
        echo "\n------- HTML ----------\n";
        echo $html;
        echo "\n------- TEXT ----------\n";
        echo $text;
    }
    if ($globals->send) {
        echo "sending to $user->email_addr\n";
        echo send_email(
            $user,
            $email_file['subject'],
            $text,
            $html
        );
        $now = time();
        $ntype = 0;
        if ($type == 'lapsed') $ntype = 2;
        if ($type == 'failed') $ntype = 3;
        $query = "insert into sent_email values($user->id, $now, $ntype)";
        _mysql_query($query);
    }
    $globals->count--;
    if ($globals->count == 0) {
        echo "reached limit set by --count - exiting...\n";
        exit();
    }
}

function last_reminder_time($user) {
    $query = "select * from sent_email where userid=$user->id";
    $result = _mysql_query($query);
    $t = 0;
    while ($r = _mysql_fetch_object($result)) {
        if ($r->email_type !=2 && $r->email_type != 3) continue;
        if ($r->time_sent > $t) $t = $r->time_sent;

    }
    _mysql_free_result($result);
    return $t;
}

function handle_user($user, $do_type) {
    global $globals;
    global $email_interval;

    if ($user->send_email == 0) {
        if ($globals->explain) {
            echo "user: $user->id send_email = 0\n";
        }
        return;
    }
    $max_email_time = time() - $globals->email_interval;
    if (last_reminder_time($user) > $max_email_time) {
        if ($globals->explain) {
            echo "user: $user->id sent too recently\n";
        }
        return;
    }
    if ($globals->explain) {
        $x = (time() - $user->create_time)/86400;
        $t = last_rpc_time($user);
        $show_lapsed_interval = (time()-$t)/86400;
        echo "user $user->id ($user->email_addr) was created $x days ago\n";
        echo "  total_credit: $user->total_credit; last RPC $show_lapsed_interval days ago\n";
        echo "  sending $do_type email\n";
    }
    mail_type($user, $do_type);
}

function do_failed() {
    global $globals;

    $max_create_time = time() - $globals->start_interval;
    $result = _mysql_query(
        "select * from user where send_email<>0 and create_time<$max_create_time and total_credit = 0;"
    );
    while ($user = _mysql_fetch_object($result)) {
        handle_user($user, 'failed');
    }
    _mysql_free_result($result);
}

function do_lapsed() {
    global $globals;
    $max_last_rpc_time = time() - $globals->lapsed_interval;

    // the following is an efficient way of getting the list of
    // users for which no host has done an RPC recently
    //
    $result = _mysql_query(
        "select userid from host group by userid having max(rpc_time)<$max_last_rpc_time;"
    );
    while ($host = _mysql_fetch_object($result)) {
        $uresult = _mysql_query("select * from user where id = $host->userid;");
        $user = _mysql_fetch_object($uresult);
        _mysql_free_result($uresult);
        if (!$user) {
            echo "Can't find user $host->userid\n";
            continue;
        }
        handle_user($user, 'lapsed');
    }
    _mysql_free_result($result);
}

if (!function_exists('make_php_mailer')) {
    echo "You must use PHPMailer (http://phpmailer.sourceforge.net)\n";
    exit();
}

$email_files = read_email_files();

if ($globals->userid) {
    $user = BoincUser::lookup_id($globals->userid);
    if (!$user) {
        echo "No such user: $globals->userid\n";
        exit();
    }
    $user->last_rpc_time = last_rpc_time($user);
    mail_type($user, 'failed');
    mail_type($user, 'lapsed');
} else {
    if ($globals->do_failed) {
        do_failed();
    }
    if ($globals->do_lapsed) {
        do_lapsed();
    }
}

?>
