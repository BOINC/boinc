<?
require_once("docutil.php");
page_head("Workunit and result state transitions");
echo "

<p>
The processing of workunits and results
can be described in terms of transitions of their state variables.

<h3>Workunit state variables</h3>
<p>
Several workunits parameters are described
<a href=work.php>here</a>.

Other workunit state variables are described in
the following table:
";
list_start();
list_item(
    "canonical_resultid",
    "The ID of the canonical result for this workunit, or zero."
);

list_item("transition_time",
    "The next time to check for state transitions for this WU.
    <ul>
    <li>Initially now.
    <li>Set to now by scheduler when get a result for this WU.
    <li>Set to min(current value, now + delay_bound) by scheduler
    when send a result for this WU
    <li>Set to min(x.sent_time + wu.delay_bound) over IN_PROGRESS results x
    by transitioner when done handling this WU
    <li>Set to now by validater if it finds canonical result,
    or if there is already a canonical result
    and some other results have validate_state = INIT,
    or if there is no consensus and the number of successful results
    is > wu.max_success_results
    </ul>
    "
);
list_item("file_delete_state",
    "Indicates whether input files should be deleted.
    <ul>
    <li>Initially INIT
    <li>Set to READY by transitioner when all results have server_state=OVER
        and wu.assimilate_state=DONE
    <li>Set to DONE by file_deleter when it has attempted to delete files.
    </ul>
    "
);

list_item("assimilate_state",
    "Indicates whether the workunit should be assimilated.
    <ul>
    <li> Initially INIT
    <li> Set to READY by transitioner if wu.assimilate_state=INIT
        and WU has error condition
    <li> Set to READY by validater when find canonical result
        and wu.assimilate_state=INIT
    <li> Set to DONE by assimilator when done
    </ul>
    "
);

list_item("need_validate",
    "Indicates that the workunit has a result that needs validation.
    <ul>
    <li> Initially FALSE
    <li> Set to TRUE by transitioner if the number of success results
        is at least wu.min_quorum and there is a success result
        not validated yet
    <li> Set to FALSE by validater
    </ul>
    "
);

list_item("error_mask",
    "A bit mask for error conditions.
    <ul>
    <li> Initially zero
    <li> Transitioner sets COULDNT_SEND_RESULT if some result couldn't be sent.
    <li> Transitioner sets TOO_MANY_RESULTS if too many error results
    <li> Transitioner sets TOO_MANY_TOTAL_RESULTS if too many total results
    <li> Validater sets TOO_MANY_SUCCESS_RESULTS if no consensus
        and too many success results
    </ul>
    "
);
list_end();
echo "

</ul>

Workunit invariants:
<ul>
<li> eventually either canonical_resultid or error_mask is set
<li> eventually transition_time = infinity
<li> Each WU is assimilated exactly once
</ul>

<p>
Notes on deletion of input files:
<ul>
<li> Input files are eventually deleted,
but only when all results have state=OVER
(so that clients don't get download failures)
and the WU has been assimilated
(in case the project wants to examine input files in error case).
<li>
All output files (including errors) are retained
until the WU has been assimilated.
After that,
the canonical result output files are retained until
all other results are over and (if necessary) validated.
Error output files can be deleted immediately.
Success output files can be deleted after validation.
</ul>

<h3>Result state variable</h3>
Result state variables are listed in the following table:
";


list_start();
list_item("report_deadline",
    "Give up on result (and possibly delete input files)
    if don't get reply by this time.
    <ul>
    <li> Set by scheduler to now + wu.delay_bound when send result
    </ul>
    "
);
list_item("server_state",
    "Values: UNSENT, IN_PROGRESS, OVER
    <ul>
    <li> Initially UNSENT
    <li> Set by scheduler to IN_PROGRESS when send result
    <li> Set by scheduler to OVER when get reply from client
    <li> Set by transitioner to OVER if now < result.report_deadline
    <li> Set by transitioner to OVER if WU has error condition
        and result.server_state=UNSENT
    <li> Set by validater to OVER if WU has canonical result
        and result.server_state=UNSENT
    </ul>
    "
);
list_item("outcome",
    "Values: SUCCESS, COULDNT_SEND, CLIENT_ERROR, NO_REPLY, DIDNT_NEED.
    <br>Defined iff result.server_state=OVER
    <ul>
    <li> Set by scheduler to SUCCESS if get reply and no client error
    <li> Set by scheduler to CLIENT_ERROR if get reply and client error
    <li> Set by transitioner to NO_REPLY if server_state=IN_PROGRESS
        and now<report_deadline
    <li> Set by transitioner to DIDNT_NEED if WU has error condition
        and result.server_state=UNSENT
    <li> Set by validater to DIDNT_NEED if WU has canonical result
        and result.server_state=UNSENT
    </ul>
    "
);
list_item("client_state",
    "Records the client state (upload, process, or download)
    where an error occurred.
    Defined if outcome is CLIENT_ERROR.
    "
);

list_item("file_delete_state",
    "
    <ul>
    <li> Initially INIT
    <li> Set by transitioner to READY if this is the canonical result,
        and file_delete_state=INIT,
        and wu.assimilate_state=DONE
    <li> Set by transitioner to READY if wu.assimilate_state=DONE
        and result.outcome=CLIENT_ERROR
        or result.validate_state!=INIT
    "
);

list_item("validate_state",
    "
    Defined iff result.outcome=SUCCESS
    <ul>
    <li> Initially INIT
    <li> Set by validater to VALID if outcome=SUCCESS and matches canonical result
    <li> Set by validater to INVALID if outcome=SUCCESS and doesn't match canonical result
    "
);
list_end();

echo "

<p>
Result invariants:
<ul>
<li> Eventually server_state = OVER.
<li> Output files are eventually deleted.
</ul>
Notes on deletion of output files:
<ul>
<li> Non-canonical results can be deleted as soon as the WU is assimilated.
<li> Canonical results can be deleted only when all results have server_state=OVER.
<li> If a result reply arrives after its timeout,
the output files can be immediately deleted.
</ul>
How do we delete output files that arrive REALLY late?
(e.g. uploaded after all results have timed out, and never reported)?
Possible answer:
let X = create time of oldest unassimilated WU.
Any output files created before X can be deleted.
</ul>
";
page_tail();
?>
