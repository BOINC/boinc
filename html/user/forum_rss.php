<?php

// get a forum as RSS feed

require_once("../project/project.inc");
require_once("../inc/db.inc");
db_init();

$userid = get_int('userid', true);
$forumid = get_int('forumid', true);
$nitems = get_int('nitems', true);

if(!$nitems || $nitems < "1" or $nitems > "9") {
    $nitems = "9";
}

if ($userid) {
    $user = BoincUser.lookup_id($user);
    if (!$user) error_page("no such user");
}

$last_mod_time = BoincThread.get_int("max(timestamp)",
    "where owner=$userid and forum=$forumid and status=0 and hidden=0 and sticky=0"
);
//$query="select max(timestamp) from thread where owner=".$userid." and forum=".$forumid." and status=0 and hidden=0 and sticky=0";
//$result=mysql_query($query);
if ($result) {
  $last_mod_time=mysql_fetch_object($result);
} else {
  echo "error in ".$query."
      ";
  exit(1);
}

$threads = BoincThread::enum_fields("id,create_time,title", "where owner=$userid and forum=$forumid and status=0 and hidden=0 and sticky=0 order by create_time desc limit $news"
);
//$query="select id,create_time,title from thread where owner=".$userid." and forum=".$forumid." and status=0 and hidden=0 and sticky=0 order by create_time desc limit ".$news;

//$result=mysql_query($query);
if (!$result) {
  echo "error in ".$query."
      ";
  exit(1);
}

// Get unix time that last modification was made to the news source
//
$create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 

// Now construct header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()+86400) . " GMT");
header ("Last-Modified: " . $create_date);
header ("Content-Type: application/xml");


// Create channel header and open XML content
//
$description=$username->name."'s ".PROJECT." Blog";
$channel_image = URL_BASE . "rss_image.gif";
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
    <title>".$description."</title>
    <link>".URL_BASE."</link>
    <description>".$description."</description>
    <copyright>".COPYRIGHT_HOLDER."</copyright>
    <lastBuildDate>".$create_date."</lastBuildDate>
    <language>".$language."</language>
    <image>
        <url>".$channel_image."</url>
        <title>".PROJECT."</title>
        <link>".URL_BASE."</link>
    </image>
";

// write news items
//
foreach ($threads as $thread) {
    $post_date=gmdate('D, d M Y H:i:s',$thread->create_time).' GMT';
    $unique_url=URL_BASE."forum_thread.php?id=".$thread->id;

    $posts = BoincPost::enum("thread=$thread->id and user=$userid order by timestamp limit 1");
    $post = $posts[0];
    echo "<item>
    <title>".strip_tags($thread->title)."</title>
    <link>$unique_url</link>
    <guid isPermaLink=\"true\">$unique_url</guid>
    <description>".substr(strip_tags($post->content),0,255)." . . .</description>
    <pubDate>$post_date</pubDate>
</item>
";
}

// Close XML content
//
echo "
    </channel>
    </rss>
";

?>
