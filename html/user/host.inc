<?php

function location_form($host) {
    if ($host->venue == "home") $h = "selected";
    if ($host->venue == "work") $w = "selected";
    if ($host->venue == "school") $s = "selected";
    $x = "<form action=host_venue_action.php>
        <input type=hidden name=hostid value=$host->id>
        <select name=venue>
        <option value=home $h>Home
        <option value=work $w>Work
        <option value=school $s>School
        </select>
        <input type=submit value=Update>
        </form>
    ";
    return $x;
}

function show_host($host, $private) {
    start_table();
    row1("Host Information");
    if ($private) {
        row2("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
        row2("Domain name", $host->domain_name);
        $x = $host->timezone/3600;
        row2("Time zone", "UTC - $x hours");
    }
    if ($private) {
        row2("Name", $host->domain_name);
    }
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

    if ($private) {
        $x = $host->m_swap/(1024*1024);
        $y = round($x, 2);
        row2("Swap space", "$y MB");
        $x = $host->d_total/(1024*1024*1024);
        $y = round($x, 2);
        row2("Total disk space", "$y GB");
        $x = $host->d_free/(1024*1024*1024);
        $y = round($x, 2);
        row2("Free Disk Space", "$y GB");
    }
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
        row2("Average upload rate", "$y KB/sec");
    } else {
        row2("Average upload rate", "Unknown");
    }
    $x = $host->n_bwdown/(1024);
    $y = round($x, 2);
    if ($y > 0) {
        row2("Average download rate", "$y KB/sec");
    } else {
        row2("Average download rate", "Unknown");
    }
    if ($private) {
        row2("Number of times client has contacted server", $host->rpc_seqno);
        row2("Last time contacted server", time_str($host->rpc_time));
        row2("% of time client is on", 100*$host->on_frac." %");
        row2("% of time host is connected", 100*$host->connected_frac." %");
        row2("% of time user is active", 100*$host->active_frac." %");
        row2("Location", location_form($host));
    }
    echo "</table>\n";

}

function host_table_start($title, $private) {
    start_table();
    row1($title, 6);
    echo "<tr>
        <th>Rank<br><font size=-2>Click for more info</font></th>
    ";
    if ($private) {
        echo "<th>Name</th>\n";
    } else {
        echo "<th>Owner</th>\n";
    }
    echo "
        <th>Total credit</th>
        <th>Recent average credit</th>
        <th>CPU type</th>
        <th>Operating system</th>
        </tr>
    ";
}

function show_host_row($host, $i, $private) {
    $result = mysql_query("select * from user where id = $host->userid");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
    echo "<tr>
        <td><a href=show_host_public.php?hostid=$host->id>$i</a></td>
    ";
    if ($private) {
        echo"<td>$host->domain_name</td>";
    } else {
        echo"<td><a href=show_user.php?userid=$user->id>$user->name</a></td>";
    }
    echo"
        <td>$host->total_credit</td>
        <td>$host->expavg_credit</td>
        <td>$host->p_vendor $host->p_model</td>
        <td>$host->os_name $host->os_version</td>
        </tr>
    ";
}

?>
