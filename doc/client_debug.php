<?php
require_once("docutil.php");
page_head("Core client: debugging");
echo "
<h3>Command-line options</h3> 
<p>
Some command-line options intended for debugging:
<dl>
<dt> -exit_when_idle 
<dd> Exit when we're not working on anything and a scheduling server
gives a 'no work' return.
<dt> -no_time_test
<dd> Don't run performance benchmarks; used fixed numbers instead.
<dt> -exit_after N
<dd> Exit after about N seconds
<dt> -giveup_after N
<dd> Give up on file transfers after N seconds (default is 2 weeks)
<dt> -limit_transfer_rate N
<dd> Limit total network traffic to N bytes/sec.
<dt> -min
<dd> Put client in the background after starting up
</dl>
<h3>Logging and error output</h3> 
<p>
The core client writes error messages to stderr.
This mechanism is reserved for serious problems,
i.e. those that reflect bugs in the core
client program or conditions that require user intervention.
<p>
In addition, the core client can write a variety of 'logging'
information to stdout.
The logging options are read from a file
<b>log_flags.xml</b>.
This file has the following format: 
<pre>
&lt;log_flags&gt;
    [ flags ]
&lt;/log_flags&gt;
</pre>
The flags are as follows: 
<dl>
<dt> &lt;task/&gt; 
<dd> Log the start, restart and completion of computational tasks. 
<dt> &lt;file_xfer/&gt; 
<dd> Log the start, restart and completion of file transfers. 
<dt> &lt;sched_ops/&gt; 
<dd> Log connections with scheduling servers. 
<dt> &lt;state_debug/&gt; 
<dd> Log changes to the 'client state' data structures. 
<dt> &lt;task_debug/&gt; 
<dd> Log debugging information about task execution. 
<dt> &lt;file_xfer_debug/&gt; 
<dd> Log debugging information about file transfers. 
<dt> &lt;sched_op_debug/&gt; 
<dd> Log the request and reply messages of exchanges with scheduling servers. 
<dt> &lt;time_debug/&gt; 
<dd> Log the passage of time. 
<dt> &lt;http_debug/&gt; 
<dd> Log debugging information about HTTP operations. 
<dt> &lt;net_xfer_debug/&gt; 
<dd> Log debugging information about network communication. 
<dt> &lt;measurement_debug/&gt; 
<dd> Log debugging information about measurements of CPU speed, platform, disk space, etc. 
<dt> &lt;guirpc_debug/&gt; 
<dd> Log debugging information about the GUI RPC interface. 
<dt> &lt;sched_cpu_debug/&gt; 
<dd> Log debugging information about the CPU scheduler. 
<dt> &lt;scrsave_debug/&gt; 
<dd> Log debugging information about the screen saver. 
</dl>
";
page_tail();
?>
