<?php

function db_init() {
    $retval = mysql_connect();
    if (!$retval) {
        echo "Unable to connect to database - please try again later";
        exit();
    }
    $db_name = parse_config("<db_name>");
    if(!mysql_select_db($db_name)) {
	    echo "Unable to select database - please try again later";
	    exit();
    }

    return 0;
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

function result_server_state_select() {
    echo "
        <select name=result_server_state>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".result_server_state_string($i) . "\n";
    }
    echo "</select>\n";
}

function result_outcome_select() {
    echo "
        <select name=result_outcome>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".result_outcome_string($i) . "\n";
    }
    echo "</select>\n";
}

function result_client_state_select() {
    echo "
        <select name=result_client_state>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".result_client_state_string($i) . "\n";
    }
    echo "</select>\n";
}

function result_sort_select() {
    echo "
        <select name=sort_by>
        <option value=''>None
        <option value=create_time>Create time
        <option value=sent_time>Sent time
        <option value=received_time>Received time
        </select>
    ";
}

function table_title($table) {
    switch($table) {
    case "platform": return "Platforms";
    case "app": return "Applications";
    case "app_version": return "Application Versions";
    case "host": return "Hosts";
    case "workunit": return "Workunits";
    case "result": return "Results";
    case "team": return "Teams";
    case "user": return "Users";
    default: return "????";
    }
}

function show_platform($platform) {
    start_table();
    row("ID", $platform->id);
    row("Created", time_str($platform->create_time));
    row("Name", $platform->name);
    row("User friendly name", $platform->user_friendly_name);
    row("","<a href=db_action.php?table=app_version&plat_id=$platform->id>App versions for this platform</a>");
    end_table();
}

function show_app($app) {
    start_table();
    row("ID", $app->id);
    row("Created", time_str($app->create_time));
    row("Name", $app->name);
    row("","<a href=db_action.php?table=app_version&app_id=$app->id>App Versions for this application</a>");
    row("","<a href=db_action.php?table=workunit&app_id=$app->id>Workunits for this application</a>");
    end_table();
}

function show_app_version($app_version, $hide_xml_docs) {
    start_table();
    row("ID", $app_version->id);
    row("Created", time_str($app_version->create_time));
    row("Application", "<a href=db_action.php?table=app&id=$app_version->appid>" . app_name_by_id($app_version->appid) . "</a>");
    row("Version num", $app_version->version_num);
    row("Platform", "<a href=db_action.php?table=platform&id=$app_version->platformid>" . platform_name_by_id($app_version->platformid) . "</a>" );
    if (!$hide_xml_docs) {
        row("XML doc", "<pre>".htmlspecialchars($app_version->xml_doc)."</pre>");
    }
    row("Min_core_version", $app_version->min_core_version);
    row("Max_core_version", $app_version->max_core_version);
    row("Message", $app_version->message);
    row("Deprecated", $app_version->deprecated);
    end_table();
}

function host_short_header() {
    echo "
        <tr>
        <th>host ID</th>
        <th>IP address</th>
        <th>name</th>
        <th>CPU</th>
        <th>OS</th>
        </tr>
    ";
}

function show_host_short($host) {
    echo "
        <tr>
        <td><a href=db_action.php?table=host&id=$host->id>$host->id</a></td>
        <td>$host->last_ip_addr</td>
        <td>$host->domain_name</td>
        <td>$host->p_vendor $host->p_model</td>
        <td>$host->os_name $host->os_version</td>
        </tr>
    ";
}

