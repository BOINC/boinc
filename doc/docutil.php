<?

function last_mod() {
    return date("g:i A, F d Y", filemtime($_SERVER["SCRIPT_FILENAME"]));
}

function page_head($title) {
    $d = last_mod();
    echo "
        <head>
        <link rel='shortcut icon' href='iconsmall.ico'>
        <title>$title</title>
        </head>
        <body bgcolor=ffffff>
        <table width=100%>
        <tr>
        <td><center><h1>$title</h1></center>
        <td align=right><img src=boinc.gif>
            <br>
            <nobr><font size=2>Last modified $d</font></nobr>
        </td>
        </tr></table>
        <hr size=0 noshade>
    ";
}

function page_tail() {
    echo "
        <hr size=0 noshade>
        <center>
        <a href=index.html>Return to BOINC main page</a>
        <br><br>
        Copyright &copy; 2003 University of California
    ";
}

function list_start() {
    echo "<p><table border=1 cellpadding=6 width=100%>\n";
}

function list_item($x, $y) {
    echo "
        <tr>
            <td bgcolor=ffffcc valign=top><b>$x</b></td>
            <td valign=top>$y</td>
        </tr>
    ";
}

function list_end() {
    echo "</table><p>\n";
}

?>
