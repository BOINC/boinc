<?php
require_once("docutil.php");
page_head("Back-end utility functions");
echo "
The following functions can be used in your validator and assimilator code:
<hr>

".html_text("
int get_output_file_path(RESULT const&, std::string&);
"),"
Returns the path of a result's output file
(parses result.xml_doc_out and computes the file's position in the
 <a href=hier_dir.php>hierarchical directory structure</a>).

<p>
Note: this function doesn't handle multiple output files
(if there are multiple files, it returns the path of the first one).
If your application has multiple output files, see below.
<hr>
".html_text("
int try_fopen(char* path, FILE*& f, char* mode);
")."
Open a file, distinguishing between recoverable and nonrecoverable errors.
Returns zero on success.
Returns ERR_FOPEN if the directory is present but not the file
(this is considered a nonrecoverable error).
Returns ERR_OPENDIR if the directory is not there
(this is generally a recoverable error, like NFS mount failure).
<hr>
".html_text("
double median_mean_credit(vector<RESULT> const& results);
")."
Given a vector of N correct results, computes a canonical credit as follows:
<ul>
<li> if N==1, return that result's claimed credit
<li> if N==2, return min of claimed credits
<li> if N>2, toss out high and low claimed credit,
and return the average of the rest.
</ul>
<hr>
<h3>Multiple output files</h3>
If your application has multiple output files
you'll need to generalize get_output_file_path().
To do this you'll need to know the following:

<p>
The database field 'result.xml_doc_out'
describes a result's output files.
It has the form
".html_text("
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
