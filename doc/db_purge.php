<?php
require_once("docutil.php");
page_head("Database purging utility");
echo "
As a BOINC project operates, the size of its
workunit result tables increases,
and eventually they become inconveniently large
(for example, adding a field or building an index may take hours or days).

<p>
To address this problem, BOINC provides a utility <b>db_purge</b>
that 'purges' result and WU records by writing them
to XML-format archive files, then deleting them from the database.

<p>
Workunits are purged only when their input files have been deleted.
Because of BOINC's file-deletion policy,
this implies that all results are completed.
So when a workunit is purged, all its results are purged too.

<p>
The archive files have names of the form
wu_archive_TIME and result_archive_TIME
where TIME is the Unix time the file was created.
In addition, db_purge generates index files
'wu_index' and 'result_index'
associating each WU and result ID with the timestamp of its archive file.

<p>
Run db_purge from the project's bin/ directory.
It will create an archive/ directory and store archive files there.

<p>
db_purge is normally run as a daemon,
specified in the <a href=configuration.php>config.xml</a> file.
It has the following command-line options:
";
list_start();
list_item("-min_age_days N",
    "Purge only WUs with mod_time at least N days in the past.
    Recommended value: 7 or so.
    This lets users examine their recent results."
);
list_item("-max N", "Purge at most N WUs, then exit");
list_item("-max_wu_per_file N",
    "Write at most N WUs to each archive file.
    Recommended value: 10,000 or so."
);
list_item("-zip", "Compress archive files using zip");
list_item("-gzip", "Compress archive files using gzip");
list_item("-d N", "Set logging verbosity to N (1,2,3)");
list_end();

echo "
";
page_tail();
?>
