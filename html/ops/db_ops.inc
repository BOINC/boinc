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

function join_query_string($s1, $s2) {
    if ($s1) {
        if ($s2) {
            return "$s1&s2";
        } else {
            return $s1;
        }
    } else {
        return $s2;
    }
}

class SqlQueryString {
    var $table;
    var $query;
    var $urlquery;
    function SqlQueryString() {
        $this->table = $_GET['table'];
        $this->query = $_GET['query'];
        $this->urlquery = "";
    }
    function add($clause) {
        if (!$this->query) {
            $this->query .= "where $clause";
        } else {
            $this->query .= " and $clause";
        }
    }
    function addclause($clause) {
        if ($clause) {
            $this->add($clause);
        }
    }
    function addeq($name) {
        $value = $_GET[$name];
        if (strlen($value)) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addeqnz($name) {
        $value = $_GET[$name];
        if (strlen($value) && $value > 0) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addgt($name) {
        $value = $_GET[$name];
        if (strlen($value) && $value > 0) {
            $this->add("$name > '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addsort($name) {
        $value = $_GET[$name];
        if (strlen($value)) {
            $this->query .= " order by $value desc";
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }

    function count() {
        $count_query = "select count(*) as cnt from $this->table $this->query";
        $result = mysql_query($count_query);
        if (!$result) return 0;
        $res = mysql_fetch_object($result);
        mysql_free_result($result);
        return $res->cnt;
    }

    function get_select_query($entries_to_show, $start_at) {
        if ($entries_to_show) {
            if ($start_at) {
                return "select * from $this->table $this->query limit $start_at,$entries_to_show";
            } else {
                return "select * from $this->table $this->query limit $entries_to_show";
            }
        } else {
            return "select * from $this->table $this->query";
        }
    }

    function get_url_query($query_extra) {
        $q = join_query_string($this->query, $query_extra);
        $s = "db_action.php?table=$this->table$this->urlquery";
        if ($q) {
            $s = "$s&query=$q";
        }
        return $s;
    }
}

function build_sql_query() {
    $q = new SqlQueryString();

    $q->addeq('id');
    $q->addeq('platformid');
    $q->addeq('appid');
    $q->addeq('workunitid');
    $q->addeq('hostid');
    $q->addeq('userid');
    $q->addeq('teamid');
    if ($_GET['nsecs']) {
        $_GET['received_time'] = time() - $_GET['nsecs'];
    }
    $q->addgt('received_time');
    $q->addeqnz('server_state');
    $q->addeqnz('outcome');
    $q->addeqnz('client_state');
    $q->addsort('sort_by');

    return $q;
}

function link_results($n, $mq, $query) {
    if ($n == '0') { // intentional compare by string
        return "0";
    } else {
        return "<a href=db_action.php?table=result&query=$mq&$query&sort_by=received_time&detail=low>$n</a>";
    }
}

function show_result_summary() {
    $server_state = array();
    $outcome = array();
    $client_state = array();

    for ($ss=1; $ss<6; $ss++) {
        $server_state[$ss] = 0;
    }
    for ($ro=0; $ro<6; $ro++) {
        $outcome[$ro] = 0;
    }
    for ($cs=1; $cs<6; $cs++) {
        $client_state[$cs] = 0;
    }

    $_GET['table'] = 'result';
    $_GET['sort_by'] = ''; // ignore sort
    $q = build_sql_query();
    $main_query = $q->get_select_query(0,0);
    $urlquery = $q->urlquery;
    echo "<p>Query: <b>$main_query</b><p>\n";
    $result = mysql_query($main_query);
    $ntotal =0; // TODO: how to count $result?

    $nvalid = 0;    // for SUCCESS results
    $ninvalid = 0;
    $nfile_deleted = 0;

    while ($res = mysql_fetch_object($result)) {
        $server_state[$res->server_state] += 1;
        $ntotal  += 1;
        if ($res->server_state == 5) {
            $outcome[$res->outcome] += 1;
            if ($res->outcome == 3) {
                $client_state[$res->client_state] += 1;
            }
            if ($res->outcome == 1) {
                if ($res->validate_state == 1) {
                    $nvalid += 1;
                }
                if ($res->validate_state == 2) {
                    $ninvalid += 1;
                }
                if ($res->file_delete_state >= 1) {
                    $nfile_deleted +=1;
                }
            }
        }
    }
    mysql_free_result($result);

    echo "<table>";
    echo "<tr valign=top>";
    echo "<td><h2>" . link_results("$ntotal results", $urlquery, '') . "</h2></td>";
    echo "<td><h2>" . link_results("'Over' results", $urlquery, "server_state=5") . "</h2></td>";
    echo "<td><h2>" . link_results("'Success' results", $urlquery, "outcome=1") . "</h2></td>";
    echo "<td><h2>" . link_results("'Client error' results", $urlquery, "outcome=3") . "</h2></td>";
    echo "</tr>";
    echo "<tr valign=top>";
    echo "<td><table border=2 cellpadding=4\n";
    echo "<tr><th>Server state</th><th># results</th></tr>\n";
    for ($ss=1; $ss<6; $ss++) {
        row2(server_state_string($ss),
             link_results($server_state[$ss],  $urlquery,"server_state=$ss"));
    }
    echo "</table></td>";

    echo "<td><table border=2 cellpadding=4\n";
    echo "<tr><th>Outcome</th><th># results</th></tr>\n";
    for ($ro=0; $ro<6; $ro++) {
        c_row2($outcome[$ro]?outcome_color($ro):'', outcome_string($ro),
            link_results($outcome[$ro], $urlquery, "outcome=$ro")
        );
    }
    echo "</table></td>";

    echo "<td>";
    echo "<td><table border=2 cellpadding=4\n";
    echo "<tr><th>Validate state</th><th># results</th></tr>\n";
    row2("Valid", link_results($nvalid, $urlquery, "validate_state=1"));
    row2("Invalid", link_results($ninvalid, $urlquery, "validate_state=2"));
    echo "</table>";
    echo "<table border=2 cellpadding=4\n";
    echo "<tr><th>File Delete state</th><th># results</th></tr>\n";
    row2("Files deleted", link_results($ninvalid, $urlquery, "file_delete_state= 1 or file_delete_state=2"));
    echo "</table></td>";

    echo "<td><table border=2 cellpadding=4\n";
    echo "<tr><th>Client state</th><th># results</th></tr>\n";
    for ($cs=1; $cs<6; $cs++) {
        row2(client_state_string($cs),
            link_results($client_state[$cs], $urlquery, "client_state=$cs")
        );
    }
    echo "</td></table>";

}

function server_state_select() {
    echo "
        <select name=server_state>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".server_state_string($i) . "\n";
    }
    echo "</select>\n";
}

function outcome_select() {
    echo "
        <select name=outcome>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".outcome_string($i) . "\n";
    }
    echo "</select>\n";
}

function client_state_select() {
    echo "
        <select name=client_state>
        <option value=0 selected> Any
    ";
    for($i=1; $i<=6; $i++) {
        echo "<option value=$i> ".client_state_string($i) . "\n";
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
    row("","<a href=db_action.php?table=app_version&platformid=$platform->id>App versions for this platform</a>");
    end_table();
}

function show_app($app) {
    start_table();
    row("ID", $app->id);
    row("Created", time_str($app->create_time));
    row("Name", $app->name);
    row("","<a href=db_action.php?table=app_version&appid=$app->id>App Versions for this application</a>");
    row("","<a href=db_action.php?table=workunit&appid=$app->id>Workunits for this application</a>");
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
    row("User", "<a href=db_action.php?table=user&id=$host->userid>$host->userid</a>");
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
    row("Estimated FP Operations", $wu->rsc_fpops_est);
    row("Max FP Operations", $wu->rsc_fpops_bound);
    row("Max Memory Usage", $wu->rsc_memory_bound);
    row("Max Disk Usage", $wu->rsc_disk_bound);
    row("Need validate?", $wu->need_validate?"yes":"no");
    row("Canonical resultid",
            "<a href=db_action.php?table=result&id=$wu->canonical_resultid>".$wu->canonical_resultid."</a>");
    row("Canonical credit", $wu->canonical_credit);
    row("Timeout check time", time_str($wu->timeout_check_time));
    row("Delay bound", $wu->delay_bound);
    row("Error mask", wu_error_mask_str($wu->error_mask));
    row("File delete state", file_delete_state_str($wu->file_delete_state));
    row("Assimilation state", assimilate_state_str($wu->assimilate_state));
    // row("","<a href=db_action.php?table=result&workunitid=$wu->id&detail=low>Show associated results</a>");
    row("min quorum", $wu->min_quorum);
    row("max error results", $wu->max_error_results);
    row("max total results", $wu->max_total_results);
    row("max success results", $wu->max_success_results);
    row("","<a href='show_log.php?s=$wu->name'>Grep logs</a>");
    if (!$hide_xml_doc) {
        row("result template", "<pre>".htmlspecialchars($wu->result_template)."</pre>");
    }
    end_table();

    $_GET = array('workunitid' => $wu->id);
    show_result_summary();

    echo "<p>";
}

function server_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In Progress";
    case 5: return "Over";
    }
    return "Unknown";
}

function outcome_string($s) {
    switch($s) {
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3: return "Client error";
    case 4: return "No reply";
    case 5: return "Didn't need";
    }
    return "Unknown";
}

function client_state_string($s) {
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

function client_version_string($client_version_num) {
    if (!$client_version_num) {
        return '---';
    } else {
        return sprintf("%.2f", $client_version_num/100);
    }
}

function host_user_link($hostid)
{
    if (!$hostid) return '----';

    $h = "<a href=db_action.php?table=host&id=$hostid>$hostid</a>";
    $result = mysql_query("select userid from host where id='$hostid' limit 1");
    if (!$result) return $h;
    $host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$host->userid) return $h;
    $result = mysql_query("select * from user where id='$host->userid' limit 1");
    if (!$result) return $h;
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    return "$h<br><small>(<a href=db_action.php?table=user&id=$user->id>$user->name</a>)</small>";
}

function outcome_color($s) {
    switch($s) {
    case 0: return 'eeeeee'; // unknown (server state not over probably)
    case 1: return '33cc33'; // "Success"
    case 3: return 'ff3333'; // "Client error"
    }
    return '';
}

function credit_str($c) {
    if ($c) {
        return sprintf("%.3f", $c);
    } else {
        return '---';
    }
}

function show_result($result, $hide_xml_docs, $hide_stderr, $hide_times)
{
    $wu_name = wu_name_by_id($result->workunitid);

    echo "<h2>$result->name</h2>";
    echo "<p>WU: <a href=db_action.php?table=workunit&id=$result->workunitid>$wu_name</a>";

    start_table();



    if (!$hide_times) {
        row("Created", time_str($result->create_time));
        row("Sent", time_str($result->sent_time));
        row("Received", time_str($result->received_time));
    }
    row("Name", $result->name);
    row("Workunit", "<a href=db_action.php?table=workunit&id=$result->workunitid>" . wu_name_by_id($result->workunitid) . "</a>" );
    row("Server state", server_state_string($result->server_state));
    row("Outcome", outcome_string($result->outcome));
    row("Client state", client_state_string($result->client_state));
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
    row("Client version", client_version_string($result->client_version_num));
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
        <th>host (user)</th>
        <th>client ver</th>
        <th>received</th>
        <th>CPU hours</th>
        <th>claimed<br>credit</th>
        <th>granted<br>credit</th>
        </tr>
    ";
}

function show_result_short($result) {
    $ss = server_state_string($result->server_state);
    $oc = outcome_string($result->outcome);
    if ($result->outcome == 3) {
        $cs = client_state_string($result->client_state);
        $oc = "$oc ($cs)";
    }
    $received = time_str($result->received_time);
    $version = client_version_string($result->client_version_num);
    $outcome_color = outcome_color($result->outcome);
    $host_user = host_user_link($result->hostid);
    $cpu_hours = sprintf("%.1f",$result->cpu_time / 3600);
    $claimed_credit = credit_str($result->claimed_credit);
    $granted_credit = credit_str($result->granted_credit);

    echo "
        <tr>
        <td><a href=db_action.php?table=result&id=$result->id>$result->id</a></td>
        <td><a href=db_action.php?table=workunit&id=$result->workunitid>$result->workunitid</a></td>
        <td>$ss</td>
        <td bgcolor=$outcome_color>$oc</td>
        <td>$host_user</td>
        <td>$version</td>
        <td>$received</td>
        <td>$cpu_hours</td>
        <td>$claimed_credit</td>
        <td>$granted_credit</td>
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
    row("", "<a href=db_action.php?table=user&teamid=$team->id>List All Members</a>");
    end_table();
}

function team_name_by_id($teamid) {
    $result = mysql_query("select * from team where id = $teamid");
    $team = mysql_fetch_object($result);
    return $team->name;
}

function user_name_by_id($user_id) {
    $result = mysql_query("select * from user where id = $user_id");
    $user = mysql_fetch_object($result);
    return $user->name;
}

function app_name_by_id($appid) {
    $result = mysql_query("select * from app where id = $appid");
    $app = mysql_fetch_object($result);
    return $app->name;
}

function wu_name_by_id($workunitid) {
    $result = mysql_query("select * from workunit where id = $workunitid");
    $wu = mysql_fetch_object($result);
    return $wu->name;
}

function platform_name_by_id($platformid) {
    $result = mysql_query("select * from platform where id = $platformid");
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
