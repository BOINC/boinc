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
    row1("Computer information");
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
    row2("Total Credit", format_credit($host->total_credit));
    row2("Recent average credit", format_credit($host->expavg_credit));
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
    $last_result = host_last_result($host);
    $nresults = host_nresults($host);
    $x = "<a href=results_host.php?hostid=$host->id>$nresults</a>";
    if ($last_result) {
        $y = time_str($last_result->received_time);
        $x = $x."<br><font size=2>Last received: $y</font>\n";
    }
    row2("Results", $x);

    if ($private) {
        row2("Number of times client has contacted server", $host->rpc_seqno);
        row2("Last time contacted server", time_str($host->rpc_time));
        row2("% of time client is on", 100*$host->on_frac." %");
        row2("% of time host is connected", 100*$host->connected_frac." %");
        row2("% of time user is active", 100*$host->active_frac." %");
        row2("Location", location_form($host));
        row2("Edit", "<a href=host_edit_form.php?hostid=$host->id>Merge this host</a>");
    }
    echo "</table>\n";

}

function host_table_start($title, $private) {
    start_table();
    row1($title, 7);
    echo "<tr>";
    if ($private) {
        echo "<th>Name
            <br><font size=-2>Click for more info</font>
            </th>\n
        ";
    } else {
        echo "<th>
            Rank<br><font size=-2>Click for more info</font>
            </th>
        ";
        echo "<th>Owner</th>\n";
    }
    echo "
        <th>Recent average credit</th>
        <th>Total credit</th>
        <th>CPU type</th>
        <th>Operating system</th>
        <th>Results<br><font size=-2>Click for list</font></th>
        </tr>
    ";
}

function host_last_result($host) {
    $result = mysql_query("select * from result where hostid = $host->id order by received_time desc limit 1");
    $last_result = mysql_fetch_object($result);
    mysql_free_result($result);
    return $last_result;
}

function host_nresults($host) {
    $result = mysql_query("select count(*) as nresults from result where hostid=$host->id");
    $foobar = mysql_fetch_object($result);
    return $foobar->nresults;
}

// If private is true, we're showing the host to its owner,
// so it's OK to show the domain name etc.
// If private is false, show the owner's name only if they've given permission
//
function show_host_row($host, $i, $private) {
    $result = mysql_query("select * from user where id = $host->userid");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);

    echo "<tr>";
    if ($private) {
        echo "<td>
            <a href=show_host_detail.php?hostid=$host->id&private=1>
            $host->domain_name</a></td>
        ";
    } else {
        echo "<td><a href=show_host_detail.php?hostid=$host->id>$i</a></td>\n";
	if ($user->show_hosts) {
	    echo "<td><a href=show_user.php?userid=$user->id>$user->name</a></td>\n";
        } else {
            echo "<td>Anonymous</td>\n";
        }
    }
    printf("
        <td>%s</td>
        <td>%s</td>
        <td>%s %s</td>
        <td>%s %s</td>",
           format_credit($host->expavg_credit), format_credit($host->total_credit),
           $host->p_vendor, $host->p_model,
           $host->os_name, $host->os_version
    );

    $last_result = host_last_result($host);
    $nresults = host_nresults($host);
    echo "<td><a href=results_host.php?hostid=$host->id>$nresults</a>";
    if ($last_result) {
        $x = time_str($last_result->received_time);
        echo "<br><font size=2>Last received: $x</font>\n";
    }
    echo "</td></tr>\n";
}

// return true iff it's possible that the hosts are actually
// the same machine
//
function hosts_compatible($host1, $host2) {
    if ($host2->p_vendor != $host1->p_vendor) return false;
    if ($host2->p_model != $host1->p_model) return false;
    if ($host2->os_name != $host1->os_name) return false;
    return true;
}

?>
