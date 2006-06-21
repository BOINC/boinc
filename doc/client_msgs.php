<?php
require_once("docutil.php");
page_head("Core client configuration");
echo "
The core client reads configuration options from a file
<b>cc_config.xml</b>.
This file has the following format: 
".html_text("
<cc_config>
    [ <save_stats_days>N</save_stats_days> ]
    [ <dont_check_file_size>0|1</dont_check_file_size> ]
    [ <ncpus>N</ncpus> ]
    <log_flags>
        [ flags ]
    </log_flags>
</cc_config>
")."
";
list_start();
list_item_func("<save_stats_days>",
    "How many days to save the per-project credit totals
    that are displayed in the Statistics tab of the BOINC Manager.
    Default is 30."
);
list_item_func("<dont_check_file_sizes>",
    "If nonzero, don't check file sizes"
);
list_item_func("<ncpus>",
    "Act as if there were N CPUs (for debugging)."
);
list_end();
echo "
The core client generates messages:
informational messages saying what it's doing,
and error messages when it encounters problems.
If you run the BOINC manager, these appear in the Messages tab -
informational messages in black, error messages in red.
On Unix, informational messages are written to standard output,
and error messages are written to standard error.
<p>
These messages can be turned on and off with the following tags
(&lt;tag>0&lt;/tag> for off, &lt;tag>1&lt;/tag> for on):
";
list_start();
list_item_func("<task>",
    "Log the start, restart and completion of computational tasks."
);
list_item_func("<file_xfer>",
    " Log the start, restart and completion of file transfers. "
);
list_item_func("<sched_ops>",
    " Log connections with scheduling servers. "
);
list_item_func("<state_debug>",
    " Log changes to the 'client state' data structures. "
);
list_item_func("<task_debug>",
    " Log debugging information about task execution. "
);
list_item_func("<file_xfer_debug>",
    " Log debugging information about file transfers. "
);
list_item_func("<sched_op_debug>",
    " Log the request and reply messages of exchanges with scheduling servers. "
);
list_item_func("<http_debug>",
    " Log debugging information about HTTP operations. "
);
list_item_func("<proxy_debug>",
    " Log debugging information about HTTP proxy operations. "
);
list_item_func("<time_debug>",
    " Log the passage of time. "
);
list_item_func("<net_xfer_debug>",
    " Log debugging information about network communication. "
);
list_item_func("<measurement_debug>",
    " Log debugging information about measurements of CPU speed, platform, disk space, etc. "
);
list_item_func("<poll_debug>",
    " Show what poll functions do"
);
list_item_func("<guirpc_debug>",
    " Log debugging information about the GUI RPC interface. "
);
list_item_func("<cpu_sched_debug>",
    " Log debugging information about the CPU scheduler. "
);
list_item_func("<scrsave_debug>",
    " Log debugging information about the screen saver. "
);
list_end();

page_tail();
?>
