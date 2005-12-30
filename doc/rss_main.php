<?php
// rss_main.php:
// RSS 2.0 feed for BOINC default server installation.
// Channel Main show the current news on project mainpage 
// - for more informations about RSS see RSS 2.0 Specification:
//   http://blogs.law.harvard.edu/tech/rss

// Create and send out http header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()) . " GMT");
header ("Last-Modified: " . gmdate('D, d M Y H:i:s') . " GMT");
header ("Content-Type: text/xml");

// Get or set display options
// - from 1 to 9 News could be set by option news, default is up to 9
//
$news = $_GET["news"];
if (!$news) { 
    $news = "9";
} else {
    if($news < "1" or $news > "9") {
        $news = "9";
    }
}

// inclue project constants and news file
//
require_once("boinc_news.inc");

// Create channel header and open XML content
//
$description = "BOINC project ".PROJECT.": Main page News";
$channel_image = "http://boinc.berkeley.edu/boinc.gif";
$create_date  = gmdate('D, d M Y H:i:s') . ' GMT'; 
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
        <title>BOINC</title>
        <link>http://boinc.berkeley.edu</link>
        <description>Berkeley Open Infrastructure for Network Computing</description>
        <copyright>U.C. Berkeley</copyright>
        <lastBuildDate>$create_date</lastBuildDate>
        <language>$language</language>
        <image>
            <url>$channel_image</url>
            <title>BOINC</title>
            <link>http://boinc.berkeley.edu</link>
        </image>
";

// - Create news items
//
$news = min( count($project_news), $news);
for( $item=0; $item < $news; $item++ ) {
    if( count($project_news[$item]) == 2) {
        $d = strtotime($project_news[$item][0]);
        $d = strftime("%a, %d %b %Y", $d);
        echo "<item>
            <title>Project News ".strip_tags($project_news[$item][0])."</title>
            <link>http://boinc.berkeley.edu/old_news.php#$item</link>
            <guid>$item</guid>
            <description>".strip_tags($project_news[$item][1])."</description>
            <pubDate>$d</pubDate>
            </item>
        ";
    }
}

// Close XML content
//
echo "</channel>\n</rss>";

?>
