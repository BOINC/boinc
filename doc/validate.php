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
BOINC supplies a framework program <b>validate.C</b>.
This program must be linked with two application-specific functions:
<pre>",
htmlspecialchars("
int check_set(vector<RESULT> results, int& canonicalid, double& credit);
int check_pair(RESULT& r1, RESULT& r2, bool& match);
"),
"</pre>
<b>check_set()</b> takes a set of results.
If there is sufficient agreement,
it selects one of them as the canonical result
(returning its ID) and also decides what credit should
be granted for correct results for this workunit.
<p>
<b>check_pair()</b> compares two results and returns match=true
if they agree.

<p>
Two example validators are supplied
(each implements check_set() and check_pair()):
<ul>
<li>
<b>validate_test</b> requires a strict majority,
and regards results as equivalent only if they agree byte for byte.
<li>
<b>validate_trivial</b>
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
