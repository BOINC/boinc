<?php

// The BOINC client uses this to get a current list of projects,
// which it does every 14 days.
// Don't break backwards compatibility!

require_once("projects.inc");
require_once("account_managers.inc");
require_once("get_platforms.inc");

header('Content-type: text/xml');
echo '<?xml version="1.0" encoding="ISO-8859-1" ?>
<projects>
';

$proj_list = array();

shuffle($areas);
foreach ($areas as $area) {
    $area_name = $area[0];
    $projects = $area[1];
    shuffle($projects);
    foreach ($projects as $p) {
        $np = null;
        if ($p[5]) {
            $np->image = $p[5];
        }
        $np->url = $p[1];
        $np->web_url = $p[1];
        if (array_key_exists(6, $p)) {
            $np->web_url = $p[6];
        }
        $np->home = $p[2];
        $np->general_area = $area_name;
        $np->specific_area = $p[3];
        $np->description = $p[4];
        $np->name = $p[0];

        $proj_list[] = $np;
    }
}

foreach($proj_list as $p) {
    echo "    <project>
        <name>$p->name</name>
        <url>$p->url</url>
        <general_area>$p->general_area</general_area>
        <specific_area>$p->specific_area</specific_area>
        <description>$p->description</description>
        <home>$p->home</home>
";
    $platforms = get_platforms_cached($p->web_url);
    if ($platforms) {
        echo "    <platforms>\n";
        foreach ($platforms as $platform) {
            if ($platform == 'Unknown') continue;
            echo "        <name>$platform</name>\n";
        }
        echo "    </platforms>\n";
    }
    if (isset($p->image)) {
        echo "      <image>http://boinc.berkeley.edu/images/$p->image</image>
";
    }
    echo "    </project>
";
}

foreach ($account_managers as $am) {
    $name = $am[0];
    $url = $am[1];
    $desc = $am[2];
    $image = $am[3];
    echo "   <account_manager>
        <name>$name</name>
        <url>$url</url>
        <description>$desc</description>
        <image>http://boinc.berkeley.edu/images/$image</image>
    </account_manager>
";
}

echo "</projects>
";

?>
