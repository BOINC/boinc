<?php
require_once("docutil.php");
page_head("Validation");
echo "
<p>
<b>Validation</b> is the process of comparing redundant results
and deciding which is to be considered correct.
Because floating-point arithmetic varies between platforms,
this decision is an application-specific.
<p>
A <b>validator</b> is a back-end program that does validation
and credit-granting.
You must supply a validator for each application in your project.
BOINC supplies a framework program <b>validator.C</b>.
This program must be linked with two application-specific functions:
<pre>",
htmlspecialchars("
int check_set(vector<RESULT> results, DB_WORKUNIT& wu, int& canonicalid, double& credit, bool& retry);
"),
"</pre>
<ul>
<li><b>check_set()</b> takes a set of results (all with outcome=SUCCESS).
It reads and compares their output files.
If there is a quorum of matching results,
it selects one of them as the canonical result, returning its ID.
In this case it also returns the credit to
be granted for correct results for this workunit.

<li>
If, when an output file for a result has a nonrecoverable error
(e.g. the directory is there but the file isn't,
 or the file is present but has invalid contents),
then it must set the result's outcome (in memory, not database)
to outcome=RESULT_OUTCOME_VALIDATE_ERROR and validate_state=VALIDATE_STATE_INVALID.
<p>
Note: use BOINC's
<a href=backend_util.php>back-end utility functions</a>
to get file pathnames
and to distinguish recoverable and nonrecoverable file-open errors.
<li>
If a canonical result is found, check_set() must set the
validate_state field of each non-ERROR result
(in memory, not database) to either validate_state=VALIDATE_STATE_VALID 
or validate_state=VALIDATE_STATE_INVALID.

<li>
If a recoverable error occurs while reading output files
(e.g. a directory wasn't visible due to NFS mount failure)
then check_set() should return retry=true.
This tells the validator to arrange for this WU to be
processed again in a few hours.
<li>
check_set() should return nonzero if a major error occurs.
This tells the validator to write an error message and exit.
</ul>
<p>
<pre>",
htmlspecialchars("
int check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry);
"),
"</pre>
<ul>
<li>
<b>check_pair()</b> compares a new result to the canonical result.
In the absence of errors,
it sets the new result's validate_state to either VALIDATE_STATE_INVALID or
VALIDATE_STATE_VALID.
<li>
If it has a nonrecoverable error reading an output file of either result,
or if the new result's output file is invalid,
it must set the new result's outcome (in memory, not database)
to VALIDATE_ERROR.
<li>
If it has a recoverable error while reading an output file of either result,
it returns retry=true,
which causes the validator to arrange for the WU to be examined
again in a few hours.
<li>
check_pair() should return nonzero if a major error occurs.
This tells the validator to write an error message and exit.
</ul>

<p>
Neither function should delete files or access the BOINC database.
<p>
A more detailed description is <a href=validate_logic.txt>here</a>.
<p>
Two example validators are supplied
(each implements check_set() and check_pair()):
<ul>
<li>
<b>sample_bitwise_validator</b> requires a strict majority,
and regards results as equivalent only if they agree byte for byte.
<li>
<b>sample_trivial_validator</b>
regards any two results as equivalent if their CPU time
exceeds a given minimum.
</ul>
<p>
A placeholder, validator_placeholder.C is also provided.  You can replace
this file with your own code and 'make' will correctly build and link it.

";
page_tail();
?>
