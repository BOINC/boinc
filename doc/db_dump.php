<?
require_once("docutil.php");
page_head("Downloadable statistics data");
echo "

<p>
BOINC projects may export data describing teams, users and hosts.
This data is exported in XML files that can be downloaded via HTTP.
<p>
This data can be summarized and represented as Web pages.
Some examples are listed at
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats.php>
http://setiboinc.ssl.berkeley.edu/ap/stats.php</a>.

<p>
The data is presented in several different 'views':
teams ordered by credit, teams ordered by ID, etc.
Each view is available in two ways:
<ul>
<li> As a single file.
<li>
Broken into a number of files,
each containing a fixed number of records.
This lets you get a single record or range of records efficiently.
</ul>
<p>
For files that are ordered by ID,
each file contains a fixed-size segment of the ID range,
not a fixed number of records.
If the database ID allocation has gaps,
files will have fewer than this number of records.
<p>
The entries in a given file are in either 'summary' or 'detail' form.
For example, the summary of a team gives its ID, name, and credit,
while the detailed from also contains a list of its members.
<p>
The files are as follows:

<p>
<b>tables.xml</b>
<p>
For each table (team, user, and host) this gives
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
<b>team_total_credit.xml, team_total_credit_N.xml</b>
<br>
Team summaries, ordered by decreasing <a href=credit.php>total credit</a><.
N is 0, 1, ...
<p>
<b>team_expavg_credit.xml, team_expavg_credit_N.xml</b>
<br>
Team summaries, ordered by decreasing <a href=credit.php>recent-average credit</a>.
<p>
<b>team_id.xml, team_id_N.xml</b>
<br>
Team details, ordered by increasing ID.
<p>
<b>user_total_credit.xml, user_total_credit_N.xml</b>
<br>
User summaries, ordered by decreasing total credit.
<p>
<b>user_expavg_credit.xml, user_expavg_credit_N.xml</b>
<br>
User summaries, ordered by decreasing recent-average credit.
<p>
<b>user_id.xml, user_id_N.xml</b>
<br>
User details, ordered by increasing ID.
<p>
<b>host_total_credit.xml, host_total_credit_N.xml</b>
<br>
Host summaries, ordered by decreasing total credit.
<p>
<b>host_expavg_credit.xml, host_expavg_credit_N.xml</b>
<br>
Host summaries, ordered by decreasing recent-average credit.
<p>
<b>host_id.xml, host_id_N.xml</b>
<br>
Host details, ordered by increasing ID.
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
