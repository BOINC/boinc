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

require_once("../project/project.inc");

function notify_rss_auth($user) {
    return md5($user->authenticator.$user->passwd_hash."notify_rss");
}

function notify_rss_url($user) {
    return URL_BASE."notify_rss.php?userid=$user->id&auth=".notify_rss_auth($user);
}

function show_notify_rss_item($notify) {
    switch ($notify->type) {
    case NOTIFY_FRIEND_REQ:
        friend_req_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_FRIEND_ACCEPT:
        friend_accept_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_PM:
        pm_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_SUBSCRIBED_POST:
        subscribe_rss($notify, $title, $msg, $url);
        break;
    }

    $news_date=gmdate('D, d M Y H:i:s',$notify->create_time) . ' GMT';
    echo "<item>
        <title>$title</title>
        <guid>".URL_BASE."_notify_$notify->id</guid>
        <link>".htmlentities($url)."</link>
        <description><![CDATA[$msg]]></description>
        <pubDate>$news_date</pubDate>
        </item>
    ";
}

?>