function show_host($host) {
    start_table();

    row("ID", $host->id);
    row("Created", time_str($host->create_time));
    row("Venue", $host->venue);
    row("Total credit", $host->total_credit);
    row("Average credit", $host->expavg_credit);
    row("Average update time", time_str($host->expavg_time));
    row("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
    row("Domain name", $host->domain_name);
    $x = $host->timezone/3600;
    row("Time zone", "UTC - $x hours");
    row("Number of CPUs", $host->p_ncpus);
    row("CPU", "$host->p_vendor $host->p_model");
    row("FP ops/sec", $host->p_fpops);
    row("Int ops/sec", $host->p_iops);
    row("memory bandwidth", $host->p_membw);
    row("Operating System", "$host->os_name $host->os_version");
    $x = $host->m_nbytes/(1024*1024);
    $y = round($x, 2);
    row("Memory", "$y MB");
    $x = $host->m_cache/1024;
    $y = round($x, 2);
    row("Cache", "$y KB");
    $x = $host->m_swap/(1024*1024);
    $y = round($x, 2);
    row("Swap Space", "$y MB");
    $x = $host->d_total/(1024*1024*1024);
    $y = round($x, 2);
    row("Total Disk Space", "$y GB");
    $x = $host->d_free/(1024*1024*1024);
    $y = round($x, 2);
    row("Free Disk Space", "$y GB");
    row("Avg network bandwidth (upstream)", "$host->n_bwup bytes/sec");
    row("Avg network bandwidth (downstream)", "$host->n_bwdown bytes/sec");
    row("Number of RPCs", $host->rpc_seqno);
    row("Last RPC", time_str($host->rpc_time));
    row("% of time client on", 100*$host->on_frac." %");
    row("% of time host connected", 100*$host->connected_frac." %");
    row("% of time user active", 100*$host->active_frac." %");
    row("Results", "<a href=db_action.php?table=result&detail=low&hostid=$host->id&sort_by=sent_time>click here</a>");
    end_table();

}

function wu_error_mask_str($s) {
    $x = "";
    if ($s & 1) $x = $x."Couldn't send result; ";
    if ($s & 2) $x = $x."Too many errors (may have bug); ";
    if ($s & 4) $x = $x."Too many results (may be nondeterministic)";
    $x = $x."<br>";
    return $x;
}

function assimilate_state_str($s) {
    switch($s) {
    case 0: return "Initial";
    case 1: return "Ready to assimilate";
    case 2: return "Assimilated";
    }
}

function file_delete_state_str($s) {
    switch($s) {
    case 0: return "Initial";
    case 1: return "Ready to delete";
    case 2: return "Deleted";
    }
}

function show_workunit($wu, $hide_xml_doc) {
    start_table();
    row("Created", time_str($wu->create_time));
    row("Name", $wu->name);
    if (!$hide_xml_doc) {
        row("XML doc", "<pre>".htmlspecialchars($wu->xml_doc)."</pre>");
    }
    row("Application", "<a href=db_action.php?table=app&id=$wu->appid>" . app_name_by_id($wu->appid) . "</a>");
    row("Batch", $wu->batch);
    row("FP Operations", $wu->rsc_fpops);
    row("Integer Operations", $wu->rsc_iops);
    row("Memory Requirement", $wu->rsc_memory);
    row("Disk Requirement", $wu->rsc_disk);
    row("Need validate?", $wu->need_validate?"yes":"no");
    row("Canonical resultid",
            "<a href=db_action.php?table=result&id=$wu->canonical_resultid>".$wu->canonical_resultid."</a>");
    row("Canonical credit", $wu->canonical_credit);
    row("Timeout check time", time_str($wu->timeout_check_time));
    row("Delay bound", $wu->delay_bound);
    row("Error mask", wu_error_mask_str($wu->error_mask));
    row("File delete state", file_delete_state_str($wu->file_delete_state));
    row("Assimilation state", assimilate_state_str($wu->assimilate_state));
    row("","<a href=db_action.php?table=result&wu_id=$wu->id>Show associated results</a>");
    row("","<a href='show_log.php?s=$wu->name'>Grep logs</a>");
    end_table();
    echo "<p>";
}

function result_server_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In Progress";
    case 5: return "Over";
    }
    return "Unknown";
}

function result_outcome_string($s) {
    switch($s) {
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3: return "Client error";
    case 4: return "No reply";
    case 5: return "Didn't need";
    }
    return "Unknown";
}

function result_client_state_string($s) {
    switch($s) {
    case 0: return "New";
    case 1: return "Downloading";
    case 2: return "Downloaded";
    case 3: return "Computing";
    case 4: return "Uploading";
    case 5: return "Done";
    }
    return "Unknown";
}

function validate_state_str($s) {
    switch($s) {
    case 0: return "Initial";
    case 1: return "Valid";
    case 2: return "Invalid";
    }
    return "Unknown";
}

