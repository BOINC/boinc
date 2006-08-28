<?php
require_once("docutil.php");
page_head("Groups and permissions");
echo "
BOINC server programs run as two different users:
<ul>
<li>
The scheduler and file upload handler are CGI programs,
so they run as the same user as the web server
(typically user 'apache', group 'apache').
<li>
BOINC daemons runs as whoever created the project
(let's say user 'boincadm', group 'boinc').
</ul>
By default, the directories created by user apache are not world-writeable.
This causes problems:
for example, when the file upload handler creates a
directory in the <a href=hier_dir.php>upload hierarchy</a>,
it's owned by (apache, apache),
and the <a href=file_deleter.php>file deleter</a>
(which runs as boincadm)
won't be able to delete the files there.

<h3>Recommended solution</h3>
<p>
Edit /etc/group so that apache belongs to group boinc, i.e. the line:
<pre>
boinc:x:566:
</pre>
becomes:
<pre>
boinc:x:566:apache
</pre>
(Apache will need to be stopped/restarted for this to take effect.)
<p>
When you create a BOINC project using
<a href=make_project.php>make_project</a>,
the critical directories are owned by boincadm
and have the set-GID bit set;
this means that any directories or files created by apache
in those directories will have group boinc (not group apache).
The BOINC software makes all directories group read/write.
Thus, both apache and boinc will have read/write access to
all directories and files,
but other users will have no access.
<p>
On an existing project, do:
<pre>
chmod 02770 upload
chmod 02770 html/cache
chmod 02770 html/inc
chmod 02770 html/languages
chmod 02770 html/languages/compiled
chmod 02770 html/user_profiles
</pre>
You may also need to change the ownership of these directories
and all their subdirectories to boincadm/boinc.

<p>
If you're running several projects on the same server and
want to isolate them from each other,
you can create a different user and group for each project,
and add apache to all of the groups.

<h3>Non-recommended solutions</h3>
<p>
The following solutions should work,
but may introduce security vulnerabilities:
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
Now any file apache creates should have group writable permissions
(thanks to the umask) and user boincadm, who now belongs to group
apache, will be able to update/delete these files.
</ul>
";
page_tail();
?>
