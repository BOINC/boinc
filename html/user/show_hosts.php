<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");
    require_once("user.inc");

function show_host($host) {

    echo TABLE2."\n";
    echo "<tr>Below is information on all the hosts you have running the BOINC core client.";
    echo "<p>";
    echo "<tr>".TD2.LG_FONT."<b>Host Information:</b></font></td></tr>\n";
    row("<b>IP address:   </b>", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
    row("<b>Domain name:   <b>", $host->domain_name);
    $x = $host->timezone/3600;
    row("<b>Time zone:   </b>", "UTC - $x hours");
    row("<b>Created:   </b>", time_str($host->create_time));
    row("<b>Total Credit:</b>", $host->total_credit);
    row("<b>Recent average credit per day:</b>", $host->expavg_credit);
    row("<b>CPU:   </b>", "$host->p_vendor $host->p_model");
    row("<b>Number of CPUs:   </b>", $host->p_ncpus);
    row("<b>Operating System:   </b>", "$host->os_name $host->os_version");
    $x = $host->m_nbytes/(1024*1024);
    $y = round($x, 2);
    row("<b>Memory:   </b>", "$y MB");
    $x = $host->m_cache/1024;
    $y = round($x, 2);
    row("<b>Cache:   </b>", "$y KB");
    $x = $host->m_swap/(1024*1024);
    $y = round($x, 2);
    row("<b>Swap Space:   </b>", "$y MB");
    $x = $host->d_total/(1024*1024*1024);
    $y = round($x, 2);
    row("<b>Total Disk Space:   </b>", "$y GB");
    $x = $host->d_free/(1024*1024*1024);
    $y = round($x, 2);
    row("<b>Free Disk Space:   </b>", "$y GB");
    row("<b>Avg network bandwidth:<br>(upstream)   </b>", "$host->n_bwup bytes/sec");
    row("<b>Avg network bandwidth:<br>(downstream)   </b>", "$host->n_bwdown bytes/sec");
    row("<b>Number of times client has contacted server:   </b>", $host->rpc_seqno);
    row("<b>Last time contacted server:   </b>", time_str($host->rpc_time));
    row("<b>% of time client on:   </b>", 100*$host->on_frac." %");
    row("<b>% of time host connected:   </b>", 100*$host->connected_frac." %");
    row("<b>% of time user active:   </b>", 100*$host->active_frac." %");
    echo "</table>\n";

}
    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    if ($user) {
        page_head("Hosts stats");
        show_hosts($user);
        page_tail();
    } else {
        print_login_form();
    }
?>
