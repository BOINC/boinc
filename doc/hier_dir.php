<?php

require_once("docutil.php");

page_head("Hierarchical upload/download directories");

echo "
The data server for a large project,
may store 100Ks or millions of files at any given point.
If these files are stored in 'flat' directories
(project/download and project/upload)
the data server may spend a lot of CPU time searching directories.
If you see a high CPU load average,
with a lot of time in kernel mode,
this is probably what's happening.

<p>
The solution is to use
<b>hierarchical upload/download directories</b>.
To do this, include the line
".html_text("
<uldl_dir_fanout>1024</uldl_dir_fanout>
")."
in your <a href=configuration.php>config.xml file</a>
(this is the default for new projects).

<p>
This causes BOINC to use hierarchical upload/download directories.
Each directory will have a set of 1024 subdirectories, named 0 to 3ff.
Files are hashed (based on their filename) into these directories.

<p>
The hierarchy is used for input and output files only.
Executables and other application version files are
in the top level of the download directory.

<p>
This affects your project-specific code in a couple of places.
First, your work generator must put input files in
the right directory before calling <a href=tools_work.php>create_work()</a>.
To do this, it can use the function
".html_text("
int dir_hier_path(
    const char* filename, const char* root, int fanout, bool newhash, char* result, bool make_directory_if_needed
);
")."
This takes a name of the input file
and the absolute path of the root of the download hierarchy
(typically the download_dir element from config.xml)
and returns the absolute path of the file in the hierarchy.
For new projects, newhash should be set to true.  This argument may eventually
disappear: it was added to work around a poor initial choice of hashing
function for determining fanout directory names.  Generally make_directory_if_needed should also be set to true: this creates a fanout directory if needed
to accomodate a particular file.

Note: you may see this function occuring at times with the final argument
missing.  In this case C++ defaults to setting make_directory_if_needed==false.
[DAVID: PLEASE CONFIRM!]

<p>
Secondly, your validator and assimilator should call
".html_text("
int get_output_file_path(RESULT const& result, string& path);
")."
to get the paths of output files in the hiearchy.
If your application has multiple output files,
you'll need to generalize this function.

<p>
A couple of utility programs are available:
".html_text("
dir_hier_move src_dir dst_dir fanout
dir_hier_path filename
")."
<code>dir_hier_move</code> moves all files from src_dir (flat)
into dst_dir (hierarchical with the given fanout).
<code>dir_hier_path</code>, given a filename,
prints the full pathname of that file in the hierarchy.


<h2>Transitioning from flat to hierarchical directories</h2>
<p>
If you are operating a project with flat directories,
you can transition to a hierarchy as follows:
<ul>
<li> Stop the project and add &lt;uldl_dir_fanout> to config.xml.
You may want to locate the hierarchy root at a new place
(e.g. download/fanout); in this case update the
&lt;download_dir> element of config.xml,
and add the element
".html_text("
<download_dir_alt>old download dir</download_dir_alt>
")."
This causes the file deleter to check both old and new locations.

<li> Use dir_hier_move to move existing upload files to a hierarchy.
<li> Start the project, and monitor everything closely for a while.
</ul>
";

page_tail();
?>
