<?
require_once("docutil.php");
page_head("Creating a server complex");
echo "
<p>
BOINC provides a set of scripts for setting up and controlling a BOINC server
complex.
These scripts require all the server components to run on a single host.
This has been tested only on Linux and Solaris hosts;
it may work with small modifications on Windows also.

<p>
The scripts can be used to create multiple BOINC projects on the same host.
This can be handy for creating separate projects for testing and debugging.
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

</P>
At this point you need to add platform(s) and application(s) to the database
using the <a href=tool_add.php>add</a> tool or the <a
href=tool_xadd.php>xadd</a> tool, and add core versions and application
versions using the <a href=tool_update_versions.php>update_versions</a> tool.

<h3>Directory structure</h3>

Designate a 'BOINC projects' directory on the server host.
The scripts will create subdirectories as follows:
<pre>
boinc_projects/
    proj1/
        config.xml
        bin/
        cgi-bin/
        log/
        pid/
        download/
        html_ops/
        html_user/
        keys/
        upload/
    proj2/
    ...
</pre>
where proj1, proj2 etc. are the names of the projects you create.
Each project directory contains:
<ul>
<li>config.xml: configuration file
<li>bin: server daemons and programs,
including the main <code>start</code> program as well as all the daemons
<li>cgi-bin: cgi programs
<li>log: log output, lock files, pid files
<li> download: storage for data server downloads.
<li> html_ops: copies of PHP files for project management.
<li> html_user: copies of PHP files for the public web site.
<li> keys: encryption keys used by the project.
<li> upload: storage for data server uploads.
</ul>

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


<p>
At this point you hopefully have a functioning BOINC server,
but it has no applications or work to distribute.
The remaining steps to make a public project include:
<ul>
<li> Develop, debug and test your application.
<li> Develop back-end systems for generating work and processing results.
<li> Using the 'add' utility, add application versions to the BOINC database.
<li> Develop your web site.
</ul>

<p>
If you want distributed or replicated server components,
you'll need <a server_details.php>more information</a>.
";
page_tail();
?>
