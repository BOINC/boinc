<?php

// send "reminder" emails to
// - failed users: those who
//   1) were created at least Sstart_interval seconds ago,
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

$start_interval = 14*86400;
$email_interval = 30*86400;
$lapsed_interval = 60*86400;

// set the following to false to actually send emails
// (rather than just print to stdout)
//
$testing = true;

// If the following is nonzero, email will be sent to the given user ID
// Otherwise it will be sent to all users
//
$userid = 1;

require_once('../project/project.inc');
require_once("../inc/db.inc");
require_once("../inc/email.inc");

db_init();
set_time_limit(0);

// File names for the various mail types.
// Change these here if needed.
$dir = "remind_email";
$failed_html = "$dir/failed_html";
$failed_text = "$dir/failed_text";
$failed_subject = "$dir/failed_subject";
$lapsed_html = "$dir/lapsed_html";
$lapsed_text = "$dir/lapsed_text";
$lapsed_subject = "$dir/lapsed_subject";

// return time of last scheduler RPC from this user,
// or zero if they're never done one
//
function last_rpc_time($user) {
    $x = 0;
    $result = mysql_query("select * from host where userid=$user->id");
    while ($host = mysql_fetch_object($result)) {
        if ($host->rpc_time > $x) $x = $host->rpc_time;
    }
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
    if ($email_file['html']) {
        $html = replace($user, $email_file['html']);
    } else {
        $html = null;
    }
    $text = replace($user, $email_file['text']);
    if ($testing) {
        if (true) {
            echo "------- SUBJECT ----------\n";
            echo $email_file['subject'];
            echo "\n------- HTML ----------\n";
            echo $html;
            echo "\n------- TEXT ----------\n";
            echo $text;
        }
    } else {
        send_email(
            $user,
            $email_file['subject'],
            $text,
            $html
        );
        $now = time();
        mysql_query("update user set posts=$now where id=$user->id");
    }
}

// NOTE: we're using user.posts (a deprecated field)
// to store the time the user was last sent an email.
// At some point maybe we'll rename this field.

function handle_user($user, $email_files) {
    global $lapsed_interval;
    global $testing;
    if ($testing) {
        $x = (time() - $user->create_time)/86400;
        echo "user $user->email_addr was created $x days ago\n";
    }
    if ($user->total_credit == 0) {
        if ($testing) {
            echo "zero credit, sending failed email\n";
        }
        mail_type($user, $email_files['failed']);
    } else {
        $t = last_rpc_time($user);
        if ($t < time() - $lapsed_interval) {
            if ($testing) {
                $x = (time()-$t)/86400;
                echo "nonzero credit, last RPC $x days ago, sending lapsed email\n";
            }
            mail_type($user, $email_files['lapsed']);
        }
    }
}

function do_batch($email_files, $startid, $n) {
    global $email_interval;
    global $start_interval;

    $max_email_time = time() - $email_interval;
    $max_create_time = time() - $start_interval;
    $result = mysql_query(
        "select * from user where id>$startid and send_email<>0 and posts<$max_email_time and create_time<$max_create_time and expavg_credit < 10 order by id limit $n"
    );
    while ($user = mysql_fetch_object($result)) {
        handle_user($user, $email_files);
        $startid = $user->id;
    }
    mysql_free_result($result);
    return $startid;
}

function main($email_files) {
    $startid = 0;
    $n = 1000;
    while (1) {
        $new_startid = do_batch($email_files, $startid, $n, $f);
        if ($new_startid == $startid) break;
        $startid = $new_startid;
    }
    echo "All done!\n";
}

if (!$USE_PHPMAILER) {
    echo "You must use PHPMailer.\n";
    exit();
}

$email_files = read_email_files();

if ($userid) {
    $user = lookup_user_id($userid);
    if (!$user) {
        echo "No such user: $userid\n";
        exit();
    }
    mail_type($user, $email_files['failed']);
    mail_type($user, $email_files['lapsed']);
} else {
    main($email_files);
}

?>
