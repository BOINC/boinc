<?php

function db_init() {
    $retval = mysql_pconnect();
    if (!$retval) {
        exit();
    }
    $fp = fopen(db_name, "r");
    $db_name = fgets($fp, 1024);
    $db_name = rtrim($db_name);
    mysql_select_db($db_name);
    fclose($fp);
    return $db_name;
}

function lookup_user_auth($auth) {
    $result = mysql_query("select * from user where authenticator='$auth'");
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        return $user;
    } else {
        return 0;
    }
}

function show_platform($platform) {
    start_table();
    row("ID", $platform->id);
    row("created", time_str($platform->create_time));
    row("name", $platform->name);
    echo "</table>";
}

function show_app($app) {
    start_table();
    row("ID", $app->id);
    row("created", time_str($app->create_time));
    row("name", $app->name);
    row("alpha version", $app->alpha_vers);
    row("beta version", $app->beta_vers);
    row("production version", $app->prod_vers);
    row("result template", "<pre>".htmlspecialchars($app->result_xml_template)."</pre>");
    echo "</table>";
}

function show_app_version($app_version) {
    start_table();
    row("ID", $app_version->id);
    row("created", time_str($app_version->create_time));
    row("app ID", $app_version->appid);
    row("version num", $app_version->version_num);
    row("platform ID", $app_version->platformid);
    row("XML doc", "<pre>".htmlspecialchars($app_version->xml_doc)."</pre>");
    row("min_core_version", $app_version->min_core_version);
    row("max_core_version", $app_version->max_core_version);
    row("message", $app_version->message);
    row("deprecated", $app_version->deprecated);
    echo "</table>";
}

function show_host($host) {

    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>Host Information:</b></font></td></tr>\n";
    row("<b>IP address:   </b>", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
    row("<b>Domain name:   <b>", $host->domain_name);
    $x = $host->timezone/3600;
    row("<b>Time zone:   </b>", "UTC - $x hours");
    row("<b>Created:   </b>", time_str($host->create_time));
    row("<b>CPU:   </b>", "$host->p_vendor $host->p_model");
    row("<b>Number of CPUs:   </b>", $host->p_ncpus);
    row("<b>Operating System:   </b>", "$host->os_name $host->os_version");
    $x = $host->m_nbytes/(1024*1024);
    row("<b>Memory:   </b>", "$x MB");
    $x = $host->m_cache/1024;
    row("<b>Cache:   </b>", "$x KB");
    $x = $host->m_swap/(1024*1024);
    row("<b>Swap Space:   </b>", "$x MB");
    $x = $host->d_total/(1024*1024*1024);
    row("<b>Total Disk Space:   </b>", "$x GB");
    $x = $host->d_free/(1024*1024*1024);
    row("<b>Free Disk Space:   </b>", "$x GB");
    row("<b>Avg network bandwidth:<br>(upstream)   </b>", "$host->n_bwup bytes/sec");
    row("<b>Avg network bandwidth:<br>(downstream)   </b>", "$host->n_bwdown bytes/sec");
    row("<b>Number of times client has contacted server:   </b>", $host->rpc_seqno);
    row("<b>Last time contacted server:   </b>", time_str($host->rpc_time));
    row("<b>% of time client on:   </b>", 100*$host->on_frac." %");
    row("<b>% of time client connected:   </b>", 100*$host->connected_frac." %");
    row("<b>% of time client active:   </b>", 100*$host->active_frac." %");
    echo "</table>\n";


}

function show_workunit($wu) {
    start_table();
    row("created", time_str($wu->create_time));
    row("name", $wu->name);
    row("XML doc", "<pre>".htmlspecialchars($wu->xml_doc)."</pre>");
    row("app ID", $wu->appid);
    row("batch", $wu->batch);
    row("FP operations", $wu->rsc_fpops);
    row("integer operations", $wu->rsc_iops);
    row("memory requirement", $wu->rsc_memory);
    row("disk requirement", $wu->rsc_disk);
    row("dynamic results", $wu->dynamic_results?"yes":"no");
    row("max_results", $wu->max_results);
    row("nresults", $wu->nresults);
    row("nresults_unsent", $wu->nresults_unsent);
    row("nresults_done", $wu->nresults_done);
    row("nresults_fail", $wu->nresults_fail);
    echo "</table>";
}

function res_state_string($s) {
    switch($s) {
    case 1: return "inactive";
    case 2: return "unsent";
    case 3: return "in progress";
    case 4: return "done";
    case 5: return "timeout";
    case 6: return "error";
    }
}

function show_result($result) {
    start_table();
    row("created", time_str($result->create_time));
    row("name", $result->name);
    row("workunit ID", $result->workunitid);
    row("state", res_state_string($result->state));
    row("host ID", $result->hostid);
    row("sent", time_str($result->sent_time));
    row("received", time_str($result->received_time));
    row("exit status", $result->exit_status);
    row("CPU time", $result->cpu_time);
    row("XML doc in", "<pre>".htmlspecialchars($result->xml_doc_in)."</pre>");
    row("XML doc out", "<pre>".htmlspecialchars($result->xml_doc_out)."</pre>");
    row("stderr out", "<pre>".htmlspecialchars($result->stderr_out)."</pre>");
    row("batch", $result->batch);
    row("validated", $result->validated);
    echo "</table>";
}

function show_user_stats($user, $project) {
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User Stats:</b></font></td></tr>\n";
    row("<b>User ID:   </b>", $user->id);
    $row = sprintf("<b>%s user since:   </b>", $project);
    row($row, time_str($user->create_time));
    row("<b>Total credit:   </b>", $user->total_credit);
    row("<b>Recent averaged credit:   </b>", $user->expavg_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row("<b>Member of team:   </b>", "<a href=team_display.php?id=$team->id>$team->name</a>");
    } else {
        row("<b>Member of team:   </b>", "You are currently not part of a team");
    }
    echo "</table>\n";
}

function show_user_profile($user) {
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User Information:</b></font></td></tr>\n";
    row("<b>User name:   </b>", $user->name);
    row("<b>Email address:   <b>", $user->email_addr);
    row("<b>Country:   </b>", $user->country);
    row("<b>Postal (ZIP) code:   </b>", $user->postal_code);
    echo "<tr><td align=right><a href=edit_user_info.php>Edit User Information</a></td>\n";
    echo "<td align=center><a href=change_password.php>Change Password</a></td></tr>\n";
    echo "</table>\n";
}

function show_team($team) {
    start_table();
    row("ID", $team->id);
    row("founder", $team->userid);
    row("name", $team->name);
    row("url", $team->url);
    row("type", $team->type);
    row("name_html", $team->name_html);
    row("description", $team->description);
}

?>
