<?php

require_once("docutil.php");
page_head("Software prerequisites");

echo "
You need certain software packages to build BOINC.
The list depends on the system you're on,
and the parts of BOINC that you need.
<b>If you're creating a BOINC project,
you don't need to build the Core client or BOINC Manager.</b>

<h2>Unix</h2>
<p>
The various parts of BOINC have dependencies on other software;
most or all of these may already be present on your system.
You'll need to download and install the ones you don't have.

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
    (included with the BOINC source distribution for Windows)
    </td>
    <td>X</td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://curl.haxx.se/>libcurl</a> version 7.13.2+
    (included with the BOINC source distribution for Windows)
    </td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://www.wxwidgets.org/>WxWidgets</a> 2.6.1
    </td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td>GL, GLU, GLUT libraries</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
</tr>
<tr>
    <td>X11 libraries and include files</td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td>X</td>
</tr>
<tr>
    <td><code>configure</code> option</td>
    <td>--enable-server</td>
    <td>--enable-client</td>
    <td>--enable-client (if WxWidgets and X11 found)</td>
    <td>--enable-client</td>
    <td>--enable-client (if GL/GLU/GLUT found)</td>
</tr>
</table>
<h3>Operating system notes</h3>
<ul>
<li>
Some parts of the BOINC server (the feeder and scheduling server)
use shared memory.
On hosts where these run,
the operating system must have shared memory enabled,
with a maximum segment size of at least 32 MB.
How to do this depends on the operating system;
some information is
<a href=http://developer.postgresql.org/docs/postgres/kernel-resources.html>here</a>.
</ul>
<h3>WxWidgets notes</h3>
<p>
Configure with the <code>--with-gtk --disable-shared</code> options
(BOINC needs a static library).
<p>
If you have an older WxWidgets install,
uninstall it (make uninstall), then install 2.6.

<h3>MySQL notes</h3>
<ul>

<li>
After installing and running the server,
grant permissions for your own account and for
the account under which Apache runs
('nobody' in the following; may be different on your machine).
All mysql accounts should be password protected including root.
<pre>
    mysql -u root
    grant all on *.* to yourname@localhost identified by 'password';
    grant all on *.* to yourname identified by 'password';
    grant all on *.* to nobody@localhost identified by 'password';
    grant all on *.* to nobody identified by 'password';
</pre>
<li>
Set your PATH variable to include MySQL programs
(typically /usr/local/mysql and /usr/local/mysql/bin).

<li>
You'll need to back up your database.
Generally this requires stopping the project,
making a copy or snapshot, and restarting.
An example is <a href=mysql_backup.txt>here</a>.
<li>
BOINC gets MySQL compiler and linker flags from a program
called <code>mysql_config</code> which comes with your MySQL distribution.
This sometimes references libraries that are not part of your base system
installation, such as <code>-lnsl</code> or <code>-lnss_files</code>.
You may need to install additional packages
(often you can use something called 'mysql-dev' or 'mysql-devel')
or fiddle with Makefiles.

<li>
MySQL can be the bottleneck in a BOINC server.
To optimize its performance, read about
<a href=mysql_config.php>configuring MySQL for BOINC</a>.

<li>
<a href=mysql_cluster.txt>Notes on running MySQL on a cluster</a>.

</ul>
<h3>MySQLclient notes</h3>
<ul>
<li>
Configure mysql with the --enable-thread-safe-client switch.
<li>
Set your LD_LIBRARY_PATH to refer to the correct library.
</ul>

<h3>PHP notes</h3>
<ul>
<li> Make sure 'magic quotes' are enabled (this is the default).
The file /etc/php.ini should contain
<pre>
magic_quotes_gpc = On
</pre>
<li>
By default, BOINC uses PHP's <code>mail</code> function to send
email to participants.
This uses sendmail.
If this doesn't work, you can use
<a href=http://phpmailer.sourceforge.net/>PHPMailer</a> instead,
which is a very flexible mail-sending mechanism.
To do this:
<ul>
<li> Download PHPMailer and put it under PROJECT/html/inc/phpmailer.
<li> Set the following variables in your
PROJECT/html/project/project.inc file (substitute your own values):
<pre>
$USE_PHPMAILER = true;
$PHPMAILER_HOST = \"xxx.xxx.xxx\";
$PHPMAILER_MAILER = \"smtp\"; 
</pre>
</ul>
</ul>

<h3>Library notes</h3>
<p>
If you have old versions of libraries (curl, openssl etc.)
in /usr/lib, and newer versions somewhere else (like /usr/local/lib)
you must tell the linker where to find the newer versions:
<pre>
./configure LDFLAGS=-L/usr/local/lib
</pre>

<h3>GLUT notes</h3>
<p>
Freeglut 2.2, freeglut 2.4 and OpenGL Utility Toolkit (GLUT) libraries
are supported.  

<h3>X11 notes</h3>
<p>
To get the X11 support,
select the relevant options when you're installing Linux,
or (Redhat) go to System Settings/Add Software.

<p>
See notes for <a href=debian_linux_install.txt>Debian Linux</a>.

<h3>Apache notes</h3>
<p>
Read about <a href=groups.php>groups and permissions</a>.
<p>
In httpd.conf, set the default MIME type as follows
(otherwise you'll get file upload signature verification errors):
<pre>
DefaultType application/octet-stream
</pre>
<hr>

<h2>Windows</h2>
<table border=1 cellpadding=8>
<tr>
    <th><br></th>
    <th>Core client<br></th>
    <th>BOINC Manager<br></th>
    <th>Installer</th>
    <th>Applications</th>
</tr>
<tr>
    <td>Visual Studio .NET (Visual C++ 7.0)</td>
    <td>X</td>
    <td>X</td>
    <td><br></td>
    <td>X</td>
</tr>
<tr>
    <td>WxWidgets 2.6.2</td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td>Installshield X</td>
    <td><br></td>
    <td><br></td>
    <td>X</td>
    <td><br></td>
</tr>
</table>

<h3>WxWidgets notes</h3>
<ul>
<li> Download and install the WxWidgets source
    according to instructions on the web site.
<li> In Settings/Control Panel/System,
    select Advanced; click Environment Variables.
    Under 'User variables' click New.
    Create a variable named 'wxwin'
    with value 'c:\wx' (or wherever you installed it).
    Then restart Visual Studio.
</ul>

";
page_tail();
?>
