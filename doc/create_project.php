<?php
require_once("docutil.php");
page_head("Creating BOINC projects");

echo "
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
<li><a href=homogeneous_redundancy.php>Numerical discrepancies</a>
<li><a href=work_distribution.php>Work distribution</a>
<li><a href=sched_locality.php>Locality scheduling</a>
<li><a href=trickle.php>Trickle messages</a>
<li><a href=int_upload.php>Intermediate upload</a>
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
<li><a href=diagnostics.php>The BOINC diagnostics API</a>
<li><a href=graphics.php>The BOINC graphics API</a>
<li><a href=app_dev.php>Application development</a>
<li><a href=fortran.php>FORTRAN applications</a>
<li><a href=compound_app.php>Compound applications</a>
<li><a href=trickle_api.php>Trickle messages</a>
<li> <a href=app_debug.php>Application debugging</a>
<li><a href=myers.txt>App development cookbook</a> (courtesy of Eric Myers from Vassar)
</ul>

<font size=+1><b>
Creating a BOINC project
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
The components of a BOINC project, and how to create them.
</font>
<ul>
<li> <a href=compile.php>Compile BOINC software</a>
<li> <a href=project_cookbook.php>Project creation cookbook</a>
    <ul>
    <li> <a href=bashford_cookbook.txt>Another cookbook, from Don Bashford</a>
    </ul>
<li> <a href=server_components.php>What is a project?</a>
<li> <a href=database.php>The BOINC database</a>
<li> <a href=server_dirs.php>Directory structure</a>
<li> <a href=configuration.php>The project configuration file</a>
<li> <a href=make_project.php>The make_project script</a>
<li> <a href=tool_start.php>Project control</a>
<li> <a href=tool_xadd.php>Adding applications/platforms</a>
<li> <a href=code_signing.php>Code signing</a>
<li> <a href=tool_update_versions.php>Adding application versions</a>
<li> <a href=tool_upgrade.php>Upgrading a project's server software</a>

</ul>

<font size=+1><b>
Getting work done
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
How to generate tasks and handle the results.
</font>
<ul>
<li> <a href=backend_programs.php>Overview of daemons</a>
<li> <a href=tools_work.php>Generating work</a>
<li> <a href=validate.php>Result validation</a>
<li> <a href=assimilate.php>Result assimilation</a>
<li> <a href=file_deleter.php>Server-side file deletion</a>
<li> <a href=db_purge.php>Database purging utility</a>
</ul>

<font size=+1><b>
Monitoring a BOINC project
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
Tools for monitoring a BOINC project
</font>
<ul>
<li> <a href=html_ops.php>Administrative web interface</a>
<li> <a href=watchdog.php>Watchdogs</a>
<li> <a href=stripchart.php>Stripcharts: a tool for viewing time-varying data </a>
<li> <a href=stripchart_data.php>Recording data for Stripcharts</a>
</ul>

<font size=+1><b>
Managing distributed data
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
Using disk space on participant hosts
</font>
<ul>
<li> <a href=get_file_list.php>Uploading file lists</a>
<li> <a href=get_file.php>Uploading files</a>
<li> <a href=send_file.php>Downloading files</a>
<li> <a href=delete_file.php>Deleting files on client hosts</a>
</ul>

<font size=+1><b>
Web site
</b></font>
<br> &nbsp; &nbsp; &nbsp;
<font size=-1>
Communicating with participants via the web
</font>
<ul>
<li> <a href=web.php>Managing the project web site</a>
<li> <a href=gui_urls.php>Add project-specific links to the client GUI</a>
<li> <a href=translation.php>Web site translation</a>
</ul>
<font size=+1><b>
Miscellaneous
</b></font>
<ul>
<li> <a href=grid.php>Integrating BOINC projects with Grids</a>
<li> <a href=gui_urls.php>Add project-specific buttons to the client GUI</a>
</ul>
";
page_tail();
?>
