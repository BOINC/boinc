<?php
require_once("docutil.php");
page_head("Server-side file deletion");
echo "
Files are deleted from the data server's upload and download directories
by the <b>file_deleter</b> daemon.
Typically you don't need to customize this.
The default file deletion policy is:
<ul>
<li> A workunit's input files are deleted when
all results are 'over' (reported or timed out)
and the workunit is assimilated.
<li> A result's output files are deleted
after the workunit is assimilated.
The canonical result is handled differently,
since its output files may be needed to validate
results that are reported after assimilation;
hence its files are deleted only when all results are over,
and all successful results have been validated.
</ul>

Command-line options are:
";
list_start();
list_item(
    "-preserve_wu_files",
    "Don't delete input files"
);
list_item(
    "-preserve_result_files",
    "Don't delete output files"
);
list_item(
    "-retry_errors",
    "Retry file deletions that failed previously."
);
list_end();
echo "

<p>
In some cases you may not want files to be deleted.
There are three ways to accomplish this:
<ul>
<li> Run the file_deleter daemon with
the -preserve_wu_files
and/or the -preserve_result_files command-line options.
<li> Include &lt;no_delete/>
in the <a href=files.php>&lt;file_info></a> element for a file in a
<a href=tools_work.php>workunit or result template</a>.
This lets you suppress deletion on a file-by-file basis.

<li> Include <code>nodelete</code> in the workunit name.

</ul>
You may need to implement your own scheme for deleting files,
to avoid overflowing data server storage.
";
page_tail();
?>
