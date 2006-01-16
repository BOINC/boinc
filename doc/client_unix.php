<?php
require_once("docutil.php");
page_head("Core client command-line options");
echo "
<p>
The core client has
command-line options that provide minimal control
(e.g. the ability attach and detach projects).
<p>
More detailed control, and the ability to interact
with a running client, is provided by the
<a href=boinc_cmd.php>BOINC command tool</a>.
<p>
<h3>Command-line options</h3>
";
list_start();
list_item("-help",
    "Show client options."
);
list_item("-version",
    "Show client version."
);
list_item("<nobr>-attach_project URL account_key</nobr>",
    "Attach this computer to a new project."
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
list_item("-gui_rpc_port N",
    "Specify port for GUI RPCs"
);
list_item("-allow_remote_gui_rpc",
    "Allow GUI RPCs from remote hosts"
);
list_item("-dir abs_path",
    "Use the given directory as BOINC home"
);
list_item("-detach",
    "Detach from console (Windows only)"
);
list_item("-no_gui_rpc",
    "Don't allow GUI RPCs."
);
list_item("-daemon",
    "Run as daemon (detach from controlling terminal; Linux only)
);
list_end();
echo "
<h2>Environment variables</h2>
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
echo "
<h2>Command-line options for debugging</h2>
";
list_start();
list_item(" -exit_when_idle ",
    " Exit when we're not working on anything and a scheduling server
gives a 'no work' return."
);
list_item(" -no_time_test",
    " Don't run performance benchmarks; used fixed numbers instead."
);
list_item(" -exit_after N",
    " Exit after about N seconds"
);
list_item(" -giveup_after N",
    " Give up on file transfers after N seconds (default is 2 weeks)"
);
list_item(" -limit_transfer_rate N",
    " Limit total network traffic to N bytes/sec."
);
list_item(" -min",
    " Put client in the background after starting up"
);
list_end();
page_tail();
?>
