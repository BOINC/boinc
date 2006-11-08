<?php
require_once("docutil.php");

page_head("Example applications");

echo "
BOINC provides several example applications.
See the
<a href=compile_app.php>instructions for building BOINC applications</a>.
<p>

The example applications are:
<ul>
<li>
<b>upper_case</b>:
a full-featured example BOINC application.
The application does things (like checkpointing and graphics)
that can be tricky or confusing.
You can use it as a template for your own BOINC application;
just rip out the computation part
(which is trivial) and replace it with your code.
<p>
The graphics show a bouncing 3D ball.
If you copy the files boinc/txf/Helvetica.txf and logo.jpg in the directory
where it runs, you'll also see an image and some nice-looking text
(thanks to Tolu Aina for the latter).
<li>
<b>wrapper</b>: used to support <a href=wrapper.php>legacy applications</a>.
<li>
<b>worker</b>: a representative legacy application
(i.e. it doesn't use the BOINC API or runtime library).
Used for testing wrapper.
<li>
<b>sleeper</b>: test application for non-CPU-intensive projects
(used for testing the BOINC core client).

</ul>

<p>
";

page_tail();
?>
