<?php
   require_once("docutil.php");
   page_head("Upgrading project software");

echo "
    The script <b>bin/upgrade</b> copies files from
    a BOINC source tree to a project tree.
    This is typically done after updating the source tree
    and recompiling the software.
    The script must reside in the project's bin/ directory,
    and you must run it from the source tree root.
    <p>
    <b>upgrade</b> does the following:
    <ul>
    <li>
    It copies files from the source/html/* directories
    to the corresponding project directories.
    <li>
    It copies source/sched/cgi and source/sched/file_upload_handler
    to projects/cgi-bin.
    <li>
    It copies other programs in source/sched to project/bin.
    <li>
    It copies programs in source/tools to project/bin.
    </ul>

";
   page_tail();
?>

