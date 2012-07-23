<?php
//
// This page redirects help requests from the stock BOINC manager
//
//
// $target can be any of the following:
//   advanced = advanced GUI help requests
//   simple = simple GUI help requests
//
// $version is the verson number of the BOINC Manager requesting help.
//   Only valid for BOINC Manager 5.9.3 or better
//
// $controlid is the control identifier for the control that captured
//   the context sensitive help request. Please see the Events.h file
//   in the clientgui directory for a list of valid control ids.
//

$target = null;
$version = null;
$controlid = null;

if (isset($_GET['target'])) $target = $_GET['target'];
if (isset($_GET['version'])) $version = $_GET['version'];
if (isset($_GET['controlid'])) $controlid = $_GET['controlid'];

if ($target == "notice") {
	if ($controlid == 'download') {
		header('Location: http://boinc.berkeley.edu/wiki/Release_Notes');
	} else if ($controlid == 'statefile') {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	} else if ($controlid == 'proxy_env') {
		header('Location: http://boinc.berkeley.edu/wiki/Client_configuration#Environment_variables');
	} else if ($controlid == 'app_info') {
		header('Location: http://boinc.berkeley.edu/wiki/Anonymous_platform');
	} else if ($controlid == 'remote_hosts') {
		header('Location: http://boinc.berkeley.edu/wiki/Controlling_BOINC_remotely');
	} else if ($controlid == 'log_flags') {
		header('Location: http://boinc.berkeley.edu/wiki/Client_configuration#Logging_flags');
	} else if ($controlid == 'config') {
		header('Location: http://boinc.berkeley.edu/wiki/Client_configuration#Configuration_file');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	}
} else if  ($target == "advanced_preferences") {
    header('Location: http://boinc.berkeley.edu/wiki/Preferences');
} else if (($target == "advanced") && version_compare($version, "5.10.0", ">=") && version_compare($version, "6.12.0", "<")) {
	if ($controlid == "6024") {
		header('Location: http://boinc.berkeley.edu');
	} else if ($controlid == "6025") {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view_6_10');
	} else if ($controlid == "6035") {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view_6_10');
	}
} else if (($target == "simple") && version_compare($version, "5.10.0", ">=") && version_compare($version, "6.12.0", "<")) {
	if ($controlid == "6024") {
		header('Location: http://boinc.berkeley.edu');
	} else if ($controlid == "6025") {
		header('Location: http://boinc.berkeley.edu/wiki/Simple_view');
	} else if ($controlid == "6035") {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	}
} else if (($target == "advanced") && version_compare($version, "6.12.0", ">=") && version_compare($version, "7.0.0", "<")) {
	if ($controlid == "6024") {
		header('Location: http://boinc.berkeley.edu');
	} else if ($controlid == "6025") {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view_6_12');
	} else if ($controlid == "6035") {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view_6_12');
	}
} else if (($target == "simple") && version_compare($version, "6.12.0", ">=") && version_compare($version, "7.0.0", "<")) {
	if ($controlid == "6024") {
		header('Location: http://boinc.berkeley.edu');
	} else if ($controlid == "6025") {
		header('Location: http://boinc.berkeley.edu/wiki/Simple_view_6_12');
	} else if ($controlid == "6035") {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
	}
} else {
    if ($target == "advanced") {
		if ($controlid == "6024") {
			header('Location: http://boinc.berkeley.edu');
		} else if ($controlid == "6025") {
			header('Location: http://boinc.berkeley.edu/wiki/Advanced_view');
		} else if ($controlid == "6035") {
			header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
		} else {
			header('Location: http://boinc.berkeley.edu/wiki/Advanced_view');
		}
	} else if ($target == "simple") {
		if ($controlid == "6024") {
			// "Show info about BOINC" item on Mac simple-view menu
			//
			header('Location: http://boinc.berkeley.edu');
		} else if ($controlid == "6025") {
			// "Show info about BOINC manager" item on Mac simple-view menu
			//
			header('Location: http://boinc.berkeley.edu/wiki/Simple_view');
		} else if ($controlid == "6035") {
			// "Show info about BOINC and BOINC Manager"
			// item on Mac simple-view menu ?? do we need this item?
			//
			header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
		} else {
			// the question-mark button
			//
			header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
		}
	} else {
        header('Location: http://boinc.berkeley.edu/wiki/BOINC_Help');
    }
}

?>
