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

function show_host($host, $private, $ipprivate) {
    start_table();
    row1("Computer information");
    if ($private) {
        if ($ipprivate) {
            row2("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
        } else { 
            row2("IP address", "<a href=show_host_detail.php?hostid=$host->id&ipprivate=1>Show IP address</a>");
        }
        row2("Domain name", $host->domain_name);
        $x = $host->timezone/3600;
        if ($x > 0) $x="+$x";
        row2("Time zone", "UTC $x hours");
    }
    if ($private) {
        row2("Name", $host->domain_name);
    }
    if (!$private) {
        $user = lookup_user_id($host->userid);
        if ($user && $user->show_hosts) {
            row2("Owner", user_links($user));
        } else {
            row2("Owner", "Anonymous");
        }
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
if (false) {
    $nresults = host_nresults($host);
    if ($nresults) {
        $results = "<a href=results.php?hostid=$host->id>$nresults</a>";
    } else {
        $results = "0";
    }
    row2("Results", $results);
}

    if ($private) {
        row2("Number of times client has contacted server", $host->rpc_seqno);
        row2("Last time contacted server", time_str($host->rpc_time));
        row2("% of time client is on", 100*$host->on_frac." %");
        row2("% of time host is connected", 100*$host->connected_frac." %");
        row2("% of time user is active", 100*$host->active_frac." %");
        row2("Location", location_form($host));
        if ($nresults == 0) {
            $x = " | <a href=host_delete.php?hostid=$host->id>Delete this host</a> ";
        } else {
            $x = "";
        }
        row2("Click to", "<a href=host_edit_form.php?hostid=$host->id>merge this computer</a> $x");
        //row2("Click to", "<a href=host_update_credit.php?hostid=$host->id>recompute credit</a>");
    }
    echo "</table>\n";

}

// The following is used to show a user's hosts
//
function user_host_table_start($private) {
    start_table();
    echo "<tr>";
    echo "<th>Computer ID<br><font size=-2>Click for more info</font></th>\n";
    if ($private) {
        echo "<th>Name</th>\n
        ";
    } else {
        echo "<th>Rank</th>";
    }
    echo "
        <th>Recent average credit</th>
        <th>Total credit</th>
        <th>CPU type</th>
        <th>Operating system</th>
        </tr>
    ";
}

// the following is used for list of top hosts
//
function top_host_table_start($sort_by) {
    start_table();
    echo "<tr>";
    echo "<th>Computer ID<br><font size=-2>Click for more info</font></th>\n";
    echo "<th>Rank</th>";
    echo "<th>Owner</th>\n";
    if ($sort_by == 'total_credit') {
        echo "
            <th><a href=top_hosts.php?sort_by=expavg_credit>Recent average credit</a></th>
            <th>Total credit</th>
        ";
    } else {
        echo "
            <th>Recent average credit</th>
            <th><a href=top_hosts.php?sort_by=total_credit>Total credit</a></th>
        ";
    }
    echo "
        <th>CPU type</th>
        <th>Operating system</th>
        </tr>
    ";
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
function show_host_row($host, $i, $private, $show_owner) {
    $result = mysql_query("select * from user where id = $host->userid");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);

    echo "<tr><td>", host_link($host->id), "</td>\n";
    if ($private) {
        echo "<td> $host->domain_name";
    } else {
        echo "<td>$i</td>\n
        ";
        if ($show_owner) {
            if ($user->show_hosts) {
                echo "<td>", user_links($user), "</td>\n";
            } else {
                echo "<td>Anonymous</td>\n";
            }
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

    //$nresults = host_nresults($host);
    //echo "<td><a href=results.php?hostid=$host->id>$nresults</a></td>";
    echo "</tr>\n";
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

function host_update_credit($hostid) {
    $t = 0;
    $avg = 0;
    $now = time();
    $avg_time = 0;
    $half_life = 86400*7;
    $r = mysql_query("select * from result where hostid=$hostid order by received_time");
    while ($result = mysql_fetch_object($r)) {
        if ($result->granted_credit <= 0) continue;
        $t += $result->granted_credit;

        // the following taken from lib/util.C
        //
        if ($avg_time) {
            $diff = $result->received_time - $avg_time;
            if ($diff <=0) $diff = 3600;
            $diff_days = $diff/86400;
            $weight = exp(-$diff*M_LN2/$half_life);
            $avg *= $weight;
            $avg += (1-$weight)*($result->granted_credit/$diff_days);
        } else {
            $dd = ($result->received_time - $result->sent_time)/86400;
            $avg = $result->granted_credit/$dd;
        }
        $avg_time = $result->received_time;
        echo "<br>$avg\n";
    }
    mysql_free_result($r);

    $diff = $now - $avg_time;
    $weight = exp(-$diff*M_LN2/$half_life);
    $avg *= $weight;
    mysql_query("update host set total_credit=$t, expavg_credit=$avg where id=$hostid");
}

?>
