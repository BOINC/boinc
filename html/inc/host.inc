<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/credit.inc");

function link_to_results($host) {
    if (!$host) return "No host";
    $config = get_config();
    if (!parse_bool($config, "show_results")) return "Unavailable";
    $nresults = host_nresults($host);
    if (!$nresults) return "0";
    return "<a href=results.php?hostid=$host->id>$nresults</a>";
}

function sched_log_name($x) {
    if ($x == 0) return "NO_SUCH_LOG";
    return gmdate('Y-m-d_H/Y-m-d_H:i', $x) . ".txt";
}

function sched_log_link($x) {
    return "<a href=\"../sched_logs/" . sched_log_name($x) . "\">" . time_str($x) . "</a>";
}

function location_form($host) {
    $none = "selected";
    $h=$w=$s="";
    if ($host->venue == "home") $h = "selected";
    if ($host->venue == "work") $w = "selected";
    if ($host->venue == "school") $s = "selected";
    $x = "<form action=host_venue_action.php>
        <input type=hidden name=hostid value=$host->id>
        <select name=venue>
        <option value=\"\" $none>---
        <option value=home $h>Home
        <option value=work $w>Work
        <option value=school $s>School
        </select>
        <input type=submit value=Update>
        </form>
    ";
    return $x;
}

