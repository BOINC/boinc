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
    The standard policy is no checkboxes enabled.
    </b>

    <p>
    Server does EDF simulation based on current workload? <input type=checkbox name=suw>
    <p>
    Client uses Round-Robin (old-style) CPU scheduling? <input type=checkbox name=rr_only>
    <p>
    <input type=submit name=submit value=\"Run simulation\">

    </form>
    ";
}

// ?? the actual function doesn't seem to work here
function file_put_contents_aux($fname, $str) {
    $f = fopen($fname, "w");
    if (!$f) die("fopen");
    $x = fwrite($f, $str);
    if (!$x) die("fwrite");
    fclose($f);
}

if ($_POST['submit']) {
    chdir("sim");

    $x = $_POST['client_state'];
    if (!strlen($x)) {
        die("missing state");
    }
    $state_fname = tempnam("/tmp", "sim");
    file_put_contents_aux($state_fname, $x);

    $prefs_name = null;
    $config_name = null;

    $x = $_POST['global_prefs'];
    if (strlen($x)) {
        $prefs_fname = tempnam("/tmp", "sim");
        file_put_contents_aux($prefs_fname, $x);
    }

    $x = $_POST['cc_config'];
    if (strlen($x)) {
        $config_fname = tempnam("/tmp", "sim");
        file_put_contents_aux($config_fname, $x);
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

    $timeline_fname = tempnam("/tmp", "sim");
    $log_fname = tempnam("/tmp", "sim");
    $summary_fname = tempnam("/tmp", "sim");

    $cmd = "./sim --duration $duration --delta $delta $suw --state_file $state_fname --timeline_file $timeline_fname --log_file $log_fname --summary_file $summary_fname $rr_only $llflag";
    if ($prefs_fname) {
        $cmd .= " --prefs_file $prefs_fname";
    }
    if ($config_fname) {
        $cmd .= " --config_file $config_fname";
    }
    echo "cmd: $cmd\n";

    $x = system($cmd);

    echo $x;
    readfile($timeline_fname);
    echo "\n<pre>\n";
    readfile($log_fname);
    echo "\n</pre>\n";
    readfile($summary_fname);

    unlink($state_fname);
    unlink($prefs_fname);
    unlink($config_fname);
    unlink($timeline_fname);
    unlink($log_fname);
    unlink($summary_fname);
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
