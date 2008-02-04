<?php

// send reminder emails for Bolt refresh.
// Run this script at most once a day
// (to avoid multiple emails)

require_once("../inc/bolt_db.inc");

function notify_user($user) {
    $body = "
You are due for refresh on the following units from 
";
    foreach($user->refresh as $r) {
        echo "
    }
}

function notify_users() {
    $now = time();
    $rs = BoltRefresh::enum("due_time < $now");
    $users = array();

    foreach($rs as $r) {
        $view = BoltView::lookup_id($r->view_id);
        $user_id = $view->user_id;
        if (!key_exists($user_id)) {
            $user = BoincUser::lookup_id($user_id);
            BoltUser::lookup($user);
            $user->refresh = array();
            $users[$user_id] = $user;
        }
        $users[$user_id]->refresh[] = $r;
    }

    foreach ($users as $user) {
        notify_user($user);
    }
}

?>
