<?
require_once("docutil.php");
page_head("Validation");
echo "
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
