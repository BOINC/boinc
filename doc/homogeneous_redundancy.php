<?
require_once("docutil.php");
page_head("Homogeneous redundancy ");
echo"
Some applications are so numerically unstable
that platform differences cause a great
diversity of correct results for a given workunit.
In such cases it may be difficult to distinguish
between results that are correct but differ because
of numerical variation,
and results that are erroneous.

<p>
BOINC provides a feature called <b>homogeneous redundancy</b>
for such applications.
This is enabled by including the line
<pre>
&lt;homogeneous_redundancy/&gt;
</pre>
in the <a href=configuration.php>config.xml</a> file.

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
