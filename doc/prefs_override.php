<?php

require_once("docutil.php");
page_head("Preferences override file");
echo "
By default, <a href=prefs.php>global preferences</a>
are maintained on a project server,
edited via a web interface, and downloaded from the server.

<p>
Some people may want to edit preferences locally,
or modify preferences on a single host.
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
    <run_if_user_active>0</run_if_user_active>
    <idle_time_to_run>3</idle_time_to_run>
    <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
    <work_buf_min_days>0.1</work_buf_min_days>
    <max_cpus>2</max_cpus>
    <disk_interval>60</disk_interval>
    <disk_max_used_gb>100</disk_max_used_gb>
    <disk_max_used_pct>50</disk_max_used_pct>
    <disk_min_free_gb>0.1</disk_min_free_gb>
    <vm_max_used_pct>75</vm_max_used_pct>
    <max_bytes_sec_down>0</max_bytes_sec_down>
    <max_bytes_sec_up>0</max_bytes_sec_up>
</global_preferences>
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
