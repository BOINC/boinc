<?
require_once("docutil.php");
page_head("Back end programs");
echo "

<p>
A project back end consists of a set of programs,
some of which have project- or application-specific parts.

<p>
<table border=1 cellpadding=8>
<tr>
<th>Component</th>
<th>BOINC-supplied part</th>
<th>project-supplied part</th>
</tr>
<tr>
<td valign=top>
<b>Work generator</b>: generates work units and the corresponding input files.
</td>
<td valign=top>
Interfaces for creating workunits.
</td>
<td valign=top>
Programs or scripts that generate input files,
install them on data servers, and call the BOINC interface functions.
</td></tr>
<tr>
<td valign=top><b>Transitioner</b>:
Handles various state transitions of workunits and results,
such as timeouts.
Generates results for workunits as needed.
</td>
<td valign=top>A program <b>transitioner</b>.</td>
<td valign=top>None</td>
</tr>
<tr>
<td valign=top><b>Result validation and accounting</b>:
compare redundant results; select a <b>canonical result</b>
representing the correct output,
and a <b>canonical credit</b> granted to users and hosts
that return the correct output.</td>
<td valign=top>A program, <b>validate</b>,
that contains the basic logic for validation.</td>
<td valign=top>An application-specific function, linked with <b>validate</b>,
that compares sets of redundant results.</td>
</tr>
<tr>
<td valign=top><b>Assimilator</b>:
handles workunits that are 'completed':
that is, that have a canonical result or for which
an error condition has occurred.
Handling a successfully completed result might involve
record results in a database and perhaps generating more work.</td>
<td valign=top>
A program <b>assimilator</b> that contains the
basic logic for assimilation.
</td>
<td valign=top>
A handler function that assimilates a workunit,
either by processing its canonical result
or handling an error return.
</td>
</tr>
<tr>
<td valign=top><b>File deleter</b>: delete input and output files
when they are no longer needed.</td>
<td valign=top>A program <b>file_deleter</b>.</td>
<td valign=top>None.</td>
</tr>
</table>

<h3>Work generator</h3>
<p>
<pre>
    for each wu created
        wu.transition_time = now
</pre>

<h3>scheduler</h3>
<pre>
    when send a result
        result.report_deadline = now + wu.delay_bound
        wu.transition_time = min(wu.transition_time, result.report_deadline)
    when receive a result
        if client error
            result.outcome = client_error
            result.validate_state = INVALID
        else
            result.outcome = success
        result.server_state = OVER
        wu.transition_time = now
    when a result falls off the bottom of infeasible queue
        result.server_state = OVER
        result.outcome = COULDNT_SEND
        wu.transition_time = now
</pre>

<h3>Transitioner</h3>
<p>
<pre>
// gets run when either
// - a result becomes done (via timeout or client reply)
// - the WU error mask is set (e.g. by validater)
// - assimilation is finished
    for each WU with now > transition_time:

        // check for timed-out results
        for each result of WU
            if result.server_state = in_progress and now > result.report_deadline
                result.server_state = OVER
                result.outcome = NO_REPLY

        // trigger validation if needed
        K = # of SUCCESS results
        if K >= M
            if any result is server_state OVER, outcome SUCCESS, validate_state INIT
                wu.need_validate = true

        // check for WU error conditions
        if any result has outcome couldnt_send
            error_mask |= couldnt_send
        K = # results with outcome = client_error
        if K > A
            error_mask |= too_many_error_results

        // Note: check on # of success results is done in validator

        K = total # results
        if K > B
            error_mask |= too_many_total_results
        
        // if no WU errors, generate new results if needed
        if error_mask == 0
            K = # results w/ server_state = unsent or in_progress
            L = N - K
            generate L new results
        
        // if WU errors, clean up unsent results
        // and trigger assimilation if needed
        if error_mask
            for all results server_state = unsent
                server_state = over
                outcome = didnt_need
            if wu.assimilate_state == init
                wu.assimilate_state = ready
        
        // if WU is assimilated, trigger deletion of files
        if wu.assimilated_state = DONE
            // trigger input file deletion if needed
            if all results are OVER
                wu.file_delete_state = ready

            // outputs of error results can be deleted immediately;
            // outputs of successful results can be deleted when validated
            for results of WU
                if canonical result and not all results OVER
                    continue
                if outcome = CLIENT_ERROR or (SUCCESS and (VALID or INVALID))
                    if file_delete_state = INIT
                        result.file_delete_state = READY

        // get next result timeout if any
        transition_time = MAX_INT
        if any results are IN_PROGRESS
            transition_time = min(result.report_deadline)
</pre>

<h3>Validator</h3>
<p>
BOINC supplies a utility program <b>validate</b>
to perform validation and credit-granting.
This program must be linked with two project-specific functions:
<pre>
int check_set(vector<RESULT> results, int& canonicalid, double& credit);
int check_pair(RESULT& r1, RESULT& r2, bool& match);
</pre>
<b>check_set()</b> takes a set of results.
If there is sufficient agreement,
it selects one of them as the canonical result
(returning its ID) and also decides what credit should
be granted for correct results for this workunit.
<p>
<b>check_pair()</b> compares two results and returns match=true
if they agree.

<p>
The file <b>validate_test.C</b> contains an example
implementation of check_set() and check_pair().

<pre>
    for each WU w/ need_validate true
        if have canonical result
            for each result w/ validate_state INIT and outcome SUCCESS
                // possible that we've already deleted canonical output files
                if canonical_result.file_delete_state = DONE
                    validate_state = INVALID
                else
                    if matches canonical, grant credit
                    validate_state = VALID or INVALID
                need_to_handle_over_results = true
        else
            S = set of results w/ outcome SUCCESS
            if consensus(S)
                set canonical_result
                set success results as VALID or INVALID
                grant credit
                need_to_handle_over_results = true
                wu.assimilate_state = READY
                for all results server_state UNSENT
                    server_state = OVER
                    outcome = DIDNT_NEED
            else
                if # of successful results > C
                    wu.error_mask |= too_many_success_result
                    need_to_handle_over_results = true

        if need_to_handle_over_results:
            wu.transition_time = now
</pre>


<h3>Assimilator</h3>
<pre>
    for each WU with assimilate_state = READY
        call project-specific handler function
        wu.assimilate_state = done
        wu.transition_time = now
</pre>
";
page_tail();
?>
