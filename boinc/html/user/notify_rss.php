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

// RSS feed for per-user notifications

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/pm.inc");
require_once("../inc/friend.inc");
require_once("../inc/notify.inc");
require_once("../project/project.inc");

$userid = get_int('userid');
$auth = get_str('auth');

$user = BoincUser::lookup_id($userid);
if (!$user) xml_error();
if (notify_rss_auth($user) != $auth) xml_error();

$notifies = BoincNotify::enum("userid = $userid order by create_time desc");

if (count($notifies)) {
    $last_mod_time = $notifies[0]->create_time;
} else {
    $last_mod_time = time();
}
$create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 

header("Expires: ".gmdate('D, d M Y H:i:s', time())." GMT");
header("Last-Modified: ".$create_date);
header("Content-Type: application/xml");

$description = "Community notifications";
$channel_image = URL_BASE."rss_image.gif";
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">
    <channel>
    <title>".PROJECT."</title>
    <link>".URL_BASE."</link>
    <atom:link href=\"".URL_BASE."notify_rss.php\" rel=\"self\" type=\"application/rss+xml\" />
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

foreach ($notifies as $notify) {
    show_notify_rss_item($notify);
}

echo "
    </channel>
    </rss>
";

?>
