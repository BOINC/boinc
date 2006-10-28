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

".html_text("
int get_output_file_paths(RESULT const&, vector<string>&);
"),"
<p>
Same, for multiple output files.
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
";
page_tail();
?>
