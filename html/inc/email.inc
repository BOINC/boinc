<?php

require_once("../inc/util.inc");
require_once("../project/project.inc");

if (defined($USE_PHPMAILER)) {
    if ($USE_PHPMAILER) {
        require_once("../inc/phpmailer/class.phpmailer.php");
    }
} else {
    $USE_PHPMAILER = false;
}

function send_email($user, $subject, $body) {
    global $USE_PHPMAILER;
    global $PHPMAILER_HOST;
    global $PHPMAILER_MAILER;

    if ($USE_PHPMAILER) {
        $mail = new PHPMailer();
        $mail->AddAddress($user->email_addr, $user->name);
        $mail->Subject = $subject;
        $mail->Body = $body;
        $mail->From = EMAIL_FROM;
        $mail->FromName = EMAIL_FROM;
        $mail->Host = $PHPMAILER_HOST;
        $mail->Mailer = $PHPMAILER_MAILER;
        return $mail->Send();
    } else {
        if (defined('EMAIL_FROM')) {
            $headers = "From: ". EMAIL_FROM;
        } else {
            $headers ="";
        }
        return mail($user->email_addr, $subject, $body, $headers);
    }
}
// Send an email describing an account to the user.
// There are a few scenarios:
//
// 1) the account was created by user via web.
//    In this case they're currently looking at the "validate account" page
//    (account_created.php), although they might have strayed
//    so we need to give them a link.
// 2) the account was created administratively
// 3) the user requested account key for existing account
//
function send_auth_email($user, $is_new) {
    $body = "";

    if ($is_new) {
        $subject = PROJECT." account confirmation for $user->name";
        $body = "Welcome to ".PROJECT.".

This email confirms your account with ".PROJECT.":
    Project URL:         ".MASTER_URL."
    Account Key:         $user->authenticator

    User name:           $user->name
    E-mail:              $user->email_addr
";
    } else {
        $subject = PROJECT." information for $user->name";
        $body = "Your ".PROJECT." account information is:
    Project URL:         ".MASTER_URL."
    Account Key:         $user->authenticator

    User name:           $user->name
    E-mail:              $user->email_addr
";
    }

$body .= "

For further information and assistance with ".PROJECT." go to ".MASTER_URL."
";
    return send_email($user, $subject, $body);
}

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    echo "
        Your Account Key has been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes,
        your ISP may be blocking it as spam.
        In this case please contact your ISP and
        ask them to not block email from $email_from.
    ";
}

// a valid email address is of the form A@B.C
// where A, B, C are nonempty,
// A and B don't contain @ or .,
// and C doesn't contain @
//
function is_valid_email_addr($addr) {
    $x = strstr($addr, "@");
    if (!$x) return false;
    if (strlen($x) == strlen($addr)) return false;
    $x = substr($x, 1);
    if (strstr($x, "@")) return false;
    $y = strstr($x, ".");
    if (!$y) return false;
    if (strlen($y) == strlen($x)) return false;
    if (strlen($y) == 1) return false;
    return true;
}

//////////////////////////////////////////////

function send_moderation_email($user, $thread,  $post, $message) {
    $body = "";

    $subject = PROJECT." forum moderation notice for $user->name";
    $body = PROJECT." notification:

This email is sent to inform you that one of your posts in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title."
    Link:           ".MASTER_URL."/forum_thread.php?id=".$thread->id."
    
The moderator gave this explanation to why your post was moderated:
".$message;
$body .= "

This was the contents of your post:
".$post->content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    return send_email($user, $subject, $body);

}

function send_thread_moderation_email($user, $thread, $message) {
    $body = "";

    $subject = PROJECT." forum moderation notice for $user->name";
    $body = PROJECT." notification:

This email is sent to inform you that one of your threads in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title."
    Link:           ".MASTER_URL."/forum_thread.php?id=".$thread->id."
    
The moderator gave this explanation to why your thread was moderated:
".$message;
$body .= "

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    return send_email($user, $subject, $body);

}

function send_problem_email($user, $host) {
    $body = "";
	
    $host_content = "ID: ".$host->id."
    Created: ".time_str($host->create_time)."
    Venue: ".$host->venue."
    Total credit: ".$host->total_credit."
    Average credit: ".$host->expavg_credit."
    Average update time: ".time_str($host->expavg_time)."
    IP address: $host->last_ip_addr (same the last $host->nsame_ip_addr times)
    Domain name: " . $host->domain_name;
    $x = $host->timezone/3600;
    if ($x >= 0) $x="+$x";
    $host_content.="
    Local Time = UTC $x hours
    Number of CPUs: " . $host->p_ncpus."
    CPU: $host->p_vendor $host->p_model
    FP ops/sec: ".$host->p_fpops."
    Int ops/sec: ".$host->p_iops."
    memory bandwidth: ".$host->p_membw."
    Operating System: $host->os_name $host->os_version";
    $x = $host->m_nbytes/(1024*1024);
    $y = round($x, 2);
    $host_content.="
    Memory: $y MB";
    $x = $host->m_cache/1024;
    $y = round($x, 2);
    $host_content.="
    Cache: $y KB";
    $x = $host->m_swap/(1024*1024);
    $y = round($x, 2);
    $host_content.="
    Swap Space: $y MB";
    $x = $host->d_total/(1024*1024*1024);
    $y = round($x, 2);
    $host_content.="
    Total Disk Space: $y GB";
    $x = $host->d_free/(1024*1024*1024);
    $y = round($x, 2);
    $host_content.="
    Free Disk Space: $y GB
    Avg network bandwidth (upstream): $host->n_bwup bytes/sec
    Avg network bandwidth (downstream): $host->n_bwdown bytes/sec";
    $x = $host->avg_turnaround/86400;
    $host_content.="
    Average turnaround: ".round($x, 2)." days
    Number of RPCs: $host->rpc_seqno
    Last RPC: ".time_str($host->rpc_time)."
    % of time client on: ". 100*$host->on_frac." %
    % of time host connected: " . 100*$host->connected_frac." %
    % of time user active: " . 100*$host->active_frac." %
    # of results today: " . $host->nresults_today;
    
    $subject = PROJECT." notice for $user->name";
    $body = PROJECT." notification:

Dear $user->name
Your machine (host # $host->id) described below appears to have a misconfigured BOINC
installation.  Could you please have a look at it?

Sincerely,
	The ". PROJECT." team
";
$body .= "

This is the content of our database:
" . $host_content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    echo nl2br($body) . "<br><br>"; 
    return send_email($user, $subject, $body);

}

function send_report_post_email($user, $thread,  $post, $message) {
    global $forum_post_reporting_admin;
    
    $body = "";

    $subject = PROJECT." post in '".$thread->title."' reported as offensive";
    $body = PROJECT." notification:

This email is sent to inform you that one of the posts in the forum was reported as offensive in ".PROJECT.":
    Thread:         ".$thread->title."
    Post:           ".$post->id."
    Link:           ".MASTER_URL."/forum_thread.php?id=".$thread->id."#".$post->id."
    
The reporting user gave this explanation to why the post was reported:
".$message."

This was the contents of the post:
".$post->content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    //Security check, do we have an admin?
    if (!$forum_post_reporting_admin) error_page("This project has not yet defined an administrator to handle this kind of forum reports. Please contact the project and tell them to add this information in their html/project/project.inc file");
    return send_email($forum_post_reporting_admin, $subject, $body);
}


?>
