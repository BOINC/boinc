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
    NOTE: if you are upgrading from a BOINC source tree
    in a different location than the original,
    you must manually edit the file
    boinc_path_config.py in the project's bin/ directory
    (and delete the file boinc_path_config.pyc).
    Change the values of TOP_BUILD_DIR and TOP_SOURCE_DIR
    to point to your new BOINC source tree.
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

