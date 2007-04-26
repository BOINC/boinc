#! /usr/bin/env php

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
$testing = false;

// If the following is nonzero, email will be sent to the given user ID
// Otherwise it will be sent to all users
//
$userid = 0;

// If set to 1, don't bother with "current" users - only mail failed/lapsed users
//
$nocurrent = 1;

// name of input id file - if empty, just process ids in batches that are greater
// than the last id in output id file. Note: ids MUST be in increasing order.
//
$id_file = "./id_file";

// name of output id file (for checkpointing) - must be created before starting
//
$mass_email_log = './email_log';

// File names for the various mail types.
// Change these here if you like.

$email_failed_html = 'remind_email/sample_failed_html';
$email_failed_text = 'remind_email/sample_failed_text';
$email_failed_subject = 'remind_email/sample_failed_subject';
$email_lapsed_html = 'remind_email/sample_lapsed_html';
$email_lapsed_text = 'remind_email/sample_lapsed_text';
$email_lapsed_subject = 'remind_email/sample_lapsed_subject';
$email_current_html = 'remind_email/sample_current_html';
$email_current_text = 'remind_email/sample_current_text';
$email_current_subject = 'remind_email/sample_current_subject';

//////////////////////////////////////////////////////////////////////////

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
    global $nocurrent;

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
    if ($nocurrent == 0) { read_files($current); }
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
        '/<lapsed_interval\/>/',
    );
    $most_recent = 0; 
    $result = mysql_query("select * from host where userid=" . $user->id);
    while ($host = mysql_fetch_object($result)) {
      if ($host->rpc_time > $most_recent) { $most_recent = $host->rpc_time; }
    }
    mysql_free_result($result);
    $rep = array(
        $user->name,
        gmdate('d F Y', $user->create_time),
        number_format($user->total_credit, 0),
        URL_BASE."opt_out.php?code=".salted_key($user->authenticator)."&userid=$user->id",
        floor((time() - $most_recent) / 86400),
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
        echo $user->id . ": ";
        if (is_valid_email_addr($user->email_addr)) {
          send_email(
              $user,
              $email_file['subject'],
              $text,
              $html
              );
          echo "sent\n";
        } else {
          echo "invalid e-mail address\n";
        }
    }
}

function handle_user($user, $email_files) {
    global $nocurrent;
    if ($user->total_credit == 0) {
        mail_type($user, $email_files['failed']);
    } else if ($user->expavg_credit < 1000) {
        mail_type($user, $email_files['lapsed']);
    } else {
        if ($nocurrent == 0) {
          mail_type($user, $email_files['current']);
        } else {
          echo $user-> id . ": not doing current/active users\n";
        }
    }
}

function do_batch($email_files, $startid, $n, $log) {
    $result = mysql_query(
        "select * from user where id>$startid and id < 122080 order by id limit $n"
    );
    while ($user = mysql_fetch_object($result)) {
        handle_user($user, $email_files);
        $startid = $user->id;
        fputs($log, $user->id . "\n");
        fflush($log);
    }
    mysql_free_result($result);
    return $startid;
}

function do_one($email_files, $thisid, $log) {
    $result = mysql_query(
        "select * from user where id=$thisid"
    );
    while ($user = mysql_fetch_object($result)) {
        handle_user($user, $email_files);
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
        echo $mass_email_log . ' not found - create empty file and run again\n';
        exit();
    }
    $startid = 0;
    while (fscanf($f, '%d', &$startid)) {
    }
    fclose($f);
    return $startid;
}

function main($email_files) {
    global $id_file;
    global $mass_email_log;
    $startid = read_log();
    $f = fopen($mass_email_log, 'a');
    $n = 10;
    if ($id_file == "") {
        while (1) {
            $new_startid = do_batch($email_files, $startid, $n, $f);
            if ($new_startid == $startid) break;
            $startid = $new_startid;
        }
    } else {
        $fid = fopen($id_file, 'r'); 
        if (!$fid) {
            echo  $id_file . ' not found - create id list and run again\n';
            exit();
        }
        $thisid = 0;
        while (fscanf($fid, '%d', &$thisid)) {
            if ($thisid > $startid) {
                do_one($email_files, $thisid, $f);
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

if ($userid) {
    $user = lookup_user_id($userid);
    mail_type($user, $email_files['failed']);
    mail_type($user, $email_files['lapsed']);
    if ($nocurrent == 0) { mail_type($user, $email_files['current']); }
} else {
    main($email_files);
}

?>
