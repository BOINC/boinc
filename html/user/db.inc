<?php

function db_init() {
    mysql_pconnect();
    mysql_select_db("boinc");
}

function lookup_user_auth($auth) {
    $result = mysql_query("select * from user where authenticator='$auth'");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    return $user;
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
    start_table();
    row("ID", $host->id);
    row("created", time_str($host->create_time));
    row("user ID", $host->userid);
    row("prefs ID", $host->prefsid);
    row("RPC seqno", $host->rpc_seqno);
    row("RPC time", time_str($host->rpc_time));
    echo "<tr><td align=right>Domain name</td><td>$host->domain_name</td></tr>\n";
    $x = $host->timezone/3600;
    echo "<tr><td align=right>Time zone</td><td>UTC - $x hours</td></tr>\n";
    echo "<tr><td align=right>IP address</td><td>$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)</td></tr>\n";
    echo "<tr><td align=right>On fraction</td><td>$host->on_frac</td></tr>\n";
    echo "<tr><td align=right>Connected fraction</td><td>$host->connected_frac</td></tr>\n";
    echo "<tr><td align=right>Active fraction</td><td>$host->active_frac</td></tr>\n";
    echo "<tr><td align=right># CPUs</td><td>$host->p_ncpus</td></tr>\n";
    echo "<tr><td align=right>CPU vendor</td><td>$host->p_vendor</td></tr>\n";
    echo "<tr><td align=right>CPU model</td><td>$host->p_model</td></tr>\n";
    echo "<tr><td align=right>OS name</td><td>$host->os_name</td></tr>\n";
    echo "<tr><td align=right>OS version</td><td>$host->os_version</td></tr>\n";
    $x = $host->m_nbytes/(1024*1024);
    echo "<tr><td align=right>Memory</td><td>$x MB</td></tr>\n";
    $x = $host->m_cache/1024;
    echo "<tr><td align=right>Cache</td><td>$x KB</td></tr>\n";
    $x = $host->m_swap/(1024*1024);
    echo "<tr><td align=right>Swap space</td><td>$x MB</td></tr>\n";
    $x = $host->d_total/(1024*1024*1024);
    echo "<tr><td align=right>Disk, total</td><td>$x GB</td></tr>\n";
    $x = $host->d_free/(1024*1024*1024);
    echo "<tr><td align=right>Disk, free</td><td>$x GB</td></tr>\n";
    echo "<tr><td align=right>Average network bandwidth, upstream</td><td>$host->n_bwup bytes/sec</td></tr>\n";
    echo "<tr><td align=right>Average network bandwidth, downstream</td><td>$host->n_bwdown bytes/sec</td></tr>\n";
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

function show_user($user) {
    start_table();
    row("ID", $user->id);
    row("created", time_str($user->create_time));
    row("name", $user->name);
    row("email_addr", $user->email_addr);
    row("country", $user->country);
    row("postal_code", $user->postal_code);
    echo "</table>\n";
}

function show_prefs($prefs) {
    start_table();
    row("ID", $prefs->id);
    row("created", time_str($prefs->create_time));
    row("modified", time_str($prefs->modified_time));
    row("userid", $prefs->userid);
    row("XML doc", "<pre>".htmlspecialchars($prefs->xml_doc)."</pre>");
    echo "</table>\n";
}

?>
