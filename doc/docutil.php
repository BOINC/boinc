<?php

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
        <hr size=0 noshade>
    ";
}

function page_tail() {
    echo "
        <hr size='0' noshade/>
        <p align='center'>
        <a href='/'>Return to BOINC main page</a>
        <br/><br/>
        Copyright &copy; 2004 University of California
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

function list_item($x, $y, $z=null) {
    echo "
        <tr>
            <td bgcolor=d8d8ff valign=top><b>$x</b></td>
            <td valign=top>$y</td>
";
    if ($z) {
        echo "       <td valign=top>$z</a>\n";
    }
    echo " </tr>\n";
}

function list_item_func($x, $y) {
    list_item(html_text($x), $y);
}

function list_bar($x) {
    echo "
        <tr><td colspan=3 bgcolor=b8b8ff><center><b>$x</b></center></td></tr>
    ";
}

function list_end() {
    echo "</table><p>\n";
}

?>
