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


// rss_main.php:
// RSS 2.0 feed for BOINC default server installation.
// Channel Main show the current news on project mainpage 
// - for more informations about RSS see RSS 2.0 Specification:
//   http://blogs.law.harvard.edu/tech/rss

// Check your page with http://feedvalidator.org/

// Get unix time that last modification was made to the news source
//
$last_mod_time=filemtime("../project/project_news.inc");
$create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 

// Now construct header
//
header ("Expires: " . gmdate('D, d M Y H:i:s', time()) . " GMT");
header ("Last-Modified: " . $create_date);
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
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");
require_once("../project/project_news.inc");

// Create channel header and open XML content
//
$description = "BOINC project ".PROJECT.": Main page News";
$channel_image = URL_BASE . "rss_image.gif";
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
$news = min($tot, $news);
for ($i=0; $i < $news; $i++) {
    $j = $tot - $i;
    $item = $project_news[$i];
    if (count($item) < 2) continue;
    $d = strtotime($item[0]);
    $news_date=gmdate('D, d M Y H:i:s',$d) . ' GMT';
    $unique_url=URL_BASE."all_news.php#$j";
    if (isset($item[2])) {
        $title = $item[2];
    } else {
        $title = "Project News ".$item[0];
    }
    $body = image_as_link($item[1]);
    echo "<item>
        <title>".$title."</title>
        <link>$unique_url</link>
        <guid isPermaLink=\"true\">$unique_url</guid>
        <description><![CDATA[$body]]></description>
        <pubDate>$news_date</pubDate>
    ";
    if (isset($item[3])) {
        $category = $item[3];
        echo "
            <category>$category</category>
        ";
    }
    echo "
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
