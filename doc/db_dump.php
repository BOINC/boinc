<?
require_once("docutil.php");
page_head("Downloadable statistics data");
echo "

<p>
BOINC projects can export data describing teams, users and hosts.
This data is exported in downloadable XML files,
and can be summarized and represented as Web pages.
Some examples are listed at
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats.php>
http://setiboinc.ssl.berkeley.edu/ap/stats.php</a>.

<p>
The data is presented in several 'views':
teams ordered by credit, teams ordered by ID, etc.
Each view is available in two forms:
<ul>
<li> As a single file.
<li>
Divided into a number of files,
each containing a limited number of records.
This lets you get a single record or range of records efficiently.
For views that are ordered by ID,
each file contains a fixed-size segment of the ID range,
not a fixed number of records.
If the database ID allocation has gaps,
files will have fewer than this number of records.
</ul>
<p>
The entries in a given file are in either 'summary' or 'detail' form.
For example, the summary of a team gives its ID, name, and credit,
while the detailed form also contains a list of its members.
<p>
The files are as follows:

<p>
<b>tables.xml</b>
<p>
For each entity type (team, user, and host) this gives
<ul>
<li> the total number of records
<li> the number of records per file for summary files
<li> the number of records per file for detail files
</ul>
It also includes the UNIX time when the files were last generated,
and a list of the project's applications,
with counts of results in various states.
<br>
For example:
<pre>",
htmlspecialchars("<tables>
    <update_time>1046220857</update_time>
    <nusers_total>127</nusers_total>
    <nusers_per_file_summary>1000</nusers_per_file_summary>
    <nusers_per_file_detail>100</nusers_per_file_detail>
    <nteams_total>14</nteams_total>
    <nteams_per_file_summary>1000</nteams_per_file_summary>
    <nteams_per_file_detail>100</nteams_per_file_detail>
    <nhosts_total>681</nhosts_total>
    <nhosts_per_file_summary>1000</nhosts_per_file_summary>
    <nhosts_per_file_detail>100</nhosts_per_file_detail>
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
"),
"</pre>
";
list_start();
list_item(
"team_total_credit.xml<br> team_total_credit_N.xml",
"Team summaries, ordered by decreasing <a href=credit.php>total credit</a>.
The first file is the complete list;
the remaining files (for N = 0, 1, ...) is the list
in limited-size chunks."
);
list_item("team_expavg_credit.xml<br> team_expavg_credit_N.xml",
"Team summaries, ordered by decreasing <a href=credit.php>recent-average credit</a>.");
list_item("team_id.xml<br> team_id_N.xml",
"Team details, ordered by increasing ID.");
list_item("user_total_credit.xml<br> user_total_credit_N.xml",
"User summaries, ordered by decreasing total credit.");
list_item("user_expavg_credit.xml<br> user_expavg_credit_N.xml",
"User summaries, ordered by decreasing recent-average credit.");
list_item("user_id.xml, user_id_N.xml",
"User details, ordered by increasing ID.");
list_item("host_total_credit.xml<br> host_total_credit_N.xml",
"Host summaries, ordered by decreasing total credit.");
list_item("host_expavg_credit.xml<br> host_expavg_credit_N.xml",
"Host summaries, ordered by decreasing recent-average credit.");
list_item("host_id.xml<br> host_id_N.xml",
"Host details, ordered by increasing ID.");
list_end();
echo "
<p>
The format of the various XML elements is as follows:

<p>
<b>Team summary</b>
<pre>",
htmlspecialchars("
<team>
 <id>5</id>
 <name>Broadband Reports Team Starfire</name>
 <total_credit>153402.872429</total_credit>
 <expavg_credit>503030.483254</expavg_credit>
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
  <teamid>5</teamid>
 </user>
 <user>
  <id>14</id>
  <name>Liontaur</name>
  <total_credit>46389.595430</total_credit>
  <expavg_credit>122936.372641</expavg_credit>
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
 <cpid>283472938743489759837498347</cpid>
 [ <teamid>5</teamid> ]
 [ <has_profile/> ]
</user>
"), "</pre>
<p>
Note: &lt;cpid&gt; ('<a href=cpid.php>cross-project identifier</a>')
is a unique identifier across multiple projects.
Accounts with the same email address on different projects
will have the same cross-project identifier
(as long as at least one computer is attached to both accounts).
<p>
<b>User detail</b>
<pre>",
htmlspecialchars("
<user>
 <id>3</id>
 <name>Eric Heien</name>
 <total_credit>4897.904591</total_credit>
 <expavg_credit>9820.631754</expavg_credit>
 <country>United States</country>
 <create_time>1046220857</ncreate_time>
 [ <teamid>14</teamid> ]
 [ <has_profile/> ]
 <host>
    <id>27</id>
    <total_credit>0.000000</total_credit>
    <expavg_credit>0.000000</expavg_credit>
    <p_vendor></p_vendor>
    <p_model></p_model>
    <os_name>Darwin</os_name>
    <os_version>6.2</os_version>
 </host>
 <host>
    <id>266</id>
    <total_credit>0.000000</total_credit>
    <expavg_credit>0.000000</expavg_credit>
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
</host>
"),"</pre>
";
page_tail();
?>
