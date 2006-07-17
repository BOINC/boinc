<?php
require_once("docutil.php");
require_once("projects.inc");
page_head("Choosing BOINC projects");

echo "
A partial list of current projects (mouse over for details):
    <ul>
";
shuffle($projects);
foreach ($projects as $p) {
    echo "<li> <a href=$p[1] onmouseover=\"return escape('<img align=right vspace=4 hspace=4 src=images/$p[5]><b>Home:</b> $p[2]<hr><b>Area:</b> $p[3]<hr><b>Goal:</b> $p[4]')\">$p[0]</a>
    ";
}
echo "
</ul>
<p>
You can participate in several projects, ensuring that
your computer will be kept busy even when one project has no work.
You can control how your resources (such as computer time
and disk space) are divided among these projects.
When you attach to a project, you will be asked for its URL.
This is simply its web address: visit each project's web site,
and copy the URL from your browser's address field.

<p>
Projects are independent.
The BOINC developers and the University of California
have no control over the creation of BOINC-based projects,
and do not endorse them.

<p>
When you participate in a project,
you entrust that project with the health of your
computer and the privacy of your data.
In deciding whether to participate in a project,
you should consider the following questions:

<ul>
<li> Do you trust the project to ensure that its applications
  won't damage your computer or violate your privacy?
<li> Do you trust the project to use proper security practices on their servers?
<li> Does the project clearly describe its goals,
    and are these goals important and beneficial?
<li> Who owns the results of the computation?
  If the results are valuable, will they be freely available to the public
  or will they belong to a for-profit business?
</ul>

";
echo "
    <script language=\"JavaScript\" type=\"text/javascript\" src=\"wz_tooltip.js\"></script>
";
page_tail();
?>
