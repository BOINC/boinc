<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");

start_cache(600);

require_once("../inc/db.inc");

function count_estimate($query) {
    $result = mysql_query("explain $query");
    $x = mysql_fetch_object($result);
    return $x->rows;
}

function daemon_status($host, $pidname, $progname) {
    $path = "../../pid_$host/$pidname.pid";
    $running = false;
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            $foo = exec("/opt/misc/bin/ssh $host ps w $pid");
            if ($foo) {
                if (strstr($foo, $progname)) {
                    $running = true;
                }
            }
        }
    }
    return $running;
    if ($running) {
        echo "<br>$pidname is running on $host\n";
    } else {
        echo "<br>$pidname is not running on $host\n";
    }
}

function show_status($host, $function, $running) {
    echo "<tr><td>$function</td><td>$host</td>";
    if ($running) {
        echo "<td bgcolor=00ff00>Running</td>\n";
    } else {
        echo "<td bgcolor=ff0000>Not running</td>\n";
    }
}

function show_daemon_status($host, $pidname, $progname) {
    $running = daemon_status($host, $pidname, $progname);
    show_status($host, $pidname, $running);
}

page_head("SETI@home status page");
echo "
    <h2>Server status</h2>
    <table border=2 cellpadding=6>
    <tr><th>Program</th><th>Host</th><th>Status</th></tr>
";

$web_running = !file_exists("../../stop_web");
show_status("klaatu", "Data-driven web pages", $web_running);

$sched_running = !file_exists("../../stop_sched");
show_status("klaatu", "Scheduler", $sched_running);

show_daemon_status("kryten", "feeder", "feeder");
show_daemon_status("koloth", "file_deleter", "file_deleter");
show_daemon_status("klaatu", "transitioner1", "transitioner");
show_daemon_status("klaatu", "transitioner2", "transitioner");
show_daemon_status("kryten", "transitioner3", "transitioner");
show_daemon_status("kryten", "transitioner4", "transitioner");
show_daemon_status("klaatu", "sah_validate", "sah_validate");
show_daemon_status("galileo", "sah_assimilator", "sah_assimilator");
show_daemon_status("galileo", "sah_splitter", "sah_splitter");
show_daemon_status("milkyway", "sah_splitter2", "sah_splitter");
show_daemon_status("philmor", "sah_splitter3", "sah_splitter");
echo "
    </table>

    <h2>Database status</h2>
";

$retval = db_init_aux();
if ($retval) {
    echo "The database server is not accessable";
} else {
    echo "
        <table border=2 cellpadding=6>
        <tr><th>State</th><th>Approximate #results</th></tr>
    ";
    $n = count_estimate("select count(*) from result where server_state=2");
    echo "
        <tr><td>Ready to send</td><td>".number_format($n)."</td></tr>
    ";
    $n = count_estimate("select count(*) from result where server_state=4");
    echo "
        <tr><td>In progress</td><td>".number_format($n)."</td></tr>
        </table>
    ";
}

page_tail(true);
end_cache(600);

?>
