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
Run the <code>make_project</code> script.
For example:
<pre>
    cd tools/
    ./make_project cplan
</pre>
creates a project with master URL
http://&lt;hostname>/cplan/
whose directory structure is rooted at
\$HOME/projects/yah.
<pre>
    cd tools/
    ./make_project --base \$HOME/boinc --url_base http://boink/ cplan 'Cunning Plan'
</pre>
creates a project with master URL
http://boink/cplan/ and long name <b>Cunning Plan</b>,
rooted at \$HOME/boinc/projects/cplan.

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
<li> Create and initialize the MySQL database
<li> Copy source and executable files
<li> Generate the project's configuration file.
</ul>

<p>
The script gives further instructions, namely
<ul>
<li>It generates a template Apache config file that you can insert
into /etc/apache/httpd.conf (path varies), or Include directly.
<li>It generates a crontab line to paste.
</ul>

The PHP scripts need access to the database,
so the user that Apache runs under needs SELECT,INSERT,UPDATE,DELETE
to the database just created.

<p>
You should also make sure httpd.conf sets the default MIME type as follows:
<pre>
DefaultType application/octet-stream
</pre>

";
page_tail();
?>
