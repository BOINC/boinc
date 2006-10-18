<?php
require_once("docutil.php");

$strip_header = $_GET['strip_header'];

if (!$strip_header) {
page_head('BOINC add-on software');
echo "
<p>
A number of programs that complement or enhance BOINC are available.
Note that:
<ul>
<li>

These applications are not endorsed by BOINC and
you use them at your own risk.
<li>
We do not provide instructions for installing these applications.
However, the author may have provided some help on installing or
uninstalling the application.
If this is not enough you should contact the author.
Instructions for installing and running BOINC are
<a href=participate.php>here</a>.
<li>
To submit an add-on for inclusion in this list, please
email <a href=contact.php>David Anderson</a>.
</ul>
";

}

function show_group($name, $list) {
	echo "
		<h2>$name</h2>
		<table border=1 cellpadding=6 width=100%>
		<tr>
			<th>Application<br><font size=-2>click to download</font></th>
			<th>Version</th>
			<th>Description</th>
		</tr>
	";
	shuffle($list);
	foreach ($list as $item) {
		$file = $item[0];
		$itemname = $item[1];
		$version = $item[2];
        if (!$version) $version ='<br>';
		$desc = $item[3];
		echo "
			<tr><td><a href=addons/$file>$itemname</a></td>
				<td>$version</td>
				<td>$desc</td>
			</tr>
		";
	}
	echo "</table>
	";
}

$win = array(
array('boinc-irc.exe', 'Boinc mIRC', '1.0', 'A ready setup mIRC to take you straight there'),
array('boinc.mrc', 'Boinc mircstats', '0.4', 'Script to show your Boinc stats on IRC (with mIRC)'),
array('boincdv_v0306.zip', 'BoincDV', '', 'A small util to view current project debts for your v4.4x BOINC cc.'),
array('BoincLogX_Setup_v1.51.exe', 'BoincLogX', '1.51', 'BoincLogX creates detailed logfiles for all BOINC projects.'),
array('BoincSpy_455.zip', 'BoincSpy', '1.0 (RC1)', 'Views BOINC project statistics'),
array('boincview.zip', 'BoincView', '1.2.5', 'Advanced BOINC manager for networks'),
array('setup.exe', 'CPDNSpy', '', 'Stats/Benchmarkprogram. Only for CPDN!!!'),
array('sahuserstats.xpi', 'SAH User Statistics', '', 'An externsion for Firefox (may work under Mozilla) that will display your stats for SAH'),
array('SETI_at_BOINCWatch_1.10.15.exe', 'SETI@BOINCWatch', '1.10.15', 'A SETI@Home/BOINC client watcher! User statistics powered by BOINCStats'),
array('SETIatBOINCWatch(.NET2.0).exe', 'SETI@BOINCWatch(.NET 2.0)', '3.0.24 BETA', 'A SETI@Home/BOINC client watcher! User statistics powered by BOINCStats. But now built with the .NET environment.'),
array('SETIatBOINCWatch(.NET).exe', 'SETI@BOINCWatch(.NET)', '2.0.24 BETA', 'A SETI@Home/BOINC client watcher! User statistics powered by BOINCStats. But now built with the .NET environment.'),
array('SETITracker_V1.0BETA.zip', 'SETI@Boinc Tracker', '1.0BETA', 'SETI@Boinc Tracker is a GUI Client for Tracking Boinc CLI Client!'),
array('SetiMapView_Setup_v6.54.exe', 'SETI@home-MapView', '6.54', 'SETI@home-Mapview creates skymaps for the BOINC projects SETI@home-II, Einstein@home and Astropulse.'),
array('DBSetup.zip', 'SQL Setup for BoincSpy', 'V 0.7', 'Stores Workunits into SQL database'),
array('spy_pp_v101_setispy_v341.zip', 'Spy++', '', 'Loader for the new version of SetiSpy (3.4.1), the famous SETI addon of Roelof'),
array('tminst112.zip', 'ThreadMaster', '1.12build128', 'Control max CPU usage and prevent overheat'),
array('boinclogger.zip', 'boinc logger', '1.0', 'saves the messages part of boinc so you can track down errors'),
array('inst_nB_v13.exe', 'nBOiNC', '1.3.x', 'stats picker'),
);

