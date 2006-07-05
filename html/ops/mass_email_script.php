<?php

// mass email program
//
// see http://boinc.berkeley.edu/mass_email.php for info

require_once('../project/project.inc');
require_once('../inc/email.inc');
require_once('../inc/db.inc');

db_init();
set_time_limit(0);

// set the following to false to actually send emails
// (rather than just print to stdout)
//
$testing = true;

// If the following is nonzero, email will be sent to the given user ID
// Otherwise it will be sent to all users
//
$userid = 1;

// File names for the various mail types.
// Change these here if you like.

$email_failed_html = 'email_failed_html';
$email_failed_text = 'email_failed_text';
$email_failed_subject = 'email_failed_subject';
$email_lapsed_html = 'email_lapsed_html';
$email_lapsed_text = 'email_lapsed_text';
$email_lapsed_subject = 'email_lapsed_subject';
$email_current_html = 'email_current_html';
$email_current_text = 'email_current_text';
$email_current_subject = 'email_current_subject';

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
    read_files($current);
    $email_files['failed'] = $failed;
    $email_files['lapsed'] = $lapsed;
    $email_files['current'] = $current;
    return $email_files;
}

function replace($user, $template) {
    $pat = array(
        '/<name\/>/',
        '/<create_time\/>/',
        '/<total_credit\/>/',
        '/<opt_out_url\/>/',
    );
    $rep = array(
        $user->name,
        gmdate('d F Y', $user->create_time),
        number_format($user->total_credit, 0),
        URL_BASE."opt_out.php?code=".salted_key($user->authenticator)."&userid=$user->id",
    );
    return preg_replace($pat, $rep, $template);
}

function mail_type($user, $email_file) {
    global $testing;
    $html = replace($user, $email_file['html']);
    $text = replace($user, $email_file['text']);
    if ($testing) {
        echo "\nSending to $user->email_addr:\n";
        echo "------- SUBJECT ----------\n";
        echo $email_file['subject'];
        echo "\n------- HTML ----------\n";
        echo $html;
        echo "\n------- TEXT ----------\n";
        echo $text;
    } else {
        send_email(
            $user,
            $email_file['subject'],
            $text,
            $html
        );
    }
}

function handle_user($user, $email_files) {
    if ($user->total_credit == 0) {
        mail_type($user, $email_files['failed']);
    } else if ($user->expavg_credit < 1) {
        mail_type($user, $email_files['lapsed']);
    } else {
        mail_type($user, $email_files['current']);
    }
}

function do_batch($email_files, $startid, $n, $log) {
    $result = mysql_query(
        "select * from user where id>$startid order by id limit $n"
    );
    while ($user = mysql_fetch_object($result)) {
        handle_user($user, $email_files);
        $startid = $user->id;
        fputs($log, '$user->id\n');
        fflush($log);
    }
    mysql_free_result($result);
    return $startid;
}

function read_log() {
    $f = fopen('mass_email.log', 'r');
    if (!$f) {
        echo 'mass_email.log not found - create empty file and run again\n';
        exit();
    }
    $startid = 0;
    while (fscanf($f, '%d', &$startid)) {
    }
    fclose($f);
    return $startid;
}

function main($email_files) {
    $startid = read_log();
    $f = fopen('mass_email.log', 'w');
    $n = 1000;
    while (1) {
        $new_startid = do_batch($email_files, $startid, $n, $f);
        if ($new_startid == $startid) break;
        $startid = $new_startid;
    }
    echo 'All done!\n';
}

function test($email_files) {
}

if (!$USE_PHPMAILER) {
    echo "You must use PHPMailer.\n";
    exit();
}

$email_files = read_email_files();

if ($userid) {
    $user = lookup_user_id($userid);
    mail_type($user, $email_files['failed']);
    mail_type($user, $email_files['lapsed']);
    mail_type($user, $email_files['current']);
} else {
    main($email_files);
}

?>
