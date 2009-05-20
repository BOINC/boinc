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

function image_as_link($text){
    // Build some regex (should be a *lot* faster)
    $pattern = '@<img src=([^>]+)>@si'; // Gives us the URL in ${1}...
    $replacement = '<a href=${1}>[Image Link]</a>'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    return $text;
}

require_once("boinc_news.php");

// Create channel header and open XML content
//
$description = "BOINC project ".PROJECT.": Main page News";
$channel_image = "http://boinc.berkeley.edu/www_logo.gif";
$create_date  = gmdate('D, d M Y H:i:s') . ' GMT'; 
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
        <title>BOINC</title>
        <link>http://boinc.berkeley.edu</link>
        <description>
            News and announcements related to
            BOINC (Berkeley Open Infrastructure for Network Computing),
            an open-source platform for volunteer and grid commputing.
        </description>
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
$tot = count($project_news);
$news = min( $tot, $news);
for( $item=0; $item < $news; $item++ ) {
    $j = $tot - $item;
    if( count($project_news[$item]) == 2) {
        $d = strtotime($project_news[$item][0]);
        $news_date=gmdate('D, d M Y H:i:s',$d) . ' GMT';
        $unique_url="http://boinc.berkeley.edu/all_news.php#$j";
        $title = "BOINC news ". $project_news[$item][0];
        $body = image_as_link($project_news[$item][1]);
        echo "<item>
            <title>$title</title>
            <link>http://boinc.berkeley.edu/all_news.php#$j</link>
            <guid isPermaLink=\"true\">$unique_url</guid>
            <description><![CDATA[$body]]></description>
            <pubDate>$news_date</pubDate>
            </item>
        ";
    }
}

// Close XML content
//
echo "</channel>\n</rss>";

?>
