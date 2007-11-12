<?php

require_once("../inc/forum_db.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$method = get_str("method", true);
if ($method != "user_posts" && $method != "user_threads") {
    xml_error(-210);
}

$userid = get_int("userid", true);
$user = lookup_user_id($userid);
if (!$user) {
    xml_error(-136);
}

if ($method == "user_posts") {
    $count = get_int("count", true);
    if (!$count || $count <= 0 || $count > 50) { $count = 10; }
    $length = get_int("contentlength", true);
    if (($length == null) || ($length <= 0)) { $length = 0; }
    $posts = BoincPost::enum("user=$userid ORDER BY timestamp DESC LIMIT $count");
    $count = count($posts);
    echo "<rpc_response>\n";
    echo "<count>$count</count>\n";
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
