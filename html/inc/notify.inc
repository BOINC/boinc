<?php

require_once("../project/project.inc");

function notify_rss_auth($user) {
    return md5($user->authenticator.$user->passwd_hash."notify_rss");
}

function notify_rss_url($user) {
    return URL_BASE."notify_rss.php?userid=$user->id&auth=".notify_rss_auth($user);
}

?>
