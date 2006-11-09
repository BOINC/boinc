<?php

require_once("docutil.php");
page_head("Software prerequisites (Unix/Linux)");

echo "

The various parts of BOINC have dependencies on other software;
most or all of these may already be present on your system.
You'll need to download and install the ones you don't have.
The list depends on the system you're on,
and the parts of BOINC that you need.
<b>If you're creating a BOINC project,
you don't need to build the Core client or BOINC Manager.</b>
<p>

<table border=1 cellpadding=8>
<tr>
    <th><br></th>
    <th>Server</th>
    <th>Core client<br></th>
    <th>BOINC Manager<br></th>
    <th>Applications<br>(non-graphical)</th>
    <th>Applications<br>(graphical)</th>
</tr>
<tr>
    <td>GNU tools (find them <a href=http://directory.fsf.org/GNU/>here</a>):
        <br>
        <br>make 3.79+
        <br>m4 1.4+
        <br>libtool 1.4+
        <br>pkg-config 0.15+
        <br>autoconf 2.58+</a>
        <br>automake 1.8+</a>
        <br>GCC 3.0.4+
    </td>
    <td>X</td>
    <td>X</td>
    <td>X</td>
    <td>X</td>
    <td>X</td>
</tr>
<tr>
    <td>Python 2.2+
        <br>
            <a href=http://sourceforge.net/projects/mysql-python>MySQLdb module 0.9.2+</a>
            (see <a href=install_python_mysqldb.txt>installation instructions</a>)
        <br>xml module
    </td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td>MySQL 4.0+ or 4.1+
        <br>MySQL client
    </td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td>Apache with mod_ssl and PHP 4.0.6+
    </td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://www.openssl.org/>OpenSSL</a> version 0.9.8+
    </td>
    <td>X</td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://curl.haxx.se/>libcurl</a> version 7.15.5
    </td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://www.wxwidgets.org/>WxWidgets</a> 2.6.3
    <p>
<ul>
<li>Configure with the <code>--with-gtk --disable-shared</code> options
(BOINC needs a static library).
<li>If you have an older WxWidgets installed,
uninstall it (make uninstall), then install 2.6.
<li> Make sure you have the 'development' version installed
</ul>

    </td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td>Graphics libraries: GL, GLU, GLUT (or freeglut)
    <p>
    <ul>
    <li> You'll need a static (.a) version of GLUT or freeglgut
    (some Linux distributions come with only a dynamic version).
    <li> Use the 'development' version of GLUT or freeglut.
    </ul>
    </td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
</tr>
<tr>
    <td>jpeglib, X11 libraries and include files</td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td>X</td>
</tr>
</table>

";
page_tail();
?>
