<?
   require_once("docutil.php");
   page_head("Development and debugging");

   echo "
<p>
The BOINC source code is <a href=source/>here</a>.
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
<p>
<a href=http://setiathome.berkeley.edu/taskbase>View database of bugs and feature requests</a>
<p>
<font size=+1><b>
Core client
</b></font>
<ul>
<li> <a href=client_files.php>File structure</a>
<li> <a href=client_fsm.php>FSM structure</a>
<li> <a href=client_data.php>Data structures</a>
<li> <a href=client_logic.php>Main loop logic</a>
<li> <a href=client_debug.php>Debugging</a>
<li> <a href=host_measure.php>Host measurement and identification</a>
<li> <a href=client_app.php>Core client/application interaction (basic)</a>
<li> <a href=client_app_graphic.php>Core client/application interaction (graphics)</a>
</ul>
<font size=+1><b>
Scheduling server
</b></font>
<ul>
<li> <a href=database.php>The BOINC database</a>
<li> <a href=sched_policy.php>Policy</a>
<li> <a href=sched_impl.php>Implementation</a>
<li> <a href=sched_debug.php>Debugging</a>
</ul>
<font size=+1><b>
Protocols
</b></font>
<ul>
<li> <a href=comm.php>Protocol overview</a>
<li> <a href=protocol.php>The scheduling server protocol</a>
<li> <a href=rpc_policy.php>Scheduling server timing and retry policies</a>
<li> <a href=upload.php>Data server protocol</a>
<li> <a href=pers_file_xfer.php>Persistent file transfers</a>
</ul>
<font size=+1><b>
Miscellaneous
</b></font>
<ul>
<li> <a href=python.php>Python framework</a>
<li> <a href=prefs_impl.php>Preferences</a>
</ul>

";
   page_tail();
?>

