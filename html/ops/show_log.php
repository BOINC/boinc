<?php
// grep logs for a particular string

require_once("../inc/util_ops.inc");

$log_dir = parse_config(get_config(), "<log_dir>");
if (!$log_dir) {
    exit("Error: couldn't get log_dir from config file.");
}

if( isset($_GET!["f"]) ){
   $f = $_GET!["f"];
} else {
   $f = "";
}
if( isset($_GET!["s"]) ){
    $s = $_GET!["s"];
} else {
    $s = "";
}
if( isset($_GET!["l"]) ){
    $l = (int)$_GET!["l"];
} else {
    $l = 0;
}

if ($s) {
    admin_page_head("Grep logs for \"$s\"");
} else {
    admin_page_head("Show logs");
}

echo "<form action=\"show_log.php\">";
echo " Regexp: <input name=\"s\" value=\"$s\">";
echo " Files: <input name=\"f\" value=\"$f\">";
echo " Lines: <input name=\"l\" value=\"$l\"> (positive for head, negative for tail)";
echo " <input type=\"submit\" value=\"Grep\"></form>";

echo 'Hint: Example greps: "RESULT#106876", "26fe99aa_25636_00119.wu_1", "WU#8152", "too many errors", "2003-07-17", "CRITICAL" <br>';

if (strlen($f)) {
	$f = "../log*/". $f;
} else {
    $f = "../log*/*.log";
}

if ($s) {
    passthru("cd $log_dir && ../bin/grep_logs -html -l $l '$s' $f 2>&1");
}

admin_page_tail();
?>