function show_host($host, $private, $ipprivate, $user) {
    start_table();
    row1("Computer information");
    if ($private) {
        if ($ipprivate) {
            row2("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
            if (strcmp($host->last_ip_addr, $host->external_ip_addr)) row2("External IP address", $host->external_ip_addr);
        } else { 
            row2("IP address", "<a href=show_host_detail.php?hostid=$host->id&ipprivate=1>Show IP address</a>");
        }
        row2("Domain name", $host->domain_name);
        $x = $host->timezone/3600;
        if ($x >= 0) $x="+$x";
        row2("Local Standard Time", "UTC $x hours");
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
    row2("CPU type", "$host->p_vendor <br> $host->p_model");
    row2("Number of CPUs", $host->p_ncpus);
    row2("Operating System", "$host->os_name <br> $host->os_version");
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
    $x = $host->avg_turnaround/86400;
    row2("Average turnaround time", round($x, 2)." days");
    $mrd= $host->max_results_day;
    row2("Maximum daily WU quota per CPU", $mrd."/day");
    $config = get_config();
    if (parse_bool($config, "show_results")) {
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
        row2("Last time contacted server", sched_log_link($host->rpc_time));
        row2("% of time BOINC client is running", 100*$host->on_frac." %");
        if ($host->connect_frac > 0) {
            row2("While BOINC running, % of time host has an Internet connection", 100*$host->connected_frac." %");
        }
        row2("While BOINC running, % of time work is allowed", 100*$host->active_frac." %");
        if ($host->cpu_efficiency) {
            row2("Average CPU efficiency", $host->cpu_efficiency);
        }
        if ($host->duration_correction_factor) {
            row2("Result duration correction factor", $host->duration_correction_factor);
        }
        row2("Location", location_form($host));
        if ($nresults == 0) {
            $x = " | <a href=host_delete.php?hostid=$host->id".url_tokens($user->authenticator).">Delete this host</a> ";
        } else {
            $x = "";
        }
        row2("Click to", "<a href=host_edit_form.php?hostid=$host->id>merge this computer</a> $x");
        //row2("Click to", "<a href=host_update_credit.php?hostid=$host->id>recompute credit</a>");
    }
    echo "</table>\n";

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
    if ($show_owner) {
        // This is used in the TOP COMPUTERS display
        //
        printf("
            <td>%s</td>
            <td>%s</td>
            <td>%s <br> %s</td>
            <td>%s <br> %s</td>",
               format_credit($host->expavg_credit), format_credit($host->total_credit),
               $host->p_vendor, $host->p_model,
               $host->os_name, $host->os_version
        );
    } else {
        // This is used in constructing list of COMPUTERS for a given user
        //
        $config = get_config();
        if (parse_bool($config, "show_results")) {
            $ltr="<td>".link_to_results($host)."</td>";
        } else {
            $ltr="";
        }
        printf("
            <td>%s</td>
            <td>%s</td>
            <td>%s <br> %s</td>
            <td>%s <br> %s</td>
            %s
            <td>%s</td>",
               format_credit($host->expavg_credit), format_credit($host->total_credit),
               $host->p_vendor, $host->p_model,
               $host->os_name, $host->os_version, $ltr, sched_log_link($host->rpc_time)
        );
    }

    //$nresults = host_nresults($host);
    //echo "<td><a href=results.php?hostid=$host->id>$nresults</a></td>";
    echo "</tr>\n";
}

// return true iff it's possible that the hosts are actually
// the same machine
//
function ghz($x) {
    $y = explode(" ", $x);
    foreach ($y as $z) {
        if (strstr($z, "GHz")) return $z;
        if (strstr($z, "MHz")) return $z;
    }
}

function cpus_compatible($host1, $host2) {
    // we screwed around with Intel processor names,
    // so count them as compatible if both contain "Intel" and "Pentium",
    // and don't have conflicting clock rate info
    
    // Get rid of cpu capabilities before check
    $pos = strpos($host1->p_model, '[');
    $host1pm = $host1->p_model;
    if($pos) {
        $host1pm = trim(substr($host1->p_model, 0, $pos));
    }
    $pos = strpos($host2->p_model, '[');
    $host2pm = $host2->p_model;
    if($pos) {
        $host2pm = trim(substr($host2->p_model, 0, $pos));
    }
    $p1 = "$host1->p_vendor $host1pm";
    $p1 = "$host2->p_vendor $host2pm";
    
    if (strstr($p1, "Pentium") && strstr($p1, "Intel")
        && strstr($p2, "Pentium") && strstr($p2, "Intel")
    ) {
        $g1 = ghz($p1);
        $g2 = ghz($p2);
        if ($g1 && $g2) {
            if ($g1 != $g2) return false;
        } else {
            return false;
        }
    } else if (strstr($p1, "AuthenticAMD") && strstr($p2, "AuthenticAMD")) {
        return true;
    } else {
        if ($host2->p_vendor != $host1->p_vendor) return false;

        // they're compatible if models are the same,
        // or contents of [family/model/stepping] are the same
        if ($host1pm != $host2pm) return false;
    }
    return true;
}

// does one host strictly precede the other?
//
function times_disjoint($host1, $host2) {
    if ($host1->rpc_time < $host2->create_time) return true;
    if ($host2->rpc_time < $host1->create_time) return true;
    return false;
}

function os_compatible($host1, $host2) {
    if (strstr($host1->os_name, "Windows") && strstr($host2->os_name, "Windows")) return true;
    if (strstr($host1->os_name, "Linux") && strstr($host2->os_name, "Linux")) return true;
    if (strstr($host1->os_name, "Darwin") && strstr($host2->os_name, "Darwin")) return true;
    if (strstr($host1->os_name, "SunOS") && strstr($host2->os_name, "SunOS")) return true;
    if ($host1->os_name == $host2->os_name) return true;
    return false;
}

// Return true if it's possible that the two host records
// correspond to the same host
// NOTE: the cheat-proofing comes from checking
// that their time intervals are disjoint.
// So the CPU/OS checks don't have to be very strict.
//
function hosts_compatible($host1, $host2) {
    // skip disjoint-time check if one host or other has no credit
    //
    if ($host1->total_credit && $host->total_credit) {
        if (!times_disjoint($host1, $host2)) {
            echo "<br>h2 $host2->create_time $host2->rpc_time";
            return false;
        }
    }
    if (!os_compatible($host1, $host2)) {
        echo "<br>$host1->id $host2->id: OS not compatible: ($host1->os_name, $host2->os_name)\n";
        return false;
    }
    if (!cpus_compatible($host1, $host2)) {
        echo "<br>$host1->id $host2->id: CPU not compatible ($host1->p_vendor $host1->p_model, $host2->p_vendor $host2->p_model)\n";
        return false;
    }
    return true;
}

// recompute host's average credit by scanning results.
// Could be expensive if lots of results!
//
function host_update_credit($hostid) {
    $total = 0;
    $avg = 0;
    $avg_time = 0;
    $r = mysql_query("select * from result where hostid=$hostid order by received_time");
    while ($result = mysql_fetch_object($r)) {
        if ($result->granted_credit <= 0) continue;
        $total += $result->granted_credit;

        update_average(
            $result->received_time,
            $result->sent_time,
            $result->granted_credit,
            $avg,
            $avg_time
        );

        echo "<br>$avg\n";
    }
    mysql_free_result($r);

    // do a final decay
    //
    $now = time();
    update_average(now, 0, 0, $avg, $avg_time);

    mysql_query("update host set total_credit=$total, expavg_credit=$avg, expavg_time=$now where id=$hostid");
}

// decay a host's average credit
//
function host_decay_credit($host) {
    $avg = $host->expavg_credit;
    $avg_time = $host->expavg_time;
    $now = time(0);
    update_average($now, 0, 0, $avg, $avg_time);
    mysql_query("update host set expavg_credit=$avg, expavg_time=$now where id=$host->id");

}
// if the host hasn't received new credit for ndays,
// decay its average and return true
//
function host_inactive_ndays($host, $ndays) {
    $diff = time() - $host->expavg_time;
    if ($diff > $ndays*86400) {
        host_decay_credit($host);
        return true;
    }
    return false;
}

// invariant: old_host.create_time < new_host.create_time
//
function merge_hosts($old_host, $new_host) {
    if ($old_host->id == $new_host->id) {
        return "same host";
    }
    if (!hosts_compatible($old_host, $new_host)) {
        return "Can't merge host $old_host->id into $new_host->id - they're incompatible";
    }

    echo "<br>Merging host $old_host->id into host $new_host->id\n";

	// decay the average credit of both hosts
	//
	$now = time();
	update_average($now, 0, 0, $old_host->expavg_credit, $old_host->expavg_time);
	update_average($now, 0, 0, $new_host->expavg_credit, $new_host->expavg_time);

    // update the database:
    // - add credit from old to new host
    // - change results to refer to new host
    // - put old host in "zombie" state
    // - update rpc_seqno if needed
    //
    $total_credit = $old_host->total_credit + $new_host->total_credit;
    $recent_credit = $old_host->expavg_credit + $new_host->expavg_credit;
    $result = mysql_query("update host set total_credit=$total_credit, expavg_credit=$recent_credit, expavg_time=$now where id=$new_host->id");
    if (!$result) {
        return "Couldn't update credit of new computer";
    }
    $result = mysql_query("update result set hostid=$new_host->id where hostid=$old_host->id");
    if (!$result) {
        return "Couldn't update results";
    }

    $result = mysql_query("update host set total_credit=0, expavg_credit=0, userid=0, rpc_seqno=$new_host->id where id=$old_host->id");
    if (!$result) {
        return "Couldn't retire old computer";
    }
    echo "<br>Retired old computer $old_host->id\n";
    return 0;
}

?>
