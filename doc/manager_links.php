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
// $control_id is the control identifier for the control that captured
//   the context sensitive help request. Please see the Events.h file
//   in the clientgui directory for a list of valid control ids.
//

$target = null;
$version = null;
$control_id = null;

if (isset($_GET['target'])) $target = $_GET['target'];
if (isset($_GET['version'])) $version = $_GET['version'];
if (isset($_GET['controlid'])) $control_id = $_GET['controlid'];

if (($target == "advanced") && version_compare($version, "5.10.0", ">=")) {
	if ($control_id == "6024") {
		header('Location: http://boinc.berkeley.edu');
	} else if ($control_id == "6025") {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view');
	} else if ($control_id == "6035") {
		header('Location: http://boinc.berkeley.edu/help.php');
	} else {
		header('Location: http://boinc.berkeley.edu/wiki/Advanced_view');
	}
} else if (($target == "simple") && version_compare($version, "6.2.0", ">=")) {
    if ($control_id == "6024") {
        header('Location: http://boinc.berkeley.edu');
    } else if ($control_id == "6025") {
        header('Location: http://boinc.berkeley.edu/wiki/Simple_view');
    } else if ($control_id == "6035") {
        header('Location: http://boinc.berkeley.edu/help.php');
    } else {
        header('Location: http://boinc.berkeley.edu/wiki/Simple_view');
    }
} else if ($target == "advanced_preferences") {
    header('Location: http://boinc.berkeley.edu/wiki/Preferences');
} else {
    if ($target == "advanced") {
        header('Location: http://boinc.berkeley.edu/wiki/Advanced_view');
    } else {
        header('Location: http://boinc.berkeley.edu/help.php');
    }
}
