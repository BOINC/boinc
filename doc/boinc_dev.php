<?php
   require_once("docutil.php");
   page_head("Development and debugging");

   echo "
<p>
<h2>Source code</h2>
You can get the BOINC source code in several ways:
<ul>
<li>
<a href=source/>Download</a> a tarball or .zip file
(generated nightly).
<li>
Browse the CVS repository via a
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>web-based interface</a>.
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
<pre>
</ul>
<p>
Source code for a typical BOINC application,
SETI@home, is <a href=http://boinc.berkeley.edu/seti_source/>here</a>.
<p>
If you are an experienced C++ system programmer you may be able
to help us maintain and enhance BOINC.
In any case, you are welcome to browse the source code and
give us feedback.
<p>
You should understand exactly how BOINC is supposed to work
(for both <a href=participate.php>participants</a>
and <a href=create_project.php>developers</a>)
before getting into the source code.

<h2>Bug database</h2>
<p>
<a href=http://setiathome.berkeley.edu/taskbase>View database of bugs and feature requests</a>
<p>
<h2>Core client</h2>
<ul>
<li> <a href=client_files.php>File structure</a>
<li> <a href=client_fsm.php>FSM structure</a>
<li> <a href=client_data.php>Data structures</a>
<li> <a href=client_logic.php>Main loop logic</a>
<li> <a href=client_sched.php>Client scheduling policies</a>
<li> <a href=client_debug.php>Debugging</a>
<li> <a href=host_measure.php>Host measurements</a>
<li> <a href=host_id.php>Host identification</a>
<li> <a href=client_app.php>Core client/application interaction (basic)</a>
<li> <a href=client_app_graphic.php>Core client/application interaction (graphics)</a>
<li> <a href=disk_management.php>Disk space management</a>
</ul>
<h2>Server programs</h2>
<ul>
<li> <a href=database.php>The BOINC database</a>
<li> <a href=sched_policy.php>Work distribution policy</a>
<li> <a href=backend_state.php>Backend state transitions</a>
<li> <a href=backend_logic.php>The logic of backend programs</a>
<li> <a href=server_debug.php>Debugging server components</a>
</ul>

<h2>Protocols</h2>
<ul>
<li> <a href=comm.php>Protocol overview</a>
<li> <a href=protocol.php>The scheduling server protocol</a>
<li> <a href=rpc_policy.php>Scheduling server timing and retry policies</a>
<li> <a href=upload.php>Data server protocol</a>
<li> <a href=pers_file_xfer.php>Persistent file transfers</a>
</ul>
<h2>Miscellaneous</h2>
<ul>
<li> <a href=python.php>Python framework</a>
<li> <a href=prefs_impl.php>Preferences</a>
<li> <a href=trickle_impl.php>Trickle messages</a>
<li> <a href=version_diff.txt>How to see what has changed
between two versions of an executable</a>.
</ul>

";
   page_tail();
?>

