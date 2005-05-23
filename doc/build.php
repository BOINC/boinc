<?php

require_once("docutil.php");
page_head("Software prerequisites");
?>

<h2>Unix</h2>
We develop on Solaris 2.6-2.9, Red Hat 8 and Enterprise Edition,
Mac OS X, and Debian Linux stable and unstable,
so those currently work out-of-the-box.
Other Unix-like systems should work without too much configuration.
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
    <td>GNU tools:
        <br>GCC 3.0.4+
        <br> <a href=http://www.gnu.org/software/autoconf/>
        autoconf 2.59+</a>
        <br> <a href=http://www.gnu.org/software/automake/automake.html>
        automake 1.9.3+</a>
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
    <td>Apache with mod_ssl and PHP 4.0+
    </td>
    <td>X</td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
    <td><br></td>
</tr>
<tr>
    <td><a href=http://www.wxwidgets.org/>WxWidgets</a> 2.6
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
Configure with the <code>--with-gtk --disable_shared</code> options
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

</ul>

<h3>PHP notes</h3>
<ul>
<li> Make sure 'magic quotes' are enabled (this is the default).
The file /etc/php.ini should contain
<pre>
magic_quotes_gpc = On
</pre>
</ul>

<h3>X11 notes</h3>
<p>
To get the X11 support,
select the relevant options when you're installing Linux,
or (Redhat) go to System Settings/Add Software.

<li>
Notes for <a href=debian_linux_install.txt>Debian Linux</a>.
</ul>

<h3>Apache notes</h3>
<p>
Make sure httpd.conf sets the default MIME type as follows:
<pre>
DefaultType application/octet-stream
</pre>
Suppose Apache runs as user 'apache'
and BOINC daemons runs as user 'boincadm'.
Directories created by apache need to be writeable to boincadm.
This can be done in any of several ways:
<ul>
<li> Use Apache's suexec mechanism
to make the CGI programs run as boincadm.
<li> Make the CGI programs setuid and owned by boincadm.
<li>
Edit /etc/group so that boincadm belongs
to group apache, i.e. the line:
<pre>
    apache:x:48:
</pre>
becomes:
<pre>
    apache:x:48:boincadm
</pre>
Add these two lines to the beginning of the apache start script
(called apachectl, usually in /usr/sbin on linux):
<pre>
    umask 2
    export umask
</pre>
Apache will need to be stopped/restarted for this to take effect.

Now any file apache creates should have group writeable permissions
(thanks to the umask) and user boincadm, who now belongs to group
apache, should be able to update/delete these files.
</ul>

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
    <td>WxWidgets 2.4.2</td>
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

<?
page_tail();
?>
