<?php

/**
 * This index-file helps setup and debug any Bittorrent tracker and generator installation through
 * the use of some built-in checks in the code.
 * It is advised that you leave the index file in place even after succesful installation so that any
 * later debugging can easily be done.
 *
 * Also, at some point, this file will display some statistical information about the data distributed
 * through the use of the tracker.
 */
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>

<html>
    <head>
	<title>BOINC Bittorrent</title>
    </head
    <body>
	<h1>BOINC Bittorrent</h1>
	<p>
	    This page helps you identify whether the BOINC Bittorrent system has succesfully been installed
	    on the server. If all of the bellow checks are green and all of the settings seem to match your
	    setup then everything should be perfectly fine.<br />
	    If, for some reason, the entire page (including the text at the bottom, bellow the table of checks)
	    isn't displayed, you should have a look at the error-log for your server. Similarly, if one of the
	    checks fail you may find additional information on the page or in your server logs.
	</p>
	<table>
	    <tr>
		<th>Check</th><th>Status</th>
	    </tr>
<?php
function showCheck($check, $status, $comment){
    echo "<tr><td>".$check."</td><td>";
    if ($status){
	echo "<font color='green'>OK";
    } else {
	echo "<font color='yellow'>Hm...";
    }
    echo "</td><td>";
    if (!$status) echo $comment;
    echo "</td></tr>";
}
	showCheck("Linking to config.php", file_exists("./config.php"), "config.php is missing - did you remember to copy from the sample?");
	require_once("./config.php");
	showCheck("Linking to BOINC serverside framework", file_exists("../inc/util.inc"), "Cannot find the BOINC framework - did you install the BT system in the html/-directory of your BOINC installation?");
	require_once("../inc/util.inc");
	showCheck("Database link", (db_init()||true), "");
	showCheck("bittorrent_files table", mysql_query("select * from bittorrent_files"), "Table inaccessible");
	showCheck("bittorrent_ipbans table", mysql_query("select * from bittorrent_ipbans"), "Table inaccessible");
	showCheck("bittorrent_peers table", mysql_query("select * from bittorrent_peers"), "Table inaccessible");
	showCheck("bittorrent_statistics table", mysql_query("select * from bittorrent_statistics"), "Table inaccessible");
	showCheck("Linking to download dir (".$fileDirectory.")", file_exists($fileDirectory), "Directory not accessible or present");
	showCheck("Tracker present (".$trackerURL.")", fopen($trackerURL, "r"), "Either this webserver doesn't support URL-fopen-wrappers or the tracker is not available. In the first case you may safely ignore this warning.");
	showCheck("Webseeds defined", (sizeof($webseeds)>0), "No webseeds defined");
	foreach ($webseeds as $webseed){
	    showCheck("Seed present (".$webseed.")", fopen($webseed, "r"), "Either this webserver doesn't support URL-fopen-wrappers or the webseed is not present. If the first is the case this warning may safely be ignored.");
	}
	showCheck("Linking to logfile (".TRACKER_LOGFILE.")", file_exists(TRACKER_LOGFILE), "The logfile doesn't exist - this may be because the system hasn't been run yet or because the file couldn't be created.");
?>
	</table>
	<p>
	    Files used in this distribution:
	    <ul>
		<?php 
		    for ($i=0;$i<sizeof($cvs_version_tracker);$i++) {
			echo "<li>".$cvs_version_tracker[$i]."\n";
		    }
		?>
	    </ul>
	</p>				   
	<p>
	    For more information check the documentation delivered within the bt directory.
	</p>
    </body>
</html>	