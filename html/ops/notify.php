#!/usr/bin/env php

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

// Script to delete old notifications and send notification emails.
// Run once a day.
//
// We send emails for notifications generated in the last day.
// This is a slight kludge - since the timing of period tasks
// is not precise, notifications may be delivered twice or not at all.
// We use a 1-hour slop factor to err on the side of twice.
//

$cli_only = true;
require_once("../inc/boinc_db.inc");
require_once("../inc/util_ops.inc");
require_once("../project/project.inc");

// delete notifications older than 90 days
//
function delete_old_notifies() {
    $t = time()-90*86400;
    BoincNotify::delete_aux("create_time < $t");
}

function send_notify_email($userid, $message) {
    $user = BoincUser::lookup_id($userid);
    $subject = "Daily notification summary from ".PROJECT;
    $body = "The following events occurred in the past day at ".PROJECT.".
For details, visit your Account page at
".URL_BASE."home.php

$message
---------------
To change your email preferences for ".PROJECT.", visit:
".URL_BASE."edit_forum_preferences_form.php

Do not reply to this email.
";
    send_email($user, $subject, $body);

    echo "sending to $user->email_addr\n";
}

function send_notify_emails() {
    $db = BoincDb::get();

    $t = time() - (86400 + 3600);  // 1-hour slop factor
    $query = "select notify.* from ".$db->db_name.".notify, ".$db->db_name.".forum_preferences where forum_preferences.pm_notification=2 and notify.userid = forum_preferences.userid and notify.create_time > $t";

    $notifies = BoincNotify::enum_general($query);
    $userid = 0;
    $message = "";
    $i = 1;
    foreach ($notifies as $notify) {
        if ($userid && $notify->userid != $userid) {
            send_notify_email($userid, $message);
            $message = "";
            $i = 1;
        }
        $userid = $notify->userid;
        $message .= "$i) ";
        switch ($notify->type) {
        case NOTIFY_FRIEND_REQ:
            $message .= friend_notify_req_email_line($notify);
            break;
        case NOTIFY_FRIEND_ACCEPT:
            $message .= friend_notify_accept_email_line($notify);
            break;
        case NOTIFY_PM:
            $message .= pm_email_line($notify);
            break;
        case NOTIFY_SUBSCRIBED_POST:
            $message .= subscribed_post_email_line($notify);
            break;
        }
        $message .= "\n";
        $i++;
    }
    if ($userid) {
        send_notify_email($userid, $message);
    }
}

$t = time_str(time());
echo "Starting at $t\n";

delete_old_notifies();
send_notify_emails();

$t = time_str(time());
echo "Ending at $t\n\n";

?>
