<?php
require_once("docutil.php");
page_head("Core client configuration");
echo "
The BOINC client can be <b>configured</b> to
<ul>
<li> Produce more detailed log messages.
These messages appear in the Messages tab of the BOINC Manager
(informational messages in black, error messages in red).
On Windows, they are written to a file (stdoutdae.txt).
On Unix, they are written to standard output.
<li> Control various behavioral parameters, such as how many
simultaneous file transfers to allow.
</ul>
These options are useful primarily for testing and debugging.
<p>
The configuration is read from a file <b>cc_config.xml</b>.
If this file is absent, the default configuration is used.
This file has the following format: 
".html_text("
<cc_config>
    <log_flags>
        [ flags ]
    </log_flags>
    <options>
        [ <save_stats_days>N</save_stats_days> ]
        [ <dont_check_file_sizes>0|1</dont_check_file_sizes> ]
        [ <http_1_0>0|1</http_1_0> ]
        [ <ncpus>N</ncpus> ]
        [ <max_file_xfers>N</max_file_xfers> ]
        [ <max_file_xfers_per_project>N</max_file_xfers_per_project> ]
        [ <work_request_factor>X</work_request_factor> ]
    </options>
</cc_config>
")."

For example, if you want to see messages about CPU scheduling,
use a text editor (such as Notepad) to create the following file,
and save it as cc_config.xml in your BOINC directory.
".html_text("
<cc_config>
    <log_flags>
        <cpu_sched>1</cpu_sched>
    </log_flags>
</cc_config>
")."
<h2>Logging flags</h2>
The flags within &lt;log_flags&gt; are used to
selectively turn different types of messages on and off 
(&lt;tag>0&lt;/tag> for off, &lt;tag>1&lt;/tag> for on):
<p>
The following messages are enabled by default:
";
list_start();
list_item_func("<task>",
    "The start and completion of compute jobs
    (should get two messages per job)."
);
list_item_func("<file_xfer>",
    "The start and completion of file transfers."
);
list_item_func("<sched_ops>",
    "Connections with scheduling servers."
);
list_end();
echo "
The following messages are disabled by default
(typically they generate lots of output,
and are used for specific debugging purposes):
";
list_start();
list_item_func("<cpu_sched>",
    "CPU scheduler actions (preemption and resumption)"
);
list_item_func("<cpu_sched_debug>",
    "Explain CPU scheduler decisions"
);
list_item_func("<rr_simulation>",
    "Results of the round-robin simulation used by CPU scheduler and work-fetch"
);
list_item_func("<debt_debug>",
    "Changes to project debt"
);
list_item_func("<task_debug>",
    "Low-level details of process start/end (status codes, PIDs etc.),
    and when applications checkpoint."
);
list_item_func("<work_fetch_debug>",
    "Work fetch policy decisions"
);
list_item_func("<unparsed_xml>",
    "Show any unparsed XML"
);
list_item_func("<state_debug>",
    "Show summary of client state after scheduler RPC and garbage collection;
    also show garbage collection actions, and when state file is read/written."
);
list_item_func("<file_xfer_debug>",
    "Show completion status of file transfers"
);
list_item_func("<sched_op_debug>",
    "Details of scheduler RPCs"
);
list_item_func("<http_debug>",
    "Debugging information about HTTP operations"
);
list_item_func("<proxy_debug>",
    "Debugging information about HTTP proxy operations"
);
list_item_func("<time_debug>",
    "Updates to on_frac, active_frac, connected_frac."
);
list_item_func("<http_xfer_debug>",
    "Debugging information about network communication"
);
list_item_func("<benchmark_debug>",
    "Debugging information about CPU benchmarks"
);
list_item_func("<poll_debug>",
    "Show what poll functions do"
);
list_item_func("<guirpc_debug>",
    "Debugging information about GUI RPC operations"
);
list_item_func("<scrsave_debug>",
    "Debugging information about the screen saver."
);
list_item_func("<app_msg_send>",
    "Shared-memory messages sent to applications."
);
list_item_func("<app_msg_receive>",
    "Shared-memory messages received fromapplications."
);
list_item_func("<mem_usage_debug>",
    "Application memory usage."
);
list_item_func("<network_status_debug>",
    "Network status (whether need physical connection)."
);
list_item_func("<checkpoint_debug>",
    "Show when applications checkpoint"
);
list_end();
echo "
<h2>Behavioral parameters</h2>
The following options control the behavior of BOINC:
";
list_start();
list_item_func("<save_stats_days>",
    "How many days to save the per-project credit totals
    that are displayed in the Statistics tab of the BOINC Manager.
    Default is 30."
);
list_item_func("<dont_check_file_sizes>",
    "Normally, the size of application and input files
    are compared with the project-supplied values
    after the files are downloaded,
    and just before starting an application.
    If this flag is set, this check is skipped.
    Use it if you need to modify files locally for some reason."
); 
list_item_func("<http_1_0>",
    "Set this flag to use HTTP 1.0 instead of 1.1
    (this may be needed with some proxies)."
);
list_item_func("<ncpus>",
    "Act as if there were N CPUs: run N tasks at once.
    This is for debugging, i.e. to simulate 2 CPUs
    on a machine that has only 1.
    Don't use it to limit the number of CPUs used by BOINC;
    use general preferences instead."
);
list_item_func("<max_file_xfers>",
    "Maximum number of simultaneous file transfers (default 8)."
);
list_item_func("<max_file_xfers_per_project>",
    "Maximum number of simultaneous file transfers per project (default 2)."
);
list_item_func("<work_request_factor>",
    "The amount of work requested from projects will by multiplied
    by this number.
    Use a number larger than one if your computer often
    runs out of work."
);
list_end();

page_tail();
?>
