<?php
// rss_main.php:
// RSS 2.0 feed for BOINC default server installation.
// Channel Main show the current news on project mainpage 
// - for more informations about RSS see RSS 2.0 Specification:
//   http://blogs.law.harvard.edu/tech/rss

// Check your page with http://feedvalidator.org/                                                                                                                                     

// Create and send out http header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()) . " GMT");
header ("Last-Modified: " . gmdate('D, d M Y H:i:s') . " GMT");
header ("Content-Type: application/xml");

// Get or set display options
// - from 1 to 9 News could be set by option news, default is up to 9
//
$news = "9";
if (isset($_GET["news"])) $news=$_GET["news"];

if($news < "1" or $news > "9") {
    $news = "9";
}

// include project constants and news file
//
require_once("../project/project.inc");
require_once("../project/project_news.inc");

// Create channel header and open XML content
//
$description = "BOINC project ".PROJECT.": Main page News";
$channel_image = URL_BASE . "rss_image.gif";
$create_date  = gmdate('D, d M Y H:i:s') . ' GMT'; 
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
    <title>".PROJECT."</title>
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
$tot = count($project_news);
$news = min( $tot, $news);
for( $item=0; $item < $news; $item++ ) {
    $j = $tot - $item;
    if( count($project_news[$item]) == 2) {
        $d = strtotime($project_news[$item][0]);
        $news_date=gmdate('D, d M Y H:i:s',$d) . ' GMT';
        $unique_url=URL_BASE."all_news.php#$j";
        echo "<item>
            <title>Project News ".strip_tags($project_news[$item][0])."</title>
            <link>$unique_url</link>
            <guid isPermaLink=\"true\">$unique_url</guid>
            <description>".strip_tags($project_news[$item][1])."</description>
            <pubDate>$news_date</pubDate>
            </item>
        ";
    }
}

// Close XML content
//
echo "
    </channel>
    </rss>
";

?>
