<?php
require_once("docutil.php");
page_head("Homogeneous redundancy ");
echo"
Most numerical applications produce different outcomes 
for a given workunit depending on the machine 
architecture, operating system, compiler, and compiler flags.
In such cases it may be difficult to distinguish
between results that are correct but differ because
of numerical variation,
and results that are erroneous.

<p>
BOINC provides a feature called <b>homogeneous redundancy</b>
for such applications.
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
