<?php
require_once("docutil.php");
page_head("Server directory structure");
echo "

The directory structure for a typical BOINC project looks like:
<pre>
PROJECT/
    bin/
    cgi-bin/
    log_HOSTNAME/
    pid/
    download/
    html/
        user/
        ops/
    keys/
    upload/
</pre>
where PROJECT is the name of the project
and HOSTNAME is the server host.
Each project directory contains:
<ul>
<li>bin: server daemons and programs.
<li>cgi-bin: CGI programs
<li>log_HOSTNAME: log output, lock files, pid files
<li> download: storage for data server downloads.
<li> html/ops: PHP files for project management.
<li> html/user: PHP files for the public web site.
<li> keys: encryption keys
<li> upload: storage for data server uploads.
</ul>

";
page_tail();
?>
