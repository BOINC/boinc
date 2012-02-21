<?php

require_once("docutil.php");

function download_link() {
    return "
        <table border=4 cellpadding=10 width=200><tr><td bgcolor=ccccff>
        <a href=xx><font size=4><u>Download</u></font></a>
        <br>Version 5.6.3
        <br>5.8 MB
        </td></tr></table>
    ";
}

page_head("BOINC: compute for science");
echo "
<h2>Download BOINC</h2>
<table width=600 cellpadding=8 border=2>
<tr>
    <td valign=top><img src=images/ico-win.png> <b>Windows</b></td>
    <td> ".download_link()." </td>
</tr>
<tr>
    <td valign=top><img src=images/ico-osx-uni.png> <b>Mac OS X</b>
    <br>Graphical version</br>
    </td>
    <td> ".download_link()." </td>
</tr>
<tr>
    <td valign=top><img src=images/ico-osx-uni.png> <b>Mac OS X</b>
    <br>Command-line version</br>
    </td>
    <td> ".download_link()." </td>
</tr>
<tr>
    <td valign=top><img src=images/ico-tux.png> <b>Linux</b></td>
    <td> ".download_link()." </td>
</tr>
</table>
";
page_tail();
?>
