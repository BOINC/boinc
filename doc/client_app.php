<?php
require_once("docutil.php");
page_head("Core/app interaction (basic)");
echo "

TO BE WRITTEN.
<p>
Explain startup files
<p>
Explain shared memory mechanism in general
<p>
Explain work-related use of shmem


<p>
Application are executed in separate 'catbox' directories,
allowing them to create and use temporary files without name conflicts.
Input and output files are kept outside the catbox.
The mappings from virtual to physical filenames use
'symbolic link' files in the catbox directory.
The name of such a file is the virtual name,
and the file contains an XML tag with the physical name.
(This scheme is used because of the lack of filesystem links in Windows.)

<p>
Communication between the core client and applications
is done through XML files in the catbox directory.
Several files are used.
<p>
<b>Files created by the core client, read by the app:</b>
(Once, at start of app)
<ul>
<li> Symbolic link files

<li> <b>init_data.xml</b>: this contains the initialization data
returned by <tt>boinc_init()</tt> (see above),
as well as the minimum checkpoint period.
</ul>
<p>
<b>Files created by the API implementation, read by the core client:</b>
<ul>
<li>
<b>fraction_done.xml</b>:
contains the WU fraction done and the current CPU time from start of WU.
Written by the timer routine as needed.

<li>
<b>checkpoint_cpu.xml</b>
CPU time (from start of WU) at last checkpoint.
Written by checkpoint_completed.

</ul>
<p>
The API implementation uses a timer (60Hz);
the real-time clock is not available to applications.
This timer is used for several purposes:
<ul>
<li> To tell the app when to checkpoint;
<li> To regenerate the fraction done file
<li> To refresh graphics
</ul>

<p>
<b>Exit status</b>
The core client does a wait() to get the status.
boinc_finish() ends with an exit(status);
<p>
<b>Accounting of CPU time</b>:
(note: in Unix, a parent can't get the CPU time of a child
until the child exits.  So we're forced to measure it in the child.)
The core passes the WU CPU time in init_data.xml.
boinc_checkpoint_completed() and boinc_finish() compute the new WU CPU time,
and write it to checkpoint_cpu.xml.
The core deletes this after reading.
If on exit there is no checkpoint_cpu.xml, it means the app
called exit(0) rather than boinc_finish().
In this case the core measures the child CPU itself.
<p>
The core client maintains 
<p>
<b>Timing of checkpoints</b>
<p>
The app library maintains time_until_checkpoint,
decremented from the timer handler.
boinc_time_to_checkpoint() returns true if this is zero or less.
boinc_checkpoint_completed() resets it.

<p>
<b>Maintaining fraction done and current CPU</b>
<p>
These two quantities are transferred from the app library to
the core client in the file fraction_done.xml.
The parameter <tt>time_until_fraction_done_update</tt>,
passed in the initialization file,
determines how often this file is written.
It is written from the timer handler.
<p>
For multi-program applications, only the active application
must write the file.
The functions boinc_child_start() and boinc_child_done()
tell the app library to stop and start writing the file.
<p>
TO DO: this creates disk traffic.
Either figure out a way of increasing the period for users who don't
want disk access, or don't use disk files.
";
page_tail();
?>
