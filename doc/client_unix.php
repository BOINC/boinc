<?php
require_once("docutil.php");
page_head("Installing and running the BOINC command-line client");
echo "
<h3>The BOINC command-line client</h3>
<p>
Install the BOINC client by using gunzip to decompress the application.
Use 'chmod' to make it executable.
Put it in a directory by itself.
Run it manually, from your login script,
or from system startup files.
<p>
A good set of instructions for running BOINC on Unix systems is
<a href=http://noether.vassar.edu/~myers/help/boinc/unix.html>here</a>.
<p>
The command-line client has the following command-line options:
";
list_start();
list_item("-attach_project",
    "Attach this computer to a new project.
    You must have an account with that project.
    You will be asked for the project URL and the account key."
);
list_item("-show_projects",
    "Print a list of projects to which this computer is attached."
);

list_item("-detach_project URL",
    "Detach this computer from a project."
);

list_item("-reset_project URL",
    "Clear pending work for a project.
    Use this if there is a problem that is preventing
    your computer from working."
);

list_item("-update_prefs URL",
    "Contact a project's server to obtain new preferences.
    This will also report completed results
    and get new work if needed."
);

list_item("-return_results_immediately",
    "Contact scheduler as soon as any result done."
);
list_item("-run_cpu_benchmarks",
    "Run CPU benchmarks.
    Do this if you have modified your computer's hardware."
);
list_item("-check_all_logins",
    "If 'run if user active' preference is off,
    check for input activity on all current logins;
    default is to check only local mouse/keyboard"
);
list_item("-exit_when_idle",
    "Get, process and report work, then exit."
);
list_item("-allow_remote_gui_rpc",
    "Allow GUI RPCs from remote hosts"
);
list_item("-help",
    "Show client options."
);

list_item("-version",
    "Show client version."
);
list_end();
echo"
The command-line client has the following optional environment variables:
";
list_start();
list_item("HTTP_PROXY", "URL of HTTP proxy");
list_item("HTTP_USER_NAME", "User name for proxy authentication");
list_item("HTTP_USER_PASSWD", "Password for proxy authentication");
list_item("SOCKS4_SERVER", "URL of SOCKS 4 server");
list_item("SOCKS5_SERVER", "URL of SOCKS 5 server");
list_item("SOCKS5_USER", "User name for SOCKS authentication");
list_item("SOCKS5_PASSWD", "Password for SOCKS authentication");
list_end();
page_tail();
?>
