<?php
require_once("docutil.php");
page_head("Aggregate statistics data");
echo "
The following are proposed formats for project statistics data
to be distributed by aggregators.
Much of this has been implemented by James Drews;
the export files are at
<p>
<a href=http://boinc.netsoft-online.com/stats/>http://boinc.netsoft-online.com/stats/</a>
<p>
and the web RPCs for hosts are at
<p>
<a href=http://boinc.netsoft-online.com/get_host.php?cpid=????>http://boinc.netsoft-online.com/get_host.php?cpid=????</a>
<p>
(based on the host with the highest expavg_credit) and
<p>
http://boinc.netsoft-online.com/get_host_tot.php?cpid=????
<p>
(based on the host with the highest total_credit).
<p>
http://boinc.netsoft-online.com/get_user.php?cpid=????
<p>
(returns info on the user with given CPID).

<h2>Files</h2>
<h3>user_total_credit.xml</h3>
A list of top users, ordered by decreasing cross-project total credit.
Should include at least top 100 users.
";
echo html_text("
    <users>
        <nusers>100</nusers>
        <time>1129844599</time>   (when this file was generated)
        <total_credit>4983146120.8639</total_credit> (summed over all users)
        <expavg_credit>17827453.354517</expavg_credit>
        <user>
            <name>John Keck</name>
            <total_credit>42698.813543</total_credit>
            <expavg_credit>117348.653646</expavg_credit>
            <user_cpid>283472938743489759837498347</user_cpid>
            <world_rank>1</world_rank>
            <project>
                <name>SETI@home</name>
                <url>http://setiathome.berkeley.edu</url>
                <total_credit>2698.813543</total_credit>
                <expavg_credit>17348.653646</expavg_credit>
                <id>123</id>
                <country>United States</country>
                <user_name>John Keck</user_name>
                <create_time>1104210399</create_time>
                <team_name>My Team</team_name>
                <team_id>44</team_id>
                <user_url>foobar.example.com</user_url>
                <computer_count>44</computer_count>
            </project>
            ... other projects
        </user>
        ... other users
    </users>
");
echo "
<h3>user_expavg_credit.xml</h3>
A list of top users, ordered by decreasing cross-project recent average credit.
Should include at least top 100 users.
Same format as above.

<h3>host_total_credit.xml</h3>
";
echo html_text("
    <hosts>
        <nhosts>100</nhosts>
        <time>1129844599</time>   (when this file was generated)
        <host>
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
            <host_cpid>xxxxx</host_cpid>
            <user_cpid>xxxxx</user_cpid>
            <project>
                <name>SETI@home</name>
                <url>http://setiathome.berkeley.edu</url>
                <total_credit>2698.813543</total_credit>
                <expavg_credit>17348.653646</expavg_credit>
                <id>123</id>
            </project>
            ... other projects
        </host>
        ... other hosts
    </hosts>
");
echo "
<h3>host_expavg_credit.xml</h3>
<p>
Same, ordered by decreasing recent average credit.

<a name=rpc></a>

<h2>Web RPCs</h2>
<h3>get_user.php?cpid=xxxxx</h3>
<p>
";
echo html_text("
    <user>
        <name>John Keck</name>
        <total_credit>42698.813543</total_credit>
        <expavg_credit>117348.653646</expavg_credit>
        <cpid>283472938743489759837498347</cpid>
        <project>
            <name>SETI@home</name>
            <url>http://setiathome.berkeley.edu</url>
            <total_credit>2698.813543</total_credit>
            <expavg_credit>17348.653646</expavg_credit>
        </project>
        ... other projects
    </user>
");
echo "
<h3>get_host.php?cpid=xxxxx</h3>
";
echo html_text("
    <host>
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
        <host_cpid>xxxxx</host_cpid>
        <user_cpid>xxxxx</user_cpid>
        <project>
            <name>SETI@home</name>
            <url>http://setiathome.berkeley.edu</url>
            <total_credit>2698.813543</total_credit>
            <expavg_credit>17348.653646</expavg_credit>
        </project>
        ... other projects
    </host>
");
echo "
";
page_tail();
?>
