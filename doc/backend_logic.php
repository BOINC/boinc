<?php
require_once("docutil.php");
page_head("Backend program logic");
echo "
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
            if (all results are OVER and those that are outcome SUCCESS
                have validate_state != INIT)
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
        for all results IN_PROGRESS
            transition_time = min(transition_time, result.report_deadline)

        // if transitioner is way behind schedule,
        // don't repeatedly handle this WU
        transition_time = max(transition_time, now+delay_bound)
</pre>

<h3>Validator</h3>
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