function show_result($result, $hide_xml_docs, $hide_stderr, $hide_times) {
    start_table();
    if (!$hide_times) {
        row("Created", time_str($result->create_time));
        row("Sent", time_str($result->sent_time));
        row("Received", time_str($result->received_time));
    }
    row("Name", $result->name);
    row("Workunit", "<a href=db_action.php?table=workunit&id=$result->workunitid>" . wu_name_by_id($result->workunitid) . "</a>" );
    row("Server state", result_server_state_string($result->server_state));
    row("Outcome", result_outcome_string($result->outcome));
    row("Client state", result_client_state_string($result->client_state));
    row("Host ID", "<a href=db_action.php?table=host&id=$result->hostid>" . host_name_by_id($result->hostid) . "</a>");
    row("Report deadline", time_str($result->report_deadline));
    row("CPU time", $result->cpu_time);
    if (!$hide_xml_docs) {
        row("XML doc in", "<pre>".htmlspecialchars($result->xml_doc_in)."</pre>");
        row("XML doc out", "<pre>".htmlspecialchars($result->xml_doc_out)."</pre>");
    }
    if (!$hide_stderr) {
        row("stderr out", "<pre>".htmlspecialchars($result->stderr_out)."</pre>");
    }
    row("Batch", $result->batch);
    row("File delete state", file_delete_state_str($result->file_delete_state));
    row("Validate state", validate_state_str($result->validate_state));
    row("claimed credit", $result->claimed_credit);
    row("Granted credit", $result->granted_credit);
    row("","<a href='show_log.php?s=$result->name'>Grep logs</a>");
    end_table();
    echo "<p>";
}

function result_short_header() {
    echo "
        <tr>
        <th>result ID</th>
        <th>WU ID</th>
        <th>server state</th>
        <th>outcome</th>
        <th>client state</th>
        <th>host</th>
        </tr>
    ";
}

function show_result_short($result) {
    $ss = result_server_state_string($result->server_state);
    $cs = result_client_state_string($result->client_state);
    $oc = result_outcome_string($result->outcome);
    echo "
        <tr>
        <td><a href=db_action.php?table=result&id=$result->id>$result->id</a></td>
        <td><a href=db_action.php?table=workunit&id=$result->workunitid>$result->workunitid</a></td>
        <td>$ss</td>
        <td>$oc</td>
        <td>$cs</td>
    ";
    if ($result->hostid) {
        echo "
            <td><a href=db_action.php?table=host&id=$result->hostid>$result->hostid</a></td>
        ";
    } else {
        echo "<td>---</td>\n";
    }
    echo "
        </tr>
    ";
}

function show_user($user) {
    start_table();
    row("ID", $user->id);
    row("Created", time_str($user->create_time));
    row("Name", $user->name);
    row("Authenticator", $user->authenticator);
    row("Email address", $user->email_addr);
    row("Country", $user->country);
    row("Postal code", $user->postal_code);
    row("Total credit", $user->total_credit);
    row("Average credit", $user->expavg_credit);
    row("Last average time", time_str($user->expavg_time));
    row("Default venue", $user->venue);
    row("Hosts", "<a href=db_action.php?table=host&userid=$user->id&detail=low>click</a>");
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
    row("Team Founder", "<a href=db_action.php?table=user&id=$team->userid>" . user_name_by_id($team->userid) . "</a>");
    row("Name", $team->name);
    row("Name (HTML Formatted)", "<pre>" . htmlspecialchars($team->name_html) . "</pre>" );
    row("Url", "<a href=http://$team->url>" . $team->url . "</a>");
    row("Type", team_type_string($team->type));
    row("Description", $team->description);
    row("", "<a href=db_action.php?table=user&team_id=$team->id>List All Members</a>");
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

function host_name_by_id($hostid) {
    $result = mysql_query("select * from host where id = $hostid");
    $host = mysql_fetch_object($result);
    if (!strlen($host->domain_name) && !strlen($host->last_ip_addr))
        return "(blank)";
    else
        return $host->domain_name . " (" . $host->last_ip_addr . ")";
}

?>
