<?
require_once("docutil.php");
page_head("The make_project script");
echo "
<p>
BOINC provides a script for setting up a BOINC project.
This has been tested only on Linux and Solaris hosts;
it may work with small modifications on Windows also.

<p>
Install all components listed in the <a href=software.php>Software
Prerequisites</a> page.
Your operating system must have shared memory enabled,
with a max shared segment size of at least 32 MB.

<h3>Creating the server</h3>
<p>
Run the <code>make_project</code> script; example command lines:
<pre>
    cd tools/
    ./make_project yah
</pre>
will create $HOME/projects/yah with html_user_url set to
http://<hostname>/yah/ and long project name 'Yah'.
<pre>
    cd tools/
    ./make_project --base $HOME/boinc --url_base http://boink/ yah 'YETI @ Home'
</pre>
will create $HOME/boinc/projects/yah with the html_user_url set to
http://boink/yah/.

<p>
See 'make_project --help' for more command-line options available (such as
finer control of directory structure or clobbering an existing installation).

<p>
The script does the following:
<ul>
<li> Create the project directory and its subdirectories.
<li> Create the project's encryption keys if necessary.
NOTE: before making the project publicly visible,
you must move the code-signing private key
to a highly secure (preferably non-networked) host,
and delete it from the server host.
<li> Create and initialize MySQL database
<li> Copy source and built files
<li> Generate the configuration file (config.xml) used by
the server programs.
</ul>

<p>
The script gives further instructions, namely
<ul>
<li>It generates a template Apache config file which you can edit and
paste into /etc/apache/httpd.conf (path varies), or Include directly
if it is already perfect.
<li>It generates a crontab line to paste.
</ul>

The php scripts need access to the database,
so the user that Apache runs under needs SELECT,INSERT,UPDATE,DELETE
to the database just created.

<p>
When you run the <code>make_project</code> script it will give you lines to
append to your Apache <code>httpd.conf</code>.
(Basically you need to alias html_user, alias html_ops,
 and script-alias cgi-bin, all with appropriate directory permissions.)
It will also give you a crontab line to install.

<p>
You should also set the default MIME type as follows:
<pre>
DefaultType application/octet-stream
</pre>

";
page_tail();
?>
