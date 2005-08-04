<?php
require_once("docutil.php");
page_head("Database purging utility");
echo "
As a BOINC project operates, the size of its
workunit and result tables increases.
Eventually they become so large
that adding a field or building an index may take hours or days.

<p>
To address this problem, BOINC provides a utility <b>db_purge</b>
that writes result and WU records to XML-format archive files,
then deletes them from the database.

<p>
Workunits are purged only when their input files have been deleted.
Because of BOINC's file-deletion policy,
this implies that all results are completed.
So when a workunit is purged, all its results are purged too.

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
<h3>Archive file format</h3>
<p>
The archive files have names of the form
wu_archive_TIME and result_archive_TIME
where TIME is the Unix time the file was created.
In addition, db_purge generates index files
'wu_index' and 'result_index'
associating each WU and result ID with the timestamp of its archive file.
<p>
The format of both type of index files is a number of rows each containing:
<pre>
ID     TIME
</pre>
The ID field of the WU or result, 5 spaces,
and the timestamp part of the archive filename where the record with that
ID can be found.
<p>
The format of a record in the result archive file is:
".html_text("
<result_archive>
    <id>%d</id>
  <create_time>%d</create_time>
  <workunitid>%d</workunitid>
  <server_state>%d</server_state>
  <outcome>%d</outcome>
  <client_state>%d</client_state>
  <hostid>%d</hostid>
  <userid>%d</userid>
  <report_deadline>%d</report_deadline>
  <sent_time>%d</sent_time>
  <received_time>%d</received_time>
  <name>%s</name>
  <cpu_time>%.15e</cpu_time>
  <xml_doc_in>%s</xml_doc_in>
  <xml_doc_out>%s</xml_doc_out>
  <stderr_out>%s</stderr_out>
  <batch>%d</batch>
  <file_delete_state>%d</file_delete_state>
  <validate_state>%d</validate_state>
  <claimed_credit>%.15e</claimed_credit>
  <granted_credit>%.15e</granted_credit>
  <opaque>%f</opaque>
  <random>%d</random>
  <app_version_num>%d</app_version_num>
  <appid>%d</appid>
  <exit_status>%d</exit_status>
  <teamid>%d</teamid>
  <priority>%d</priority>
  <mod_time>%s</mod_time>
</result_archive>
")."
The format of a record in the WU archive file is:
".html_text("
<workunit_archive>
    <id>%d</id>
  <create_time>%d</create_time>
  <appid>%d</appid>
  <name>%s</name>
  <xml_doc>%s</xml_doc>
  <batch>%d</batch>
  <rsc_fpops_est>%.15e</rsc_fpops_est>
  <rsc_fpops_bound>%.15e</rsc_fpops_bound>
  <rsc_memory_bound>%.15e</rsc_memory_bound>
  <rsc_disk_bound>%.15e</rsc_disk_bound>
  <need_validate>%d</need_validate>
  <canonical_resultid>%d</canonical_resultid>
  <canonical_credit>%.15e</canonical_credit>
  <transition_time>%d</transition_time>
  <delay_bound>%d</delay_bound>
  <error_mask>%d</error_mask>
  <file_delete_state>%d</file_delete_state>
  <assimilate_state>%d</assimilate_state>
  <hr_class>%d</hr_class>
  <opaque>%f</opaque>
  <min_quorum>%d</min_quorum>
  <target_nresults>%d</target_nresults>
  <max_error_results>%d</max_error_results>
  <max_total_results>%d</max_total_results>
  <max_success_results>%d</max_success_results>
  <result_template_file>%s</result_template_file>
  <priority>%d</priority>
  <mod_time>%s</mod_time>
</workunit_archive>
")."
";
page_tail();
?>
