<?php

function db_init() {
    $retval = mysql_pconnect();
    if (!$retval) {
        exit();
    }
    $fp = fopen("db_name", "r");
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

function print_query_count($query, $nres, $start) {
    $result = mysql_query($query);
    $res = mysql_fetch_object($result);
    if ($res->cnt < ($nres+$start)) {
        $nres = $res->cnt-$start;
    }
    printf(
        "<p>\n"
        . "$res->cnt database entries match the query.  Displaying $start to " . ($start+$nres) . "."
        . "<p>\n"
    );
}

function show_platform($platform) {
    start_table();
    row("ID", $platform->id);
    row("Created", time_str($platform->create_time));
    row("Platform Name", $platform->name);
    row("","<a href=app_version.php?plat_id=$platform->id>Application Versions for Platform $platform->name </a>");
    end_table();
}

function show_app($app) {
    start_table();
    row("ID", $app->id);
    row("Created", time_str($app->create_time));
    row("Application Name", $app->name);
    row("","<a href=app_version?app_id=$app->id>Application Versions for Application $app->name</a>");
    row("","<a href=workunit.php?app_id=$app->id>Workunits for this Application</a>");
    end_table();
}

function show_app_version($app_version,$show_xml_docs) {
    start_table();
    row("ID", $app_version->id);
    row("Created", time_str($app_version->create_time));
    row("Application", "<a href=app.php?id=$app_version->appid>" . app_name_by_id($app_version->appid) . "</a>");
    row("Version num", $app_version->version_num);
    row("Platform", "<a href=platform.php?id=$app_version->platformid>" . platform_name_by_id($app_version->platformid) . "</a>" );
    if ($show_xml_docs) {
        row("XML doc", "<pre>".htmlspecialchars($app_version->xml_doc)."</pre>");
    }
    row("Min_core_version", $app_version->min_core_version);
    row("Max_core_version", $app_version->max_core_version);
    row("Message", $app_version->message);
    row("Deprecated", $app_version->deprecated);
    end_table();
}

function show_host($host) {

    echo TABLE2."\n";
    echo "<tr>";
    echo "<p>";
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
    row("","<a href=result.php?host_id=$host->id>Results Assigned to Host</a>");
    end_table();

}

function show_workunit($wu,$show_xml_doc) {
    start_table();
    row4("Created", time_str($wu->create_time), "Name", $wu->name);
    if ($show_xml_doc) {
        row("XML doc", "<pre>".htmlspecialchars($wu->xml_doc)."</pre>");
    }
    row4("Application", "<a href=app.php?id=$wu->appid>" . app_name_by_id($wu->appid) . "</a>",
        "Batch", $wu->batch);
    row4("FP Operations", $wu->rsc_fpops, "Integer Operations", $wu->rsc_iops);
    row4("Memory Requirement", $wu->rsc_memory, "Disk Requirement", $wu->rsc_disk);
    row4("Dynamic Results", $wu->dynamic_results?"yes":"no", "Max Results", $wu->max_results);
    row4("Number of Results", $wu->nresults, "Number of Unsent Results", $wu->nresults_unsent);
    row4("Number of Results Done", $wu->nresults_done, "Number of Failed Results",
        $wu->nresults_fail);
    row("","<a href=result.php?wu_id=$wu->id>Show Associated Results</a>");
    end_table();
    echo "<p>";
}

function res_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 3: return "In Progress";
    case 4: return "Done";
    case 5: return "Timeout";
    case 6: return "Error";
    }
}

function show_result($result,$show_xml_docs,$show_stderr,$show_times) {
    start_table();
    if ($show_times) {
        row4("created", time_str($result->create_time), "sent", time_str($result->sent_time));
        row4("received", time_str($result->received_time),"","");
    }
    row4("Name", $result->name, "Workunit",
        "<a href=workunit.php?id=$result->workunitid>" . wu_name_by_id($result->workunitid) . "</a>" );

    row4("state", res_state_string($result->state), "Host ID",
        "<a href=host.php?id=$result->hostid>" . host_name_by_id($result->hostid) . "</a>");

    row4("Exit Status", $result->exit_status, "CPU time", $result->cpu_time);
    if ($show_xml_docs) {
        row("XML doc in", "<pre>".htmlspecialchars($result->xml_doc_in)."</pre>");
        row("XML doc out", "<pre>".htmlspecialchars($result->xml_doc_out)."</pre>");
    }
    if ($show_stderr) {
        row("stderr out", "<pre>".htmlspecialchars($result->stderr_out)."</pre>");
    }
    row4("batch", $result->batch, "validated", $result->validated);
    end_table();
    echo "<p>";
}

function show_user($user) {
    start_table();
    row("Created", time_str($user->create_time));
    row("Total Credit", $user->total_credit);
    row("Recent Averaged Credit:", $user->expavg_credit);
    row("Name", $user->name);
    row("Email Address", $user->email_addr);
    row("Country", $user->country);
    row("Postal Code", $user->postal_code);
    row("Team", ($user->teamid == 0 ? "None" : "<a href=team.php?id=$user->teamid>" . team_name_by_id($user->teamid) . "</a>" ));
    row("", "<a href=host.php?user_id=$user->id>User Hosts</a>");
    end_table();
}

function team_type_string($s) {
    switch ($s) {
        case 1: return "Small Company";
        case 2: return "Medium Company";
        case 3: return "Large Company";
        case 4: return "Club";
        case 5: return "Primary School";
        case 6: return "Secondary School";
        case 7: return "Junior College";
        case 8: return "University or Department";
        case 9: return "Government Agency";
        default: return "Unknown";
    }
}

function show_team($team) {
    start_table();
    row("ID", $team->id);
    row("Team Founder", "<a href=user.php?id=$team->userid>" . user_name_by_id($team->userid) . "</a>");
    row("Name", $team->name);
    row("Name (HTML Formatted)", "<pre>" . htmlspecialchars($team->name_html) . "</pre>" );
    row("Url", "<a href=http://$team->url>" . $team->url . "</a>");
    row("Type", team_type_string($team->type));
    row("Description", $team->description);
    row("", "<a href=user.php?team_id=$team->id>List All Members</a>");
    end_table();
}

function team_name_by_id($team_id) {
    $result = mysql_query("select * from team where id = $team_id");
    $team = mysql_fetch_object($result);
    return $team->name;
}

function user_name_by_id($user_id) {
    $result = mysql_query("select * from user where id = $user_id");
    $user = mysql_fetch_object($result);
    return $user->name;
}

function app_name_by_id($app_id) {
    $result = mysql_query("select * from app where id = $app_id");
    $app = mysql_fetch_object($result);
    return $app->name;
}

function wu_name_by_id($wu_id) {
    $result = mysql_query("select * from workunit where id = $wu_id");
    $wu = mysql_fetch_object($result);
    return $wu->name;
}

function platform_name_by_id($plat_id) {
    $result = mysql_query("select * from platform where id = $plat_id");
    $plat = mysql_fetch_object($result);
    return $plat->name;
}

function host_name_by_id($host_id) {
    $result = mysql_query("select * from host where id = $host_id");
    $host = mysql_fetch_object($result);
    if (!strlen($host->domain_name) && !strlen($host->last_ip_addr))
        return "(blank)";
    else
        return $host->domain_name . " (" . $host->last_ip_addr . ")";
}

?>
