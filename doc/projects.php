<?php
require_once("docutil.php");
require_once("projects.inc");
require_once("get_platforms.inc");
page_head("Choosing BOINC projects");

echo "
<p>
BOINC is used by many volunteer computing <b>projects</b>.
Some are based at universities and research labs,
others are run by companies and individuals.
You can participate in any number of these projects.
<p>
In deciding whether to participate in a project,
read its web site and consider the following questions:

<ul>
<li> Does it clearly describe its goals?
    Are these goals important and beneficial?
<li> Do they have any published results? See
<a href=/wiki/Publications_by_BOINC_projects>
A list of scientific publications of BOINC projects</a>.
<li> Do you trust it to use proper security practices?
<li> Who owns the results of the computation?
  Will they be freely available to the public?
  Will they belong to a company?
</ul>
<p>

The following projects are known to us at BOINC,
and we believe that their descriptions are accurate.
See also
<a href=wiki/Project_list>a complete list of projects</a>.

<p>
Note: if your computer is equipped with a Graphics Processing Unit
(GPU), you may be able to
<a href=http://boinc.berkeley.edu/wiki/GPU_computing>use it to compute faster</a>.
";
list_start("cellpadding=2 width=100%");
list_heading_array(array(
    "Project<br><span class=note>Mouse over for details; click to visit web site</span>",
    "Home",
    "Research area",
    "Supported platforms"
    ));
shuffle($areas);
foreach ($areas as $area) {
    list_bar($area[0]);
    $projects = $area[1];
    shuffle($projects);
    $n = 0;
    foreach ($projects as $p) {
        $img = "";
        if (array_key_exists(5, $p) && $p[5]) {
            $img= "<img align=right vspace=4 hspace=4 src=images/$p[5]>";
        }
        $desc = addslashes($p[4]);
        $x = "<a href=$p[1] onmouseover=\"return escape('$img <b>Home:</b> $p[2]<hr><b>Area:</b> $p[3]<hr><b>Goal:</b> $desc')\">$p[0]</a>";
        $home = $p[2];
        $area = $p[3];
        $master_url = $p[1];
        if (array_key_exists(6, $p)) {
            $master_url = $p[6];
        }
        $p = get_platforms_string($master_url);
        echo "<tr class=row$n>
            <td valign=top>$x</td>
            <td valign=top>$home</td>
            <td valign=top>$area</td>
            <td width=30% valign=top>$p</td>
            </tr>
        ";
        $n = 1-$n;
    }
}
list_end();
echo "
</ul>

<p>
If you run a BOINC-based project
and would like it to be included on this list,
please <a href=http://boinc.berkeley.edu/trac/wiki/ProjectPeople>contact us</a>.
<script language=\"JavaScript\" type=\"text/javascript\" src=\"wz_tooltip.js\"></script>
";
page_tail();
?>
