<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("user.inc");

function show_host($host) {

    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>Host Information:</b></font></td></tr>\n";
    row2("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
    row2("Domain name", $host->domain_name);
    $x = $host->timezone/3600;
    row2("Time zone", "UTC - $x hours");
    row2("Created", time_str($host->create_time));
    row2("Total Credit", $host->total_credit);
    row2("Recent average credit", $host->expavg_credit);
    row2("CPU type", "$host->p_vendor $host->p_model");
    row2("Number of CPUs", $host->p_ncpus);
    row2("Operating System", "$host->os_name $host->os_version");
    $x = $host->m_nbytes/(1024*1024);
    $y = round($x, 2);
    row2("Memory", "$y MB");
    $x = $host->m_cache/1024;
    $y = round($x, 2);
    row2("Cache", "$y KB");
    $x = $host->m_swap/(1024*1024);
    $y = round($x, 2);
    row2("Swap space", "$y MB");
    $x = $host->d_total/(1024*1024*1024);
    $y = round($x, 2);
    row2("Total disk space", "$y GB");
    $x = $host->d_free/(1024*1024*1024);
    $y = round($x, 2);
    row2("Free Disk Space", "$y GB");
    $x = $host->p_fpops/(1000*1000);
    $y = round($x, 2);
    row2("Measured floating point speed", "$y million ops/sec");
    $x = $host->p_iops/(1000*1000);
    $y = round($x, 2);
    row2("Measured integer speed", "$y million ops/sec");
    $x = $host->p_membw/(1024*1024);
    $y = round($x, 2);
    row2("Measured memory bandwidth", "$y MB/sec");
    $x = $host->n_bwup/(1024);
    $y = round($x, 2);
    if ($y > 0) {
        row2("Average upload speed", "$y KB/sec");
    } else {
        row2("Average upload speed", "Unknown");
    }
    $x = $host->n_bwdown/(1024);
    $y = round($x, 2);
    if ($y > 0) {
        row2("Average download speed", "$y KB/sec");
    } else {
        row2("Average download speed", "Unknown");
    }
    row2("Number of times client has contacted server", $host->rpc_seqno);
    row2("Last time contacted server", time_str($host->rpc_time));
    row2("% of time client on", 100*$host->on_frac." %");
    row2("% of time host connected", 100*$host->connected_frac." %");
    row2("% of time user active", 100*$host->active_frac." %");
    echo "</table>\n";

}
    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    $hostid = $HTTP_GET_VARS["hostid"];
    if ($user && $hostid) {
        page_head("Host stats");

        $result = mysql_query("select * from host where id = $hostid");
        $host = mysql_fetch_object($result);
        mysql_free_result($result);

        if ($host) {
            if ($host->userid != $user->id) {
                echo "Not your host\n";
            } else {
                show_host($host);
            }
        } else {
            echo "Couldn't find host or user.<p>";
        }
        page_tail();
    } else {
        print_login_form();
    }
?>
