<?php
require_once("docutil.php");

page_head("Compound applications");

echo "
<h3>Compound applications</h3>

A <b>compound application</b> consists of a <b>main program</b>
and one or more <b>worker programs</b>.
The main program executes the worker programs in sequence,
and maintains a 'main state file' that records
which worker programs have completed.
The main program assigns to each worker application
a subrange of the overall 'fraction done' range of 0..1.
For example, if there are two subsidiary applications
with equal runtime,
the first would have range 0 to 0.5 and the second
would have range 0.5 to 1.


<p>
The BOINC API provides a number of functions,
and in developing a compound application you must decide
whether these functions are to be performed by
the main or worker program.
The functions are represented by flags in the BOINC_OPTIONS structure.
Each flag must be set in either the main or worker programs,
but not both.

<pre>
struct BOINC_OPTIONS {
	bool main_program;
	bool check_heartbeat;
	bool handle_trickle_ups;
	bool handle_trickle_downs;
	bool handle_process_control;
	bool handle send_status_msgs;
	bool direct_process_action;
};

int boinc_init_options(BOINC_OPTIONS&);
<pre>
";
list_start();
list_item("main_program",
	"Set this in the main program."
);
list_item("check_heartbeat",
	"If set, the program monitors 'heartbeat' messages from the core client.
	If the heartbeat stops, the result depends on the direct_process_action
	flag (see below)."
);
list_item("handle_trickle_ups",
	"If set, the program can send trickle-up messages."
);
list_item("handle_trickle_downs",
	"If set, the program can receive trickle-down messages."
);
list_item("handle_process_control",
	"If set, the program will handle 'suspend', 'resume', and 'quit'
	messages from the core client.
	The action depends on the direct_process_action flag."
);
list_item("send_status_msgs",
	"If set, the program will report its CPU time and fraction
	done to the core client.  Set in worker programs."
);
list_item("direct_process_action",
	"If set, the program will respond to quit messages and heartbeat
	failures by exiting, and will respond to suspend and resume
	messages by suspending and resuming.
	Otherwise, these events will result in changes to
	the BOINC_STATUS structure,
	which can be polled using boinc_get_status()."
);
<p>
Typical main program logic is:
<pre>
BOINC_OPTIONS options;

options.main_program = true;
...
boinc_init_options(options)
read main state file
for each remaining subsidiary application:
    aid.fraction_done_start = x
    aid.fraction_done_end = y
    boinc_write_init_data_file()
    run the app
	wait for the app to finish
		poll
    write main state file
	if last app:
		break
    boinc_parse_init_data_file()	// reads CPU time from app_init.xml file
boinc_finish()
</pre>
where x and y are the appropriate fraction done range limits.

<p>
Typical worker program logic is:
<pre>
BOINC_OPTIONS options;

options.main_program = false;
...
boinc_init_options(options);
...
do work, calling boinc_fraction_done() with values from 0 to 1,
and boinc_time_to_checkpoint(), occasionally
...
boinc_finish();		// this writes final CPU time to app_init.xml file

</pre>

<p>
If the graphics is handled in a program that runs concurrently with
the worker applications, it must also call
<code>boinc_init_options()</code>, typically with all options false,
then <code>boinc_init_graphics()</code>,
and eventually <code>boinc_finish()</code>.


";
page_tail();
?>
