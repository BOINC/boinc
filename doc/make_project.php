<?php
require_once("docutil.php");
page_head("The make_project script");
echo "
<p>
The <code>make_project</code> script creates
the server components of a BOINC project.
To use it, <a href=server.php>set up a BOINC server</a>.
Then, for example, type:
<pre>
    cd tools/
    ./make_project cplan
</pre>
creates a project with master URL
http://&lt;hostname>/cplan/
whose directory structure is rooted at
\$HOME/projects/cplan.

<p>
More specifically, <code>make_project</code> does the following:
<ul>
<li> Create the project directory and its subdirectories.
<li> Create the project's encryption keys if necessary.
NOTE: before making the project visible to the public,
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

The command-line syntax is as follows:
<pre>
make_project [options] project_name [ 'Project Long Name ' ]
</pre>

Options are as follows
(normally you don't need to include any of them):
";
list_start();
list_bar("directory options");
list_item("--project_root",
    "Project root directory path.  Default: \$HOME/projects/PROJECT_NAME"
);
list_item("--key_dir", "Where keys are stored.  Default: PROJECT_ROOT/keys");
list_item("--url_base", "Determines master URL  Default: http://\$NODENAME/");
list_item("--no_query", "Accept all directories without yes/no query");
list_item("--delete_prev_inst", "Delete project-root first (from prev installation)");

list_bar("URL options");
list_item("--html_user_url", "User URL.  Default: URL_BASE/PROJECT/");
list_item("--html_ops_url", "Admin URL.  Default: URL_BASE/PROJECT_ops/");
list_item("--cgi_url", "CGI URL.  Default: URL_BASE/PROJECT_cgi/");

list_bar("database options");
list_item("--db_host", "Database host.  Default: none (this host)");
list_item("--db_name", "Database name.  Default: PROJECT");
list_item("--db_user", "Database user.  Default: current user");
list_item("--db_passwd", "Database password.  Default: None");
list_item("--drop_db_first", "Drop database first (from prev installation)");

list_bar("debugging options");
list_item("--verbose={0,1,2}", "default: 1");
list_item("-v", "alias for --verbose=2");
list_item("-h or --help", "Show options");

list_end();
page_tail();
?>
