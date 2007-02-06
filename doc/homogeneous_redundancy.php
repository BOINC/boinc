<?php
require_once("docutil.php");
page_head("Dealing with numerical discrepancies");
echo"
Most numerical applications produce different outcomes 
for a given workunit depending on the machine 
architecture, operating system, compiler, and compiler flags.
For some applications these discrepancies produce
only small differences in the final output,
and results can be validated using a 'fuzzy comparison' function
that allows for deviations of a few percent.
<p>
Other applications are 'divergent' in the sense that small
numerical differences lead to unpredictably large differences
in the final output.
For such applications it may be difficult to distinguish
between results that are correct but differ because
of numerical discrepancies, and results that are erroneous.
The 'fuzzy comparison' approach does not work for such applications.

<h2>Eliminating discrepancies</h2>
<p>
One approach is to eliminate numerical discrepancies.
Some notes on how to do this for Fortran programs are given in a paper,
<a href=MOM1MP01.pdf>Massive Tracking on Heterogeneous Platforms</a>
and in an earlier <a href=fortran_numerics.txt>text document</a>,
both courtesy of Eric McIntosh from CERN.

<h2>Homogeneous redundancy</h2>
<p>
BOINC provides a feature called <b>homogeneous redundancy</b>
to handle divergent applications.
You can enable it for a project by including the line
<pre>
&lt;homogeneous_redundancy/&gt;
</pre>
in the <a href=configuration.php>config.xml</a> file.

<p>
Alternatively, you can enable it selectively for a single
application by setting the
<code>homogeneous_redundancy</code> field in its database record.
<p>
When this feature is enabled,
the BOINC scheduler will send results for a given workunit
only to hosts with the same operation system name and CPU vendor
(i.e., the os_name and p_vendor fields of the host description).
For example: if a result has been sent to a host of type
(Windows XP, Intel), then other results of that workunit will
only be sent to hosts of type (Windows XP, Intel).

<p>
If homogeneous redundancy is enabled,
it may be possible to use strict equality to compare redundant results.

";
page_tail();
?>
