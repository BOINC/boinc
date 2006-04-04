<?php
require_once("docutil.php");
page_head("Preferences implementation");
echo "
<h2>General preferences</h2>
General preferences are represented by an XML element of the form:
".html_text("
<global_preferences>
     <source_project>http://setiathome.berkeley.edu/</source_project>
     <source_scheduler>http://setiboinc.ssl.berkeley.edu/sah_cgi/cgi</source_scheduler>
    <mod_time>1140215730</mod_time>
    <run_if_user_active/>
    <idle_time_to_run>3</idle_time_to_run>
    <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
    <hangup_if_dialed/>
    <work_buf_min_days>0.1</work_buf_min_days>
    <max_cpus>2</max_cpus>
    <disk_interval>60</disk_interval>
    <disk_max_used_gb>100</disk_max_used_gb>
    <disk_max_used_pct>50</disk_max_used_pct>
    <disk_min_free_gb>0.1</disk_min_free_gb>
    <vm_max_used_pct>75</vm_max_used_pct>
    <max_bytes_sec_down>0</max_bytes_sec_down>
    <max_bytes_sec_up>0</max_bytes_sec_up>
    <venue name=\"home\">
        <idle_time_to_run>3</idle_time_to_run>
        <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
        <hangup_if_dialed/>
        <work_buf_min_days>0.1</work_buf_min_days>
        <max_cpus>2</max_cpus>
        <disk_interval>60</disk_interval>
        <disk_max_used_gb>100</disk_max_used_gb>
        <disk_max_used_pct>50</disk_max_used_pct>
        <disk_min_free_gb>0.001</disk_min_free_gb>
        <vm_max_used_pct>75</vm_max_used_pct>
        <max_bytes_sec_down>0</max_bytes_sec_down>
        <max_bytes_sec_up>0</max_bytes_sec_up>
    </venue>
</global_preferences>
")."
The first set of preferences are the default;
each set of venue-specific preferences is
enclosed in a &lt;venue> element.
The optional
&lt;source_project> and &lt;source_scheduler> elements indicate
the source of the preferences.

<p>
On the client, the global prefs are stored in global_prefs.xml
(present only if they've been obtained from a server).

<p>
A scheduler RPC request includes global_prefs.xml if it's present.
If the request includes global prefs,
the scheduler installs them in the DB if the DB copy is missing or older.
If the DB copy is newer, it includes it in the reply.
If the request has no prefs and they're present in the DB,
they're included in the reply.
<p>
When handling a scheduler RPC reply that includes global prefs,
the client writes them to global_prefs.xml,
prepending the project and scheduler URLs is absent.

<h2>Project preferences</h2>
Project preferences are represented by an XML element of the form:
".html_text("
<project_preferences>
    <resource_share>50</resource_share>
    <project_specific>
        <format_preset>Custom</format_preset>
        <text_style>Pillars</text_style>
        <graph_style>Rectangles</graph_style>
        <max_fps>30</max_fps>
        <max_cpu>50</max_cpu>
        <grow_time>10</grow_time>
        <hold_time>5</hold_time>
        <graph_alpha>0.7</graph_alpha>
        <roll_period>10</roll_period>
        <roll_range>20</roll_range>
        <pitch_period>30</pitch_period>
        <pitch_range>30</pitch_range>
        <starfield_size>2000</starfield_size>
        <starfield_speed>40</starfield_speed>
        <color_preset>Rainbow</color_preset>
        <start_hue>0</start_hue>
        <hue_change>1</hue_change>
        <app_file>
            <timestamp>1095011196</timestamp>
            <open_name>background</open_name>
            <url>http://setiweb.ssl.berkeley.edu/~davea/yosemite_08_04/800/P1010008.jpg</url>
        </app_file>
    </project_specific>
    <venue name=\"school\">
        <resource_share>100</resource_share>
        <project_specific>
            <format_preset>SETI@home classic</format_preset>
            <text_style>Pillars</text_style>
            <graph_style>Rectangles</graph_style>
            <max_fps>30</max_fps>
            <max_cpu>50</max_cpu>
            <grow_time>10</grow_time>
            <hold_time>5</hold_time>
            <graph_alpha>0.7</graph_alpha>
            <roll_period>10</roll_period>
            <roll_range>20</roll_range>
            <pitch_period>30</pitch_period>
            <pitch_range>30</pitch_range>
            <starfield_size>2000</starfield_size>
            <starfield_speed>40</starfield_speed>
            <color_preset>Rainbow</color_preset>
            <start_hue>0</start_hue>
            <hue_change>1</hue_change>
        </project_specific>
    </venue>
</project_preferences>
")."

On the client, the project prefs are stored
in a file account_XXX.xml,
which has the additional elements <lt;master_url&gt; and <lt;authenticator&gt;.
<p>
A scheduler RPC reply always includes project prefs.
<p>
When handling a scheduler RPC reply,
the client writes them to the account_X.xml file,
prepending the master URL and authenticator.

";
page_tail();
?>
