<?
require_once("docutil.php");
page_head("Validation");
echo "
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

<p>
The XML document listing the output files has the form: <pre>
&lt;file_info>...&lt;/file_info>
[ ... ]
&lt;result>
    &lt;name>foobar&lt;/name>
    &lt;wu_name>blah&lt;/wu_name>
    &lt;exit_status>blah&lt;/exit_status>
    &lt;file_ref>...&lt;/file_ref>
    [ ... ]
&lt;/result>
</pre>
The components are:
<ul>
<li> The <b>&lt;name></b> element is the result name.
<li> The <b>&lt;wu_name></b> element is the workunit name.
<li> Each <b>&lt;file_ref></b> element is an association to an
output file, described by a corresponding <b>&lt;file_info></b> element.
</ul>
<p>
The XML document describing the sizes and checksums of the output
files is just a list of <b>&lt;file_info></b> elements, with the
<b>nbytes</b> and <b>md5_cksum</b> fields present.
The project back end
must parse this field to find the locations and checksums of output files.
";
page_tail();
?>
