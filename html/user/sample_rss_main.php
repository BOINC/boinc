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
require_once("../project/project.inc");
require_once("../project/project_news.inc");

// Create channel header and open XML content
//
$description = "BOINC project ".PROJECT.": Main page News";
$channel_image = URL_BASE . "/rss_image.jpg";
$create_date  = gmdate('D, d M Y H:i:s') . ' GMT'; 
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
    <title>".PROJECT."</title>
    <link>".URL_BASE."</link>
    <link rel='alternate' type='text/xml' title='".PROJECT." RSS 2.0' href='".URL_BASE."rss.php' />
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

// - Create news items
//
$news = min( count($project_news), $news);
for( $item=0; $item < $news; $item++ ) {
    if( count($project_news[$item]) == 2) {
        echo "<item>
            <title>Project News ".strip_tags($project_news[$item][0])."</title>
            <link>".URL_BASE."</link>
            <description>".strip_tags($project_news[$item][1])."</description>
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
