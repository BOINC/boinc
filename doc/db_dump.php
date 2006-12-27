<?php
require_once("docutil.php");
page_head("Exporting credit statistics with db_dump");
echo "
The program db_dump generates XML files containing
your project's credit data.
It should be run every 24 hours.
Include an entry like
".html_text("
    <tasks>
        <task>
            <cmd>db_dump -d 2 -dump_spec ../db_dump_spec.xml</cmd>
            <output>db_dump.out</output>
            <period>24 hours</period>
        </task>
    </tasks>
")."
in your config.xml file.
Make sure the file db_dump_spec.xml is in your project's root directory.
<p>
The XML files are written to html/stats/,
and the old stats/ directory is renamed to stats_DATE.
This clutters up your html/ directory;
if you don't like this, create a directory html/stats_archive/
and add the line
".html_text("
<archive_dir>../html/stats_archive</archive_dir>
")."
to your db_dump_spec.xml file.
";
page_tail();
?>
