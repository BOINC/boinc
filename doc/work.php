<?php
require_once("docutil.php");
page_head("Workunits");
echo "
<p>
A <b>workunit</b> describes a computation to be performed.
It is represented by a row in the 'workunit' database table.
BOINC provides a <a href=tools_work.php>utility program and C function</a>
for creating workunits. 
<p>
A workunit has several attributes that are specified
when the workunit is created:
";

list_start();
list_item(
    "name",
    "A text string, unique across all workunits in the project."
);
list_item(
    "application",
    "Which <a href=app.php>application</a> will perform the computation.
    A workunit is associated with an application,
	not with a particular version or range of versions.
    If the input data format changes in
    a way that is incompatible with older versions,
    you must create a new application.
    This can often be avoided by using XML data format. "
);
list_item(
    "input files",
    "A list of the input files: their names,
    and the names by which the application refers to them.
    Typically these file are downloaded from a data server.
    However, if the &lt;generate_locally/&gt; element is present,
    the file is generated on the client
    (typically by an earlier instance of the same application).
    Applications should use file locking to prevent
    two jobs from generating the file at the same time.
    "
);
list_item(
    "priority",
    "Higher-priority work is dispatched first"
);
list_item(
    "batch",
    "An integer; can be used to operate
    (cancel, change priority etc.)
    on groups of workunits."
);
list_bar("Resource estimates and bounds");
list_item(
    "rsc_fpops_est",
    "An estimate of the average number of floating-point operations
    required to complete the computation.
    This used to estimate how long the computation will
    take on a given host."
);
list_item(
    "rsc_fpops_bound",
    "A bound on the number of floating-point operations
    required to complete the computation.
    If this bound is exceeded, the application will be aborted."
);
list_item(
    "rsc_memory_bound",
    "An estimate of application's largest working set size.
    The workunit will only be sent to hosts with
    at least this much available RAM."
);
list_item(
    "rsc_disk_bound",
    "A bound on the maximum disk space used by the application,
    including all input, temporary, and output files.
    The workunit will only be sent to hosts with
    at least this much available disk space.
    If this bound is exceeded, the application will be aborted."
);
list_bar("Redundancy and scheduling attributes");
list_item(
    "delay_bound",
    "An upper bound on the time (in seconds) between sending
    a result to a client and receiving a reply.
    The scheduler won't issue a result if the estimated
    completion time exceeds this.
    If the client doesn't respond within this interval,
    the server 'gives up' on the result
    and generates a new result, to be assigned to another client.
    <p>
    Set this to several times the average execution time
    of a workunit on a typical PC.
    If you set it too low,
    BOINC may not be able to send some results,
    and the corresponding workunit will be flagged with an error.
    If you set it too high,
    there may a corresponding delay in getting results back."
);
list_item(
    "min_quorum",
    "The minimum size of a 'quorum'.
    The validator is run when there are this many successful results.
    If a strict majority agree, they are considered correct.
    Set this to two or more if you want redundant computing."
);
list_item(
    "target_nresults",
    "How many results to create initially.
    This must be at least <b>min_quorum</b>.
    It may be more, to reflect the ratio of result loss,
    or to get a quorum more quickly."
);
list_item(
    "max_error_results",
    "If the number of client error results exceeds this,
    the work unit is declared to have an error;
    no further results are issued, and the assimilator is triggered.
    This safeguards against workunits that cause the application to crash."
);
list_item(
    "max_total_results",
    "If the total number of results for this workunit exceeds this,
    the workunit is declared to be in error.
    This safeguards against workunits that are never reported
    (e.g. because they crash the core client)."
);
list_item(
    "max_success_results",
    "If the number of success results for this workunit exceeds this,
    and a consensus has not been reached,
    the workunit is declared to be in error.
    This safeguards against workunits that produce nondeterministic results."
);
list_end();

echo "
A workunit has a dynamic attribute:
";
list_start();
list_item(
    "<nobr>error mask</nobr>",
    "A bit mask of various error conditions:
    <ul>
    <li> <b>WU_ERROR_COULDNT_SEND_RESULT</b>:
        The BOINC scheduler was unable to send the workunit
        to a large number (~100) of hosts,
        probably because its resource requirements (disk, memory, CPU)
        were too large for the hosts,
        or because no application version was available
        for the hosts' platforms.
        In this case BOINC 'gives up' on the workunit.
    <li> <b>WU_ERROR_TOO_MANY_ERROR_RESULTS</b>:
        Too many results with error conditions
        (upload/download problem, client crashes)
        have been returned for this work unit.
    <li> <b>WU_ERROR_TOO_MANY_SUCCESS_RESULTS</b>:
        Too many successful results have been returned
        without consensus.
        This indicates that the application may
        be nondeterministic.
    <li> <b>WU_ERROR_TOO_MANY_TOTAL_RESULTS</b>:
        Too many total results have been sent for this workunit.
    </ul>
    "
);
list_end();
echo "
If any of these conditions holds,
BOINC 'gives up' on the workunit and doesn't
dispatch more results for it.
";

page_tail();
?>
