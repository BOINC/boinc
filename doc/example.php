<?php
require_once("docutil.php");

page_head("Example application");

echo "
A full-featured example BOINC application.
in in a separate CVS module, <b>boinc_samples</b>
(check this out in the same parent directory as boinc,
and the relative paths will work).
The application is called 'upper_case'.

<p>
The application does things (like checkpointing and graphics)
that can be tricky or confusing.
You can use it as a template for your own BOINC application;
just rip out the computation part
(which is trivial) and replace it with your code.

<p>
The boinc_samples tree includes project files for
building the app on Windows (VS 2003) and Mac (xcode).
It also includes a Makefile for building on Linux.
This produces a separate .so containing the graphics part.
The Makefile links some libraries statically
(stdc++, glut etc.) so that the app will run on machines
where these are old or missing.
It also uses a technique where the main program
exports its symbols to the graphics .so,
which eliminates lots of the problems we had in Einstein@home.

<p>
The graphics show a bouncing 3D ball.
If you put files Helvetica.txf and logo.jpg in the directory
where it runs, you'll also see an image and some nice-looking text
(thanks to Tolu Aina for the latter).
";

page_tail();
?>
