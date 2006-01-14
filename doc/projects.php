<?php
require_once("docutil.php");
page_head("Choosing BOINC project(s)");
echo "
BOINC was originally developed at the University of California to support
<a href=http://setiathome.berkeley.edu>SETI@home</a>.
However, other distributed computing projects use BOINC.
BOINC allows you to participate in multiple projects,
and to control how your resources (such as computer time
and disk space) are divided among these projects.

<p>
Projects are independent, and each maintains its own servers.
The BOINC developers and the University of California
have no control over the creation of BOINC-based projects,
and in general do not endorse them.
The BOINC web sites lists some, but not all, projects.
To find projects, try using <a href=google.com>Google</a>.

<p>
When you participate in a project,
you entrust that project with the health of your
computer and the privacy of its data.
In deciding whether to participate in a project,
you should consider the following questions:

<ul>
<li> Do you trust the project to ensure that its applications
  won't damage your computer or cause security problems?
<li> Do you trust the project to use proper security practices on their servers?
<li> Does the project clearly describe its goals?
<li> Who owns the results of the computation?
  If the results are valuable, will they be freely available to the public
  or will they belong to a for-profit business?
</ul>

";
page_tail();
?>
