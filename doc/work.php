<?
require_once("docutil.php");
page_head("Workunits");
echo "
<p>
A <b>workunit</b> describes a computation to be performed.
The basic workunit attributes include:
";

list_start();
list_item(
    "name",
    "A text string, unique across all workunits in the project."
);
list_item(
    "application",
    "Which application performs the computation.
    A workunit is associated with an application, not with a particular
    version or range of versions.
    If the format of your input data changes in
    a way that is incompatible with older versions,
    you must create a new application.
    This can often be avoided by using XML data format. "
);
list_item(
    "input files",
    "A list of its input files: their names,
    and the names by which the application refers to them."
);
list_item(
    "error mask",
    "A bit mask of various error conditions (see below)"
);
list_end();

echo "
A workunit includes estimates of, and bounds on,
its resource usage.
";
list_start();
list_item(
    "rsc_fpops_est",
    "An estimate of the average number of floating-point operations
    required to complete the computation.
    This used to estimate how long the computation will
    take on a give host."
);
list_item(
    "rsc_fpops_bound",
    "A bound on the number of floating-point operations
    required to complete the computation.
    If this bound is exceeded, the application will be aborted."
);
list_item(
    "rsc_mem_bound",
    "A bound on the virtual memory working set size.
    The workunit will only be sent to hosts with
    at least this much available RAM.
    If this bound is exceeded, the application will be aborted."
);
list_item(
    "rsc_disk_bound",
    "A bound on the maximum disk space used by the application,
    including all input, temporary, and output files.
    The workunit will only be sent to hosts with
    at least this much available disk space.
    If this bound is exceeded, the application will be aborted."
);
list_end();

echo "
<p>
A workunit has several parameters related to redundancy and scheduling.
Values for these parameters are supplied by the project
when the workunit is created.
";

list_start();
list_item(
    "delay_bound",
    "An upper bound on the time (in seconds) between sending
    a result to a client and receiving a reply.
    The scheduler won't issue a result if the estimated
    completion time exceeds this.
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
    "The minimum size of a quorum.
    Set this to two or more if you want redundant computing."
);
list_item(
    "target_nresults",
    "How many successful results to get.
    This must be at least <b>min_quorum</b>.
    It may be more to reflect the ratio of result loss,
    or to get a quorum more quickly."
);
list_item(
    "max_error_results",
    "If the number of client error results exceeds this,
    the work unit is declared to have an error;
    no further results are issued, and the assimilator is triggered.
    This safeguards against workunits that exercise a bug
    in the application."
);
list_item(
    "max_total_results",
    "If the total number of results for this workunit exceeds this,
    the workunit is declared to be in error."
);
list_item(
    "max_success_results",
    "If the number of success results for this workunit exceeds this,
    and a consensus has not been reached,
    the workunit is declared to be in error."
);
list_end();

echo "
<p>
The <a href=tools_work.html>create_work</a> utility program provides a
simplified interface for creating workunits. 
";

page_tail();
?>
