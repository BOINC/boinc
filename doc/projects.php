<?php
require_once("docutil.php");
page_head("Choosing BOINC project(s)");
echo "
BOINC was originally developed to support SETI@home.
However, other distributed computing projects may use BOINC.
BOINC allows you to participate in multiple projects,
and to control how your resources are divided among these projects.

<p>
Projects are independent, and each maintains its own servers.
The BOINC developers and the University of California
have no control over the creation of BOINC-based projects,
and in general do not endorse them.

<p>
When you participate in a BOINC-based project,
you entrust that project with the health of your
computer and the privacy of its data.
In deciding whether to participate in a BOINC-based project,
you should consider the following questions:

<ul>
<li> Do you trust the project to ensure that their applications
  are free of bugs that could damage your computer or cause security problems?
<li> Do you trust the project to use proper security practices on their servers?
<li> For what purposes will the project use your computer?
  Does the project clearly state a limited range of purposes?
<li> Who owns the results of the computation?
  If the results are valuable, will they be freely available to the public
  or will they belong to a for-profit business?
</ul>

";
page_tail();
?>
