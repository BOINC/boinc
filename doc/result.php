<?
require_once("docutil.php");
page_head("Results");
echo "
<p>
A <b>result</b> describes an instance of a computation, either unstarted,
in progress, or completed.
The attributes of a result include:
";
list_start();
list_item(
    "name",
    "A text string, unique across all results in the project."
);
list_item(
    "workunit name", ""
);
list_item("output files",
    "A list of the names of the output files,
    and the names by which the application refers to them."
);
list_item("server state",
    "Values include: 
    <ul>
        <li> Inactive (not ready to dispatch) 
        <li> Unsent (ready to sent to a client, but not sent) 
        <li> In progress (sent, not done) 
        <li> Done successfully 
        <li> Timed out 
        <li> Done with error 
        <li> Not needed (work unit was finalized before this result was sent)
    </ul>"
);
list_end();

echo "
<p>
Additional attributes are defined after the result is completed:
";
list_start();
list_item("host",
    "The host that executed the computation"
);
list_item("exit status", "");
list_item("CPU time",
    "The CPU time that was used."
);
list_item("output file info",
    "The sizes and checksums of its output files"
);
list_item("stderr",
    "The stderr output of the computation"
);
list_item("host",
    "The host that was sent the result."
);
list_item("received time",
    "The time when the result was received."
);
list_end();
echo"
";
page_tail();
?>
