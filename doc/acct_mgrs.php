<?php
require_once("docutil.php");

page_head("Account managers");

$acct_mgrs = array(
    array(
        "The BOINCStats Account Manager",
        "http://bam.boincstats.com",
        "(BAM!)"
    ),
    array(
        "GridRepublic",
        "http://gridrepublic.org",
        ""
    ),
);

echo "
An <b>account manager</b> is a web site that simplifies
participating in BOINC, especially if you are new to BOINC,
or if you have several computers,
participate in several projects, or like to learn about new projects.
The account manager concept was conceived and
developed jointly by <a href=http://gridrepublic.org>GridRepublic</a>
and BOINC.
The following account managers are available:
<ul>
";
shuffle($acct_mgrs);
foreach($acct_mgrs as $am) {
    $name = $am[0];
    $url = $am[1];
    $text = $am[2];
    
    echo "<li> <a href=$url>$name</a> $text\n";
}
echo "
</ul>
<p>
<h3>Why use an account manager?</h3>
<table cellpadding=4 border=1>
<tr><th class=heading>With account manager</td>
    <th class=heading>Without account manager</td>
</tr>
<tr>
    <td valign=top>
        See all BOINC projects, old and new,
        listed and described at the account manager.
    </td>
    <td valign=top>
        Find BOINC projects by word-of-mouth
        or using a search engine.
    </td>
</tr>
<tr>
    <td valign=top>
        Attach to a project with one mouse click.
        If you have multiple computers,
        all of them will be attached.
    </td>
    <td valign=top>
        Attach to a project by bringing up the 'Attach Project Wizard'
        in the BOINC Manager, and entering the URL and
        your email address and password.
        You must do this separately at each of your computers.
    </td>
</tr>
<tr>
    <td valign=top>
        Change your account details (name, email address, password)
        in one web page, at the account manager.
    </td>
    <td valign=top>
        Change your account details on each project web site, separately.
    </td>
</tr>
<tr>
    <td valign=top>
        Set resource shares for all projects in one web page,
        at the account manager.
    </td>
    <td valign=top>
        Set resource shares at each project web site, separately.
    </td>
</tr>
<tr>
    <td valign=top>
        Create/join/quit teams in one web page, at the account manager.
    </td>
    <td valign=top>
        Create/join/quit teams at each project web site, separately.
    </td>
</tr>
</table>
";
page_tail();
?>
