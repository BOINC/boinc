<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$method = get_str("method", true);
if ($method != "user_posts" && $method != "user_threads") { xml_error(-210); }

if ($method == "user_posts") {
    $userid = get_int("userid", true);
    $user = lookup_user_id($userid);
    if (!$user) { xml_error(-136); }
    
    $count = get_int("count", true);
    if (!$count || $count <= 0 || $count > 50) { $count = 10; }
    $length = get_int("contentlength", true);
    if (($length == null) || ($length <= 0)) { $length = 0; }
    $res = mysql_query("SELECT * FROM post WHERE user=$userid ORDER BY timestamp DESC LIMIT $count");
    if ($res) {
        $count = mysql_num_rows($res);
        
        echo "<rpc_response>\n";
        echo "<count>$count</count>\n";
        echo "<posts>\n";
        
        while ($row = mysql_fetch_object($res)) {
            $thread = mysql_query("SELECT * FROM thread WHERE id=".$row->thread);
            $thread = mysql_fetch_object($thread);
            echo "<post>\n";
            echo "    <id>$row->id</id>\n";
            echo "    <threadid>$row->thread</threadid>\n";
            echo "    <threadtitle><![CDATA[".$thread->title."]]></threadtitle>\n";
            echo "    <timestamp>$row->timestamp</timestamp>\n";
            if ($length > 0) {
                echo "    <content><![CDATA[".substr($row->content, 0, $length)."]]></content>\n";
            } else {
                echo "    <content><![CDATA[".$row->content."]]></content>\n";
            }
            echo "</post>\n";
        }
        
        echo "</posts>\n";
        echo "</rpc_response>\n";
    } else {
        xml_error(-1, "Database error");
    }
} elseif ($method == "user_threads") {

    $userid = get_int("userid", true);
    $user = lookup_user_id($userid);
    if (!$user) { xml_error(-136); }
    
    $count = get_int("count", true);
    if (!$count || $count <= 0 || $count > 50) { $count = 10; }
    $res = mysql_query("SELECT * FROM thread WHERE owner=$userid ORDER BY timestamp DESC LIMIT $count");
    if ($res) {
        $count = mysql_num_rows($res);
	
        echo "<rpc_response>\n";
        echo "<count>$count</count>\n";
        echo "<threads>\n";
        while ($row = mysql_fetch_object($res)) {
            echo "<thread>\n";
            echo "    <id>$row->id</id>\n";
            echo "    <forumid>$row->forum</forumid>\n";
            echo "    <replies>$row->replies</replies>\n";
            echo "    <views>$row->views</views>\n";
            echo "    <timestamp>$row->timestamp</timestamp>\n";
            echo "    <title><![CDATA[$row->title]]></title>\n";
            echo "</thread>\n";
        }
        
        echo "</threads>\n";
        echo "</rpc_response>\n";
    } else {
        xml_error(-1, "Database error");
    }
}

?>
