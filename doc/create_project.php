<?php
$book = $_GET['book'];

$chapters = array();

$count = 1;

function chapter($file, $title) {
    global $chapters;
    global $count;
    global $book;
    if ($book) {
        $chapters[] = $file;
        echo "<li> $count. <a href=#$file>$title</a>
        ";
        $count++;
    } else {
        echo "<li> <a href=$file>$title</a>
        ";
    }
}

require_once("docutil.php");
echo "
 <link rel=\"stylesheet\" type=\"text/css\" href=\"white.css\"/>
";

if ($book) {
    echo "
        <table width=800><tr><td>
        <img align=right src=logo/logo_small.png>
        <h1>Creating BOINC Projects</h1>
    ";
    echo gmdate("F d Y", time());
    echo "<br><br><font size=-2>";
    copyright();
    echo "</font><br><br>";
} else {
    page_head("Creating BOINC projects");
    echo "
        These pages are also available as
        <a href=create_project.php?book=1>one big HTML file</a>
        and as <a href=boinc.pdf>a PDF file</a>.
        <p>
        Several people have created
        <a href=other_docs.php>other documents about
        creating BOINC projects, and about BOINC in general</a>.
        <p>
    ";
}

echo "
<font size=+1><b>
Distributed computation with BOINC
</b></font>
<ul>
";
chapter("intro.php", "Overview of BOINC");
chapter("parallelize.php", "What applications are suitable for BOINC?");
echo "
<li> Basic concepts
<ul>
";
chapter("project.php", "Projects and applications");
chapter("files.php", "Files and file references");
chapter("platform.php", "Platforms");
chapter("app.php", "Applications and application versions");
chapter("work.php", "Workunits");
chapter("result.php", "Results");
echo "
</ul>
<li> Work distribution
<ul>
";
chapter("redundancy.php", "Redundancy and errors");
chapter("homogeneous_redundancy.php", "Numerical discrepancies");
chapter("work_distribution.php", "Work distribution");
chapter("sched_locality.php", "Locality scheduling");
echo "
</ul>
";
chapter("trickle.php", "Trickle messages");
echo "
<!--
<li><a href=batch.php>Batches</a>
<li><a href=sequence.php>Handling long, large-footprint computations</a>
<li><a href=file_access.php>Remote file access</a>
-->
";
chapter("security.php", "Security issues");
echo "

</ul>

<font size=+1><b>
Developing a BOINC application
</b></font>
<ul>
<li> The BOINC API
<ul>
";
chapter("api.php", "Basic API");
chapter("diagnostics.php", "Diagnostics API");
chapter("graphics.php", "Graphics API");
chapter("trickle_api.php", "Trickle messages API");
chapter("int_upload.php", "Intermediate upload API");
echo "
</ul>
<li> Application development
<ul>
";
chapter("compile_app.php", "Building BOINC applications");
chapter("example.php", "Example applications");
chapter("app_dev.php", "Application development tips");
chapter("app_debug.php", "Application debugging");
echo "
</ul>
";
chapter("fortran.php", "FORTRAN applications");
chapter("wrapper.php", "Legacy applications");
chapter("compound_app.php", "Compound applications");
echo "
</ul>

<font size=+1><b>
Creating a BOINC project
</b></font>
<ul>
";
chapter("server.php", "Setting up a BOINC server");
chapter("server_components.php", "What is a project?");
echo "
<ul>
";
chapter("database.php", "The BOINC database");
chapter("server_dirs.php", "Directory structure");
chapter("configuration.php", "The project configuration file");
echo "
    <ul
";
        chapter("project_options.php", "Project options");
        chapter("project_daemons.php", "Daemons");
        chapter("project_tasks.php", "Periodic tasks");
echo "
    </ul>
</ul>
<li> How to create a project
<ul>
";
chapter("make_project.php", "The make_project script");
chapter("tool_xadd.php", "Adding applications/platforms");
chapter("tool_update_versions.php", "Adding application versions");
chapter("project_cookbook.php", "Project creation cookbook");
chapter("tool_start.php", "Project control");
chapter("project_security.php", "Project security");
echo "
<ul>
";
chapter("code_signing.php", "Code signing");
echo "
</ul>
";
chapter("tool_upgrade.php", "Upgrading a project's server software");
chapter("multi_host.php", "Increasing server capacity");
chapter("beta.php", "Beta-test applications");
echo "

</ul>

<font size=+1><b>
Getting work done
</b></font>
<ul>
";
chapter("backend_programs.php", "Work-handling daemons");
chapter("tools_work.php", "Generating work");
chapter("validate.php", "Result validation");
chapter("assimilate.php", "Result assimilation");
chapter("file_deleter.php", "Server-side file deletion");
chapter("db_purge.php", "Database purging utility");
echo "
</ul>

<font size=+1><b>
Monitoring a BOINC project
</b></font>
<ul>
";
chapter("html_ops.php", "Administrative web interface");
chapter("server_debug.php", "Debugging server components");
chapter("log_rotate.php", "Log rotation");
echo "
";
//<li> <a href=watchdog.php", "Watchdogs</a>
//<li> <a href=stripchart.php", "Stripcharts: a tool for viewing time-varying data </a>
//<li> <a href=stripchart_data.php", "Recording data for Stripcharts</a>
echo "
</ul>

<font size=+1><b>
Managing distributed data
</b></font>
<ul>
";
chapter("get_file_list.php", "Uploading file lists");
chapter("get_file.php", "Uploading files");
chapter("send_file.php", "Downloading files");
chapter("delete_file.php", "Deleting files on client hosts");
echo "
</ul>

<font size=+1><b>
Web site
</b></font>
<ul>
";
chapter("web_config.php", "Web site overview");
chapter("forum.php", "Creating and managing message boards");
chapter("translation.php", "Web site translation");
chapter("sstatus.php", "Server status page");
chapter("profile_screen.php", "Profile screening");
chapter("web_cache.php", "Caching");
echo "
</ul>
<font size=+1><b>
Miscellaneous
</b></font>
<ul>
";
chapter("recruit.php", "Recruiting and retaining volunteers");
chapter("gui_urls.php", "GUI URLs");
chapter("project_skin.php", "Creating a 'project skin'");
chapter("db_dump.php", "Export credit data as XML");
chapter("grid.php", "Integrating BOINC projects with Grids");
chapter("boinc_version.php", "Versions of BOINC");
chapter("mysql_config.php", "Configuring MySQL for BOINC");
chapter("account_control.php", "Controlling account creation");
echo "
</ul>
";

$chap_num = 1;
echo "</td></tr></table>";
if ($book) {
    foreach ($chapters as $c) {
        echo "<a name=$c></a>";
        echo "<table width=800><tr><td>";
        require_once($c);
        echo "</td></tr></table>";
        $chap_num++;
    }
}
page_tail();
?>
