<?php

// copy project-web-site translation files from new to old location.
// This is a kludge, but the simplest thing to do at this point

$d = opendir(".");
while ($f = readdir($d)) {
    if ($f == '.') continue;
    if ($f == '..') continue;
    $x = $f."/BOINC-Project-Generic.po";
    if (file_exists($x)) {
        $cmd = "cp $x ../html/languages/translations/$f.po";
        system($cmd);
    }
}

system("cp templates/BOINC-Project-Generic.pot ../html/languages/translations/en.po");

?>
