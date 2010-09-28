<?php

require_once("docutil.php");

function show_form() {
    echo "
    <form action=sim_form.php method=post>

    <b>client_state.xml</b>
    <br>
    <textarea name=client_state rows=10 cols=80>
</textarea>

    <p>
    <b>global_prefs.xml:</b> (the host's preferences)
    <br>
    <textarea name=global_prefs rows=10 cols=80>
</textarea>

    <p>
    <b>cc_config.xml:</b> (the client configuration options)
    <br>
    <textarea name=cc_config rows=10 cols=80>
</textarea>

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

    if (!file_put_contents("client_state.xml", $_POST['client_state'])) {
        echo "Can't write client_state.xml - check permissions\n"; exit();
    }
    if (!file_put_contents("global_prefs.xml", $_POST['global_prefs'])) {
        echo "Can't write global_prefs.xml - check permissions\n"; exit();
    }
    if (!file_put_contents("cc_config.xml", $_POST['cc_config'])) {
        echo "Can't write cc_config.xml - check permissions\n"; exit();
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
