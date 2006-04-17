<?php
require_once("docutil.php");
page_head("Application debugging");
echo "
Some suggestions for debugging applications:

<h3>Standalone mode</h3>

When you have built your application
and linked it with the BOINC libraries,
you can run it in 'standalone mode' (without a BOINC core client present).
To do this, put instances of all input files in the same directory.
(with the proper logical, not physical, names).
The application should run and produce output files
(also with their logical names).
You can run the program under a debugger.

<p>
When you run an application in standalone mode,
the BOINC API will recognize this and take it into account.
A couple of things to note:
<ul>
<li> If your application does graphics, it will open a graphics window.
Closing this window will exit your application.
<li> boinc_time_to_checkpoint() will always return false,
so your application will never checkpoint.
</ul>


<h3>Using the anonymous platform mechanism</h3>

Once your application works in standalone mode
you'll want to run it from the BOINC core client.
This will exercise the various forms of interaction
with the core client.

<p>
To get this process started, create a test project,
add an application version and some work,
then run the core client.
It will download everything and run your application,
which will possibly crash.

<p>
At this point you'll want to start experimenting with your application.
It would be very tedious to create a new
application version for each change.
It's far easier to use BOINC's
<a href=anonymous_platform.php>anonymous platform</a> mechanism.
To do this:
<ul>
<li> Following the <a href=anonymous_platform.php>directions</a>,
create a file 'app_info.xml' in the client's project_* directory,
with the appropriate name and version number of your application.
<li> Each time your build a new version of your application,
copy the executable into the project_* directory,
making sure it has the appropriate name.
Then restart the core client.
</ul>

<p>
On Unix, it's possible to attach a debugger to a running process.
Use 'ps' to find the process ID of your application, then
something like
<pre>
gdb exec_filename PID
</pre>
to attach a debugger to it.

<h3>Getting and deciphering stack traces</h3>
<p>
Once your application is working on your own computers,
you're ready to test it with outside computers
(alpha testers initially).
It may crash on some computers, e.g. because their
software or hardware is different from yours.
You'll get some information back in the stderr_txt field
of the results.
If your application called boinc_init_diagnostics()
with the BOINC_DIAG_DUMPCALLSTACKENABLED flag set,
and you included symbols,
hopefully you'll get symbolic stack traces.
<p>
To decipher a Windows stack trace go <a href='app_debug_win.php'>here</a>.
<p>
Otherwise, you should at least get numeric (hex) stack traces.
You can decipher these by running a symbolic debugger
with an unstripped version and typing in the hex addresses.
See http://developer.apple.com/technotes/tn2004/tn2123.html#SECNOSYMBOLS
";

page_tail();
?>
