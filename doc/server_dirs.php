<?php
require_once("docutil.php");
page_head("Server directory structure");
echo "

The directory structure for a typical BOINC project looks like:
<pre>
PROJECT/
    apps/
    bin/
    cgi-bin/
    log_HOSTNAME/
    pid/
    download/
    html/
        inc/
        ops/
        project/
        stats/
        user/
        user_profile/
    keys/
    upload/
</pre>
where PROJECT is the name of the project
and HOSTNAME is the server host.
Each project directory contains:
<ul>
<li>apps: application and core client executables
<li>bin: server daemons and programs.
<li>cgi-bin: CGI programs
<li>log_HOSTNAME: log output
<li>pid_HOSTNAME: lock files, pid files
<li> download: storage for data server downloads.
<li> html: PHP files for public and private web interfaces
<li> keys: encryption keys
<li> upload: storage for data server uploads.
</ul>

";
page_tail();
?>
