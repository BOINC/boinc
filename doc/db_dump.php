<?php
require_once("docutil.php");
page_head("Downloadable statistics data");
echo "

<p>
BOINC projects can export <b>statistics data</b>
describing teams, users and hosts.
This data can be imported and used to produce
web sites that show statistics and leaderboards
for one or more BOINC projects.
Examples of such sites are listed at
<a href=http://setiweb.ssl.berkeley.edu/sah/stats.php>
http://setiweb.ssl.berkeley.edu/sah/stats.php</a>.

<p>
Statistics data is exported in XML-format files.
XML schemas for these files are
<a href=BOINCSchemas/>here</a>,
and a graphical representation is
<a href=BOINCSchemas/documentation/>here</a>.
<p>
These files are contained in a download directory,
linked to from the project's web site.
A project can decide what data to export,
and how it is divided into files.
This is described by a file <b>db_dump.xml</b> of the following form:
";
echo html_text("
<boinc_db_dump_spec>
  <enumeration>
    <table>x</table>
    <filename>x</filename>
    <sort>x</sort>
    <output>
      <recs_per_file>n</recs_per_file>
      <detail/>
      <compression>x</compression>
    </output>
    ...
  </enumeration>
  ...
</boinc_db_dump_spec>
");
echo "
An 'enumeration' is a listing of particular table.
The fields are:
";
list_start();
list_item("table", "'user', 'host' or 'team'");
list_item("filename", "The base filename.");
list_item("sort", "The sorting criterion:
    'total_credit', 'expavg_credit', or 'id'.
    'id' is the default."
);
list_end();
echo
"An 'output' is a file or set of files containing an enumeration.
The fields are:";
list_start();
list_item("recs_per_file",
    "If present, the listing is divided into multiple files
    with the given number of records per file.
    The file names have the form xxx_N,
    where xxx is the base filename.
    For views that are ordered by ID,
    each file contains a fixed-size segment of the ID range,
    not a fixed number of records.
    If the database ID allocation has gaps,
    files will have fewer than this number of records.
    <p>
    If zero or absent,
    the listing is written to a single file."
);
list_item("detail",
    "If present, records are 'detailed':
    user records include a list of hosts,
    and team records include a list of users."
);
list_end();
echo"
<p>
The download directory contains the following files:
";
list_start();
list_item("<b>tables.xml</b>",
"This gives the total number of records
for each entity type (team, user, and host).
It also includes the UNIX time when the files were last generated,
and a list of the project's applications,
with counts of results in various states.
<br>
For example:
<pre>".
htmlspecialchars("<tables>
    <update_time>1046220857</update_time>
    <nusers_total>127</nusers_total>
    <nteams_total>14</nteams_total>
    <nhosts_total>681</nhosts_total>
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
list_item("<b>core_versions.xml</b>",
"A list of versions of the core client in the project's database"
);
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
<b>Team detail</b>
<pre>",
htmlspecialchars("
<team>
 <id>5</id>
 <name>Broadband Reports Team Starfire</name>
 <total_credit>153402.872429</total_credit>
 <expavg_credit>503030.483254</expavg_credit>
 <expavg_time>1087542007.701900</expavg_time>
 <nusers>14</nusers>
 <create_time>0</create_time>
<name_html>%3Ca%20href%3D%27http%3A%2F%2Fbroadbandreports%2Ecom%2Fforum%2Fseti%2
7%3E%3Cimg%20src%3D%27http%3A%2F%2Fi%2Edslr%2Enet%2Fpics%2Ffaqs%2Fimage2067%2Ejp
g%27%3E</name_html>
 <country>None</country>
 <user>
  <id>12</id>
  <name>John Keck</name>
  <total_credit>42698.813543</total_credit>
  <expavg_credit>117348.653646</expavg_credit>
  <expavg_time>1087542007.701900</expavg_time>
  <teamid>5</teamid>
 </user>
 <user>
  <id>14</id>
  <name>Liontaur</name>
  <total_credit>46389.595430</total_credit>
  <expavg_credit>122936.372641</expavg_credit>
  <expavg_time>1087542007.701900</expavg_time>
  <teamid>5</teamid>
 </user>
</team>
"),"</pre>
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
<b>User detail</b>
<pre>",
htmlspecialchars("
<user>
 <id>3</id>
 <name>Eric Heien</name>
 <total_credit>4897.904591</total_credit>
 <expavg_credit>9820.631754</expavg_credit>
 <expavg_time>1087542007.701900</expavg_time>
 <country>United States</country>
 <create_time>1046220857</ncreate_time>
 [ <teamid>14</teamid> ]
 [ <has_profile/> ]
 <host>
    <id>27</id>
    <total_credit>0.000000</total_credit>
    <expavg_credit>0.000000</expavg_credit>
    <expavg_time>1087542007.701900</expavg_time>
    <p_vendor></p_vendor>
    <p_model></p_model>
    <os_name>Darwin</os_name>
    <os_version>6.2</os_version>
 </host>
 <host>
    <id>266</id>
    <total_credit>0.000000</total_credit>
    <expavg_credit>0.000000</expavg_credit>
    <expavg_time>1087542007.701900</expavg_time>
    <p_vendor>GenuineIntel</p_vendor>
    <p_model>Intel(R)</p_model>
    <os_name>Linux</os_name>
    <os_version>2.4.18-18.7.x</os_version>
 </host>
</user>
"),"</pre>
<p>
<b>Host summary</b>
<pre>",
htmlspecialchars("
<host>
  <id>266</id>
  <total_credit>0.000000</total_credit>
  <expavg_credit>0.000000</expavg_credit>
  <expavg_time>1087542007.701900</expavg_time>
  <p_vendor>GenuineIntel</p_vendor>
  <p_model>Intel(R)</p_model>
  <os_name>Linux</os_name>
  <os_version>2.4.18-18.7.x</os_version>
</host>
"),"</pre>
<p>
<b>Host detail</b>
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
";
page_tail();
?>
