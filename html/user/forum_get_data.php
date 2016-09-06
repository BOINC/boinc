<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

require_once("../inc/forum_db.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");

BoincDb::get(true);
xml_header();

if (DISABLE_FORUMS) {
    xml_error(-1, "Forums are disabled");
}

$retval = db_init_xml();
if ($retval) xml_error($retval);

$method = get_str("method", true);
if ($method != "user_posts" && $method != "user_threads") {
    xml_error(-1);
}

$userid = get_int("userid", true);
$user = BoincUser::lookup_id($userid);
if (!$user) {
    xml_error(ERR_DB_NOT_FOUND);
}

if ($method == "user_posts") {
    $count = get_int("count", true);
    if (!$count || $count <= 0 || $count > 50) {
        $count = 10;
    }
    $length = get_int("contentlength", true);
    if (($length == null) || ($length <= 0)) {
        $length = 0;
    }
    $posts = BoincPost::enum("user=$userid ORDER BY timestamp DESC LIMIT $count");
    $realcount = BoincPost::count("user=$userid");
    echo "<rpc_response>\n";
    echo "<count>$realcount</count>\n";
    echo "<posts>\n";

    foreach ($posts as $post) {
        $thread = BoincThread::lookup_id($post->thread);
        echo "<post>\n";
        echo "    <id>$post->id</id>\n";
        echo "    <threadid>$post->thread</threadid>\n";
        echo "    <threadtitle><![CDATA[".$thread->title."]]></threadtitle>\n";
        echo "    <timestamp>$post->timestamp</timestamp>\n";
        if ($length > 0) {
            echo "    <content><![CDATA[".substr($post->content, 0, $length)."]]></content>\n";
        } else {
            echo "    <content><![CDATA[".$post->content."]]></content>\n";
        }
        echo "</post>\n";
    }

    echo "</posts>\n";
    echo "</rpc_response>\n";
} elseif ($method == "user_threads") {
    $count = get_int("count", true);
    if (!$count || $count <= 0 || $count > 50) { $count = 10; }
    $threads = BoincThread::enum("owner=$userid ORDER BY timestamp DESC LIMIT $count");
    $count = count($threads);

    echo "<rpc_response>\n";
    echo "<count>$count</count>\n";
    echo "<threads>\n";
    foreach($threads as $thread) {
        echo "<thread>\n";
        echo "    <id>$thread->id</id>\n";
        echo "    <forumid>$thread->forum</forumid>\n";
        echo "    <replies>$thread->replies</replies>\n";
        echo "    <views>$thread->views</views>\n";
        echo "    <timestamp>$thread->timestamp</timestamp>\n";
        echo "    <title><![CDATA[$thread->title]]></title>\n";
        echo "</thread>\n";
    }

    echo "</threads>\n";
    echo "</rpc_response>\n";
}

?>
