<?php
require_once("docutil.php");
page_head("Creating BOINC projects");

echo "
<font size=+1><b>
Designing distributed computations with BOINC
</b></font>
<ul>
<li> <a href=intro.php>Overview of BOINC</a>
<li><a href=parallelize.php>What applications are suitable for BOINC?</a>
<li> Basic concepts
<ul>
<li><a href=project.php>Projects and applications</a>
<li><a href=files.php>Files and file references</a>
<li><a href=platform.php>Platforms</a>
<li><a href=app.php>Applications and application versions</a>
<li><a href=work.php>Workunits</a>
<li><a href=result.php>Results</a>
</ul>
<li> Work distribution
<ul>
<li><a href=redundancy.php>Redundancy and errors</a>
<li><a href=homogeneous_redundancy.php>Numerical discrepancies</a>
<li><a href=work_distribution.php>Work distribution</a>
<li><a href=sched_locality.php>Locality scheduling</a>
</ul>
<li><a href=trickle.php>Trickle messages</a>
<!--
<li><a href=batch.php>Batches</a>
<li><a href=sequence.php>Handling long, large-footprint computations</a>
<li><a href=file_access.php>Remote file access</a>
-->
<li><a href=security.php>Security issues</a>

</ul>

<font size=+1><b>
Developing a BOINC application
</b></font>
<ul>
<li> The BOINC API
<ul>
<li><a href=api.php>Basic API</a>
<li><a href=diagnostics.php>Diagnostics API</a>
<li><a href=graphics.php>Graphics API</a>
<li><a href=trickle_api.php>Trickle messages API</a>
<li><a href=int_upload.php>Intermediate upload API</a>
</ul>
<li> Application development
<ul>
<li><a href=example.php>Example application</a>
<li><a href=app_dev.php>Application development tips</a>
<li> <a href=app_debug.php>Application debugging</a>
</ul>
<li><a href=fortran.php>FORTRAN applications</a>
<li><a href=wrapper.php>Legacy applications</a>
<li><a href=compound_app.php>Compound applications</a>
</ul>

<font size=+1><b>
Creating a BOINC project
</b></font>
<ul>
<li> <a href=server_components.php>What is a project?</a>
<ul>
<li> <a href=database.php>The BOINC database</a>
<li> <a href=server_dirs.php>Directory structure</a>
<li> <a href=configuration.php>The project configuration file</a>
</ul>
<li> <a href=compile.php>Compile BOINC software</a>
<li> <a href=groups.php>Groups and permissions</a>
<li> How to create a project
<ul>
<li> <a href=make_project.php>The make_project script</a>
<li> <a href=tool_xadd.php>Adding applications/platforms</a>
<li> <a href=tool_update_versions.php>Adding application versions</a>
<li> <a href=project_cookbook.php>Project creation cookbook</a>
<li> <a href=http://j4cques.blogspot.com/>Another cookbook, from Jacques Fontignie</a>
<li> <a href=bashford_cookbook.txt>Another cookbook, from Don Bashford</a>
</ul>
<li> <a href=tool_start.php>Project control</a>
<li> <a href=project_security.php>Project security</a>
<ul>
<li> <a href=code_signing.php>Code signing</a>
</ul>
<li> <a href=tool_upgrade.php>Upgrading a project's server software</a>

</ul>

<font size=+1><b>
Getting work done
</b></font>
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
<ul>
<li> <a href=html_ops.php>Administrative web interface</a>
<li> <a href=server_debug.php>Debugging server components</a>
<li> <a href=watchdog.php>Watchdogs</a>
<li> <a href=stripchart.php>Stripcharts: a tool for viewing time-varying data </a>
<li> <a href=stripchart_data.php>Recording data for Stripcharts</a>
</ul>

<font size=+1><b>
Managing distributed data
</b></font>
<ul>
<li> <a href=get_file_list.php>Uploading file lists</a>
<li> <a href=get_file.php>Uploading files</a>
<li> <a href=send_file.php>Downloading files</a>
<li> <a href=delete_file.php>Deleting files on client hosts</a>
</ul>

<font size=+1><b>
Web site
</b></font>
<ul>
<li> <a href=web_config.php>Web site overview</a>
<li> <a href=forum.php>Creating and managing message boards</a>
<li> <a href=translation.php>Web site translation</a>
<li> <a href=sstatus.php>Server status page</a>
</ul>
<font size=+1><b>
Miscellaneous
</b></font>
<ul>
<li> <a href=mass_email.php>Sending mass emails</a>
<li> <a href=gui_urls.php>Project-specific items in the client GUI</a>
<li> <a href=db_dump.php#task>Export credit data as XML</a>
<li> <a href=grid.php>Integrating BOINC projects with Grids</a>
<li> <a href=boinc_version.php>Versions of BOINC</a>
<li> <a href=mysql_config.php>Configuring MySQL for BOINC</a>
<li> <a href=account_control.php>Controlling account creation</a>
</ul>
";
page_tail();
?>
