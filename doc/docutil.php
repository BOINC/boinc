<?php

$light_blue="#d8e8ff";
$med_blue="#c0d0f0";

function last_mod() {
    return gmdate("g:i A \U\T\C, F d Y", filemtime($_SERVER["SCRIPT_FILENAME"]));
}

function page_head($title) {
    $d = last_mod();
    echo "<html>
        <head>
        <link rel='stylesheet' type='text/css' href='white.css'/>
        <link rel='shortcut icon' href='iconsmall.ico'/>
        <title>$title</title>
        </head>
        <body bgcolor='ffffff'>
        <table width='100%'>
        <tr>
        <td><center><h1>$title</h1></center>
        <td align=right><a href='.'><img src='boinc.gif'></a>
            <br>
            <nobr><font size='2'>Last modified $d</font></nobr>
        </td>
        </tr></table>
        <hr size=1>
    ";
}

function copyright() {
    echo "
        Copyright &copy; $y University of California
    ";
    return;
    echo "
        <br><br>
        <!-- Creative Commons License -->
        <a rel=\"license\" href=\"http://creativecommons.org/licenses/by-sa/2.0/\"><img alt=\"Creative Commons License\" border=\"0\" src=\"http://creativecommons.org/images/public/somerights20.gif\" /></a><br />
    ";
}

function page_tail() {
    $y = date("Y ");
    echo "
        <hr size=1>
        <p align='center'>
        <a href='/'>Return to BOINC main page</a>
        <br/><br/>
    ";
    copyright();
    echo "
        </p>
        </body>
        </html>
    ";
}

function html_text($x) {
    return "<pre>".htmlspecialchars($x)."</pre>
    ";
}

function list_start() {
    echo "<p><table border=1 cellpadding=6 width=100%>\n";
}

function list_heading($x, $y, $z=null) {
    echo "
        <tr>
            <th valign=top><b>$x</b></th>
            <th valign=top>$y</th>
";
    if ($z) {
        echo "       <th valign=top>$z</th>\n";
    }
    echo " </tr>\n";
}

function list_heading_array($x) {
    echo "<tr>";
    foreach ($x as $h) {
        echo "<th>$h</th>";
    }
    echo "</tr>\n";
}

function list_item($x, $y, $z=null) {
    global $light_blue;
    echo "
        <tr>
            <td bgcolor=$light_blue valign=top><b>$x</b></td>
            <td valign=top>$y</td>
";
    if ($z) {
        echo "       <td valign=top>$z</a>\n";
    }
    echo " </tr>\n";
}

function list_item_array($x) {
    echo "<tr>";
    foreach ($x as $h) {
        echo "<td valign=top>$h<br></td>";
    }
    echo "</tr>\n";
}

function list_item_func($x, $y) {
    list_item(html_text($x), $y);
}

function list_bar($x) {
    global $med_blue;
    echo "
        <tr><td colspan=8 bgcolor=$med_blue><center><b>$x</b></center></td></tr>
    ";
}

function list_end() {
    echo "</table><p>\n";
}

function stats_sites() {
    echo "
        <ul>
        <li>
        <a href=http://www.setisynergy.com/stats/index.php>BOINC Statistics for the WORLD!</a>
        developed by Zain Upton (email: zain.upton at setisynergy.com)
        <li>
        <a href=http://www.boincstats.com/>www.boincstats.com</a>
        by Willy de Zutter
        <li>
        <a href=http://www.seti.nl/boinc_team.php>
        SETI@Netherlands stats page</a>
        <li>
        <a href=http://www.boinc.dk/index.php?page=statistics>http://www.boinc.dk</a>,
        developed by Janus Kristensen (email: stats at boinc.dk).
        <li>
        <a href=http://www.saschapfalz.de/boincstats/boinc-stats.php>boincstats</a>,
        developed by Sascha Pfalz.

        <li>
        <a href=http://stats.boincbzh.net/BZHwds/index.php>BOINC Alliance Francophone</a>,
        developed by Vincent Mary (email: stats at hoincbzh.net).
        Supports competition between 'mini-teams'.
        <li>
        <a href=http://stats.kwsn.net/>The Knights Who Say 'Ni' stats</a>

        <li>
    <a href=http://www.teamocuk.com/>Team OcUK stats</a>
    </ul>
    ";
}
?>
