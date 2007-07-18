<?php

require_once("docutil.php");

function show_form() {
    echo "
    <form action=sim_form.php method=post>

    <b>sim_projects.xml</b> (describes a set of projects)
    <br>
    <textarea name=projects rows=10 cols=80><projects>
    <project>
        <project_name>P1</project_name>
        <resource_share>100</resource_share>
        <app>
            <latency_bound>15000</latency_bound>
            <fpops_est>3600</fpops_est>
            <fpops>
                <mean>3600</mean>
                <stdev>10</stdev>
            </fpops>
            <working_set>1e7</working_set>
        </app>
        <available>
            <frac>.7</frac>
            <lambda>1000</lambda>
        </available>
    </project>
    <project>
        <project_name>P2</project_name>
        <resource_share>50</resource_share>
        <app>
            <latency_bound>10000</latency_bound>
            <fpops_est>1800</fpops_est>
            <fpops>
                <mean>1800</mean>
                <stdev>10</stdev>
            </fpops>
            <working_set>1e7</working_set>
        </app>
        <available>
            <frac>.7</frac>
            <lambda>1000</lambda>
        </available>
    </project>
</projects></textarea>

    <p>
    <b>sim_host.xml</b>: (describes the simulated host)
    <br>
    <textarea name=host rows=10 cols=80><host>
    <p_fpops>1</p_fpops>
    <m_nbytes>1e9</m_nbytes>
    <connection_interval>600</connection_interval>
    <p_ncpus>2</p_ncpus>
    <available>
        <frac>.8</frac>
        <lambda>1000</lambda>
    </available>
</host></textarea>

    <p>
    <b>sim_prefs.xml:</b> (the host's preferences)
    <br>
    <textarea name=prefs rows=10 cols=80><global_preferences>
    <source_project>http://isaac.ssl.berkeley.edu/alpha/</source_project>
    <source_scheduler>isaac.ssl.berkeley.edu/alpha_cgi/cgi</source_scheduler>
<mod_time>1170192285</mod_time>
<run_if_user_active/>
<idle_time_to_run>3</idle_time_to_run>
<leave_apps_in_memory/>
<cpu_scheduling_period_minutes>1</cpu_scheduling_period_minutes>
<hangup_if_dialed/>
<work_buf_min_days>0.1</work_buf_min_days>
<work_buf_additional_days>0</work_buf_additional_days>
<max_cpus>4</max_cpus>
<cpu_usage_limit>100</cpu_usage_limit>
<disk_interval>180</disk_interval>
<disk_max_used_gb>100</disk_max_used_gb>
<disk_max_used_pct>50</disk_max_used_pct>
<disk_min_free_gb>2</disk_min_free_gb>
<vm_max_used_pct>75</vm_max_used_pct>
<ram_max_used_busy_pct>50</ram_max_used_busy_pct>
<ram_max_used_idle_pct>90</ram_max_used_idle_pct>
<max_bytes_sec_down>200000</max_bytes_sec_down>
<max_bytes_sec_up>200000</max_bytes_sec_up>
</global_preferences></textarea>

    <p>
    <b>cc_config.xml:</b> (the client configuration options)
    <br>
    <textarea name=cc_config rows=10 cols=80><cc_config>
    <log_flags>
        <mem_usage_debug>0</mem_usage_debug>
        <cpu_sched_debug>1</cpu_sched_debug>
        <cpu_sched>1</cpu_sched>
        <rr_simulation>0</rr_simulation>
        <benchmark_debug>1</benchmark_debug>
        <task_debug>1</task_debug>
        <work_fetch_debug>1</work_fetch_debug>
        <app_msg_send>0</app_msg_send>
        <unparsed_xml/>
    </log_flags>
</cc_config></textarea>

    <p>
    <b>
    The following control how long the simulation runs.
    Duration may not exceed TimeStep*10000.
    </b>
    <br>Time step: <input name=delta value=60>
    <br>Duration: <input name=duration value=86400>
    <p>
    <b>
    The following controls enable various experimental policies.
    The standard policy (as of 5.10.13) is no checkboxes enabled,
    and the 'Normal' DCF policy.
    </b>

    <p>
    Server does EDF simulation based on current workload? <input type=checkbox name=suw>
    <p>
    Client uses Round-Robin (old-style) CPU scheduling? <input type=checkbox name=rr_only>
    <p>
    Client uses old work fetch policy? <input type=checkbox name=work_fetch_old>
    <p>
    Duration correction factor: <input type=radio name=dcf value=normal checked> Normal
        : <input type=radio name=dcf value=stats> Stats
        : <input type=radio name=dcf value=dual> Dual
        : <input type=radio name=dcf value=none> None
    <p>
    HTML output lines per file: <input name=line_limit>
    <p>
    <input type=submit name=submit value=\"Run simulation\">

    </form>
    ";
}

if ($_POST['submit']) {
    chdir("sim");

    $x1 = file_put_contents("sim_projects.xml", $_POST['projects']);
    $x2 = file_put_contents("sim_host.xml", $_POST['host']);
    $x3 = file_put_contents("sim_prefs.xml", $_POST['prefs']);
    $x4 = file_put_contents("cc_config.xml", $_POST['cc_config']);
    if (!$x1 || !$x2 || !$x3 || !$x4) {
        echo "Can't write files - check permissions\n";
        exit();
    }
    $duration = $_POST['duration'];

    $delta = $_POST['delta'];
    if ($delta < 1) {
        echo "time step must be >= 1";
        exit();
    }

    if ($duration/$delta > 10000) {
        echo "duration/step must be <= 10000";
        exit();
    }

    $suw = '';
    if ($_POST['suw']) {
        $suw = '--server_uses_workload';
    }

    $rr_only = '';
    if ($_POST['rr_only']) {
        $rr_only = '--cpu_sched_rr_only';
    }
    $work_fetch_old = '';
    if ($_POST['work_fetch_old']) {
        $work_fetch_old = '--work_fetch_old';
    }

    $dcfflag = "";
    $dcf = ($_POST['dcf']);
    if ($dcf == "stats") {
        $dcfflag = '--dcf_stats';
    } else if ($dcf == 'none') {
        $dcfflag = '--dcf_dont_use';
    } else if ($dcf == 'dual') {
        $dcfflag = '--dual_dcf';
    }

    $llflag = '';
    $line_limit = $_POST['line_limit'];
    if ($line_limit) {
        $llflag = "--line_limit $line_limit";
    }

    Header("Location: sim/sim_out_0.html");
    $cmd = "./sim --duration $duration --delta $delta $suw $dcfflag $rr_only $work_fetch_old $llflag";
    system("/bin/rm sim_log.txt sim_out_*.html");
    system($cmd);
} else {
    page_head("BOINC client simulator");
    echo "
        This is a web interface to the BOINC client simulator.
        Fill in the following form to specify the
        <a href=trac/wiki/ClientSim>parameters of your simulation</a>.
        The results will be shown in your browser.
        <p>
    ";
    show_form();
}

?>
