<?php
require_once("docutil.php");
page_head("File-based project statistics data");
echo "

<p>
Projects export statistics data in XML-format files.
Most projects regenerate the files every 24 hours.
These files are contained in a download directory,
linked to from the project's web site
(generally X/stats/, where X is the project URL).
<p>
The download directory contains the following files:
";
list_start();
list_item("<b>tables.xml</b>",
"This gives the total number of records
for each entity type (team, user, and host).
It also includes the UNIX time when the files were last generated,
and a list of the project's applications,
with counts of various things.
<br>
For example:
<pre>".
htmlspecialchars("<tables>
    <update_time>1046220857</update_time>
    <nusers>127</nusers>
    <nteams>14</nteams>
    <nhosts>681</nhosts>
    <total_credit>1234.234</total_credit>
    <applications>
        <application>
            <name>setiathome</name>
            <results_unsent>100</results_unsent>
            <results_in_progress>1000</results_in_progress>
            <results_over>10000</results_over>
        </application>
        ...
    </applications>
</tables>
").
"</pre>");
list_item("host.gz", "List of hosts");
list_item("team.gz", "List of teams");
list_item("user.gz", "List of users");
list_end();
echo "
<p>
The format of the various XML elements
in the output files is as follows.
<br>
Notes:
<ul>
<li>
&lt;cpid&gt; ('<a href=cpid.php>cross-project identifier</a>')
is a unique identifier across multiple projects.
Accounts with the same email address on different projects
will have the same cross-project identifier
(as long as at least one computer is attached to both accounts).
<li>
All 'expavg_credit' values were computed at some point
in the past (given by 'expavg_time').
To compute their current values,
they must be scaled according to the formula given
<a href=credit.php>here</a>.
</ul>
<p>

<p>
<b>Team summary</b>
<pre>",
htmlspecialchars("
<team>
 <id>5</id>
 <name>Broadband Reports Team Starfire</name>
 <total_credit>153402.872429</total_credit>
 <expavg_credit>503030.483254</expavg_credit>
 <expavg_time>1087542007.701900</expavg_time>
 <nusers>14</nusers>
</team>
"),
"</pre>
<p>
<b>User summary</b>
<pre>",
htmlspecialchars("
<user>
 <id>12</id>
 <name>John Keck</name>
 <total_credit>42698.813543</total_credit>
 <expavg_credit>117348.653646</expavg_credit>
 <expavg_time>1087542007.701900</expavg_time>
 <cpid>283472938743489759837498347</cpid>
 [ <teamid>5</teamid> ]
 [ <has_profile/> ]
</user>
"), "</pre>
<p>
<b>Host summary </b>
<pre>",
htmlspecialchars("
<host>
  <id>102</id>
  <userid>3</userid>
  <total_credit>0.000000</total_credit>
  <expavg_credit>0.000000</expavg_credit>
  <expavg_time>1087542007.701900</expavg_time>
  <p_vendor>GenuineIntel</p_vendor>
  <p_model>Pentium</p_model>
  <os_name>Windows XP</os_name>
  <os_version>5.1</os_version>
  <create_time>1040170006</create_time>
  <timezone>28800</timezone>
  <ncpus>2</ncpus>
  <p_fpops>45724737.082762</p_fpops>
  <p_iops>43233895.373973</p_iops>
  <p_membw>4032258.064516</p_membw>
  <m_nbytes>670478336.000000</m_nbytes>
  <m_cache>1000000.000000</m_cache>
  <m_swap>1638260736.000000</m_swap>
  <d_total>9088008192.000000</d_total>
  <d_free>3788505088.000000</d_free>
  <n_bwup>24109.794088</n_bwup>
  <n_bwdown>57037.049858</n_bwdown>
  <avg_turnaround>465609.562145</avg_turnaround>
  <host_cpid>e129b5fa44ed8ba58e41c472822f2807</host_cpid>
</host>
"),"</pre>
<a name=task>
<h2>Exporting statistics data</h2>
<p>
Projects: to export statistics data, include an entry like
".html_text("
    <tasks>
        <task>
            <cmd>db_dump -d 2 -dump_spec ../db_dump_spec.xml</cmd>
            <output>db_dump.out</output>
            <period>24 hours</period>
        </task>
    </tasks>
")."
in your config.xml file.
Make sure the file db_dump_spec.xml is in your project's
root directory.
";
page_tail();
?>
