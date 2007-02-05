<?php

require_once("docutil.php");
page_head("Preferences override file");
echo "
By default, <a href=prefs.php>global preferences</a>
and host venue are maintained on a project server,
edited via a web interface, and downloaded from the server.

<p>
Some people may want to edit preferences locally,
modify preferences on a single host, or hardwire the host venue.
To accommodate these requirements, BOINC lets you
create a 'preferences override file'.
This file is read by the core client after
it reads the preferences from the server,
and it overrides those preferences.
<p>
The preferences override file is named <code>global_prefs_override.xml</code>.
Its structure as follows:
".html_text("
<global_preferences>
    <run_on_batteries>0</run_on_batteries>
    <run_if_user_active>0</run_if_user_active>
    <start_hour>0</start_hour>
    <end_hour>0</end_hour>
    <net_start_hour>0</net_start_hour>
    <net_end_hour>0</net_end_hour>
    <leave_apps_in_memory>0</leave_apps_in_memory>
    <confirm_before_connecting>0</confirm_before_connecting>
    <hangup_if_dialed>0</hangup_if_dialed>
    <work_buf_min_days>0.1</work_buf_min_days>
    <max_cpus>2</max_cpus>
    <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
    <disk_interval>60</disk_interval>
    <disk_max_used_gb>100</disk_max_used_gb>
    <disk_max_used_pct>50</disk_max_used_pct>
    <disk_min_free_gb>0.1</disk_min_free_gb>
    <vm_max_used_pct>75</vm_max_used_pct>
    <ram_max_used_busy_pct>50</ram_max_used_busy_pct>
    <ram_max_used_idle_pct>90</ram_max_used_idle_pct>
    <idle_time_to_run>3</idle_time_to_run>
    <max_bytes_sec_down>0</max_bytes_sec_down>
    <max_bytes_sec_up>0</max_bytes_sec_up>
    <cpu_usage_limit>100</cpu_usage_limit>
</global_preferences>
[ <host_venue>venue</host_venue> ]
")."

<p>
If you write a program for editing preferences locally,
you'll want to be able to tell a running core client to
reread the preferences override file.
You can do this using the
<a href=gui_rpc.php>read_global_prefs_override()</a> GUI RPC.
Or you can stop and start the core client.
";
page_tail();
?>