$linux = array(
array('boincstat-2.02.tar', 'BOINCSTAT', '2.02', 'Command line view of BOINC status'),
array('boincprog-1.1.5.tar.gz', 'BOINCprog', '', 'A BOINC project progress monitor'),
array('boinc_lcs_1.0_linux.tar.gz', 'Boinc LCS', '1.0', 'Live client state system for up to 4 clients (needs a webserver)'),
array('boinctool.tar.gz', 'Boinc Tool - Bash Script', '', 'Bash Scripts for Seti@Boinc'),
array('boinc-2.0.tar.bz2', 'Boinc bash script', '', 'Boinc state displayer'),
array('CPDNSpy.rar', 'CPDNSpy Linux/Mac', '1.3', 'Stats/Benchmarkprogram. Only for CPDN!!!'),
array('cpulimit-1.1.tar.gz', 'CPU limiter', '1.1', 'Limits the CPU usage (good for laptops)'),
array('SETITracker_V1.0BETA.zip', 'SETI@Boinc Tracker', '1.0BETA', 'SETI@Boinc Tracker is a GUI Client for Tracking Boinc CLI Client!'),
array('boinc-suse-1.1.1.tar.gz', 'boinc-suse', '', 'Init script for SuSE 9.2 and 9.3'),
array('boincctl', 'boincctl', '1.2', 'Script to start boinc at boot time.'),
array('boincgui-0.4.tar.gz', 'boincgui', '', 'Linux GUI display of BOINC'),
array('boinctop-0.5.16.tar.gz', 'boinctop', '0.5.16', 'top like view of boinc status'),
array('rc.boinc-slackware-linux-latest.txt', 'rc.boinc (Slackware)', '1.4', 'rc.boinc - Slackware Startup Script'),
);

$mac = array(
array('boinc_stat_viewer_1.3.2.zip', 'Boinc Stat Viewer', '', 'This small Cocoa application shows current state of your BOINC client.'),
array('CPDNSpy.rar', 'CPDNSpy Linux/Mac', '1.3', 'Stats/Benchmarkprogram. Only for CPDN!!!'),
array('SETIControl.hqx', 'SETI Control', '3.2', 'REALbasic application which allows you to control the Darwin (Unix) version of setiathome-BOINC'),
array('SETITracker_V1.0BETA.zip', 'SETI@Boinc Tracker', '1.0BETA', 'SETI@Boinc Tracker is a GUI Client for Tracking Boinc CLI Client!'),
array('scr0.9.dmg', 'Seti Check Revolutions', 'v0.9', 'Boinc state displayer. Only for seti@home!'),
);

$web = array(
array('BOINC_User_Statistics_Widget.widget', 'BOINC User Statistics Widget', '2.2', 'Widget displays statistics for BOINC projects'),
array('boinc_lcs_2.1.tar.gz', 'Boinc LCS', '', 'Live Client State for an unlimited number of clients (needs a webserver)'),
array('boincphpgui-2.3.tar.gz', 'BoincPHP5-GUI', '2.3', 'Monitor and control Boinc from a web browser'),
array('goinc.zip', 'GOINC', '', 'GOINC: A Google homepage module for keeping tabs on BOINC.'),
array('SetiathomeBoincProgressStatus-1.0.0.tar.gz', 'SETI@home/BOINC progress status', '1.0.0', 'Show your SETI@home/BOINC progress on your homepage'),
array('phpboinc10.zip', 'phpBOINC', '1.0', 'Script to display BOINC stats on a webpage.'),
);

show_group("Windows", $win);
show_group("Linux", $linux);
show_group("Mac", $mac);
show_group("Web applications", $web);

if (!$strip_header) {
page_tail();
}
?>
