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
If there is a quorum of matching results,
it selects one of them as the canonical result, returning its ID.
In this case it also returns the credit to
be granted for correct results for this workunit.

<li>
If, when an output file for a result has a nonrecoverable error
(i.e. the directory is there but the file isn't),
then it must set the result's outcome (in memory, not database)
to VALIDATE_ERROR.
Note: the function try_fopen() (in lib/util.C) can be used
to detect recoverable/nonrecoverable errors.
<li>
If a canonical result is found, check_set() must set the
validate_state field of each non-ERROR result to either VALID or INVALID.

<li>
The workunit is passed in for its min_quorum field.
<li>
If a recoverable error occurs while reading output files
(e.g. a directory wasn't visible due to NSF mount failure)
then check_set() should return retry=true.
This tells the validator to arrange for this WU to be
examined again in a few hours.
<li>
check_set() should return nonzero if a major error
(e.g. database failure) occurs.
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
it sets the new result's validate_state to either VALID or INVALID.
<li>
If it has a nonrecoverable error reading an output file of either result,
it must set the new result's outcome (in memory, not database)
to VALIDATE_ERROR.
<li>
If it has a recoverable error while reading an output file of either result,
it returns retry=true,
which causes the validator to arrange for the WU to be examined
again in a few hours.
<li>
check_pair() should return nonzero if a major error
(e.g. database failure) occurs.
This tells the validator to write an error message and exit.
</ul>

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
<b>validate_util.C</b> contains support functions for
both of the above.

<hr>
<b>NOTE: the above code assumes that each result
has a single output file.
Revisions will be needed to handle multiple output files.
To do this you will need to know the following:
</b>

<p>
The database field 'result.xml_doc_out'
describes a result's output files.
It has the form
<pre>
",htmlspecialchars("
<file_info>...</file_info>
[ ... ]
<result>
    <name>foobar</name>
    <wu_name>blah</wu_name>
    <exit_status>blah</exit_status>
    <file_ref>...</file_ref>
    [ ... ]
</result>
"),"
</pre>
The components are:
<ul>
<li> The <b>&lt;name></b> element is the result name.
<li> The <b>&lt;wu_name></b> element is the workunit name.
<li> Each <b>&lt;file_ref></b> element is an association to an output file,
described by a corresponding <b>&lt;file_info></b> element.
</ul>
<p>
The XML document describing the sizes and checksums of the output
files is a list of <b>&lt;file_info></b> elements,
with the <b>nbytes</b> and <b>md5_cksum</b> fields present.
The project back end
must parse this field to find the locations and checksums of output files.
";
page_tail();
?>
