<? // -*-html -*-
require_once("docutil.php");
page_head("Creating a BOINC project");
?>

<font size=+1><b>
Designing distributed computations with BOINC
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
BOINC's abstractions of data and computation.
</font>
<ul>
<li><a href=project.php>Projects and applications</a>
<li><a href=parallelize.php>What applications are suitable for BOINC?</a>
<li><a href=files.php>Files and file references</a>
<li><a href=platform.php>Platforms</a>
<li><a href=app.php>Applications and application versions</a>
<li><a href=work.php>Workunits</a>
<li><a href=result.php>Results</a>
<li><a href=redundancy.php>Redundancy and errors</a>
<!--
<li><a href=batch.php>Batches</a>
<li><a href=sequence.php>Handling long, large-footprint computations</a>
<li><a href=file_access.php>Remote file access</a>
-->
<li><a href=security.php>Security</a>
<li><a href=boinc_version.php>Versions of BOINC</a>

</ul>

<font size=+1><b>
Developing a BOINC application
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
How to develop or port an application program for use with BOINC.
</font>
<ul>
<li><a href=api.php>The BOINC API</a>
<li><a href=graphics.php>The BOINC graphics API</a>
<li> <a href=tools_other.php>Tools for creating applications and versions</a>
<li><a href=dev.php>The application development process</a>
</ul>
<font size=+1><b>
Setting up a BOINC server
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
How to install and configure the BOINC server software.
</font>
<ul>
<li> <a href=road_map.php>Road map of the BOINC software</a>
<li> <a href=server_components.php>Server components</a>
<li> <a href=build_server.php>Building server components</a> (<a href=build_system.php>Build system</a>)
<ul>
  <li> <a href=build_client_unix.php>Building the core client for Unix</a>
  <li> <a href=build_client_win.php>Building the core client for Windows</a>
  <li> <a href=build_client_mac.php>Building the core client for Macintosh</a>
</ul>
<li> <a href=single_host_server.php>Setting up a single-host server</a>
<li> <a href=database_setup.php>Setting up the BOINC database</a>
<li> <a href=key_setup.php>Creating encryption keys</a>
<li> <a href=sched_server_setup.php>Setting up a scheduling server</a>
<li> <a href=web_site.php>Setting up a web site</a>
<li> <a href=data_server_setup.php>Setting up a data server</a>
<li> <a href=test.php>Test applications and scripts</a>
<li> <a href=project_startup.php>Project startup checklist</a>

</ul>

<font size=+1><b>
Developing and operating a BOINC project back end
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
How to feed work into a BOINC system and collect the results.
</font>
<ul>
<li> <a href=backend_functions.php>Back end functions</a>
<li> <a href=backend_state.php>Back end state transitions</a>
<li> <a href=backend_programs.php>Back end programs</a>
<li> <a href=tools_work.php>Generating work</a>
<!--
<li> <a href=backend_work_sequence.php>Back ends and work sequences</a>
-->
<li> <a href=back_end.php>Back end examples</a>
</ul>

<font size=+1><b>
Monitoring the performance of a BOINC project
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
Tools for monitoring the system as a whole.
</font>
<ul>
<li> <a href=watchdog.php>Watchdogs</a>
<li> <a href=stripchart.php>Stripcharts: a tool for viewing time-varying data </a>
<li> <a href=stripchart_data.php>Recording data for Stripcharts</a>
</ul>
<?
page_tail();
?>
