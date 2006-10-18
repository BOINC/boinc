<?php

function search_form() {
    echo "<form method=get action=http://google.com/search>
    <input type=hidden name=domains value=http://boinc.berkeley.edu>
    <input type=hidden name=sitesearch value=http://boinc.berkeley.edu>
    <nobr>
    <input class=small name=q size=30>
    <input type=image name=submit src=images/search.gif alt=Search>
    </nobr>
    </form>
";
}

$light_blue="#d8e8ff";
$med_blue="#c0d0f0";

function last_mod($datefile) {
    return gmdate("g:i A \U\T\C, F d Y", filemtime($datefile));
}

function page_head($title) {
    if (defined("CHARSET")) {
        header("Content-type: text/html; charset=".tr(CHARSET));
    }

    echo "
        <html>
        <head>
        <link rel=\"stylesheet\" type=\"text/css\" href=\"white.css\"/>
        <link rel=\"shortcut icon\" href=\"iconsmall.ico\"/>
        <title>$title</title>
        </head>
        <body bgcolor='ffffff'>
        <table width='100%'>
        <tr>
        <td><center><h1>$title</h1></center>
        <td align=right><a href=index.php><img src=logo/logo_small.png></a>
        <br>
";
        search_form();
echo "
        </td>
        </tr></table>
        <hr size=1>
    ";
}

function copyright() {
    $y = date("Y ");
    echo "
        Copyright &copy; $y University of California.
        Permission is granted to copy, distribute and/or modify this document
        under the terms of the GNU Free Documentation License,
        Version 1.2 or any later version published by the Free Software Foundation.
    ";
}

function page_tail($translatable=false) {
    $datefile = $_SERVER["SCRIPT_FILENAME"];
    $d = last_mod($datefile);
    echo "
        <hr size=1>
        <center>
        <a href=\"/\">Return to BOINC main page</a>
        </center><p>
            <br>
            <font color=888888 size='2'>
    ";
    if ($translatable) {
        echo "
            This page is <a href=translation.php>translatable</a>.<br>
        ";
    }
    echo "
            Last modified $d.<br>
    ";
    copyright();
    echo "
        </font>
        </body>
        </html>
    ";
}

function html_text($x) {
    return "<div bgcolor=$light_blue><pre>".htmlspecialchars($x)."</pre></div>
    ";
}

function list_start($attrs = 'width=100%') {
    echo "<p><table border=1 cellpadding=6 $attrs>\n";
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

function error_page($x) {
    page_head("Error");
    echo $x;
    page_tail();
    exit();
}

function block_start() {
    global $light_blue;
    echo "
        <table width=100% cellpadding=4>
        <tr>
        <td bgcolor=$light_blue width=100%><pre>";
}

function block_end() {
    echo "</pre></td></tr></table>
    ";
}

function get_str($name) {
    if (isset($_GET[$name])) {
        $x = $_GET[$name];
        $x = trim($x);
        return mysql_real_escape_string($x);
    }
    return null;
}

?>
