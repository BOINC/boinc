<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/credit.inc");
require_once("../inc/stats_sites.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");

function link_to_results($host) {
    if (!$host) return tra("No host");
    $config = get_config();
    if (!parse_bool($config, "show_results")) return tra("Unavailable");
    $nresults = host_nresults($host);
    if (!$nresults) return "0";
    return "<a href=results.php?hostid=$host->id>$nresults</a>";
}

function sched_log_name($x) {
    if ($x == 0) return "NO_SUCH_LOG";
    return gmdate('Y-m-d_H/Y-m-d_H:i', $x) . ".txt";
}

function sched_log_link($x) {
    if (file_exists("sched_logs")) {
        return "<a href=\"../sched_logs/" . sched_log_name($x) . "\">" . time_str($x) . "</a>";
    } else {
        return time_str($x);
    }
}

function location_form($host) {
    $none = "selected";
    $h=$w=$s=$m="";
    if ($host->venue == "home") $h = "selected";
    if ($host->venue == "work") $w = "selected";
    if ($host->venue == "school") $s = "selected";
    $x = "<form action=host_venue_action.php>
        <input type=hidden name=hostid value=$host->id>
        <select class=\"form-control\" name=venue>
        <option value=\"\" $none>---
        <option value=home $h>".tra("Home")."
        <option value=work $w>".tra("Work")."
        <option value=school $s>".tra("School")."
        </select>
        <p></p>
        <input class=\"btn btn-primary btn-sm\" type=submit value=\"".tra("Update location")."\">
        </form>
    ";
    return $x;
}

function cross_project_links($host) {
    global $host_sites;
    $x = "";
    foreach ($host_sites as $h) {
        $url = $h[0];
        $name = $h[1];
        $img = $h[2];
        $x .= "<a href=$url".$host->host_cpid."><img class=\"icon\" border=2 src=img/$img alt=\"$name\"></a> ";
    }
    return $x;
}

// Show full-page description of $host.
// If $user is non-null, it's both the owner of the host
// and the logged in user (so show some extra fields)
//
function show_host($host, $user, $ipprivate) {
    $config = get_config();
    start_table();
    row1(tra("Computer information"));
    $anonymous = false;
    if ($user) {
        if ($ipprivate) {
            row2(tra("IP address"), "$host->last_ip_addr<br>".tra("(same the last %1 times)", $host->nsame_ip_addr));
            if ($host->last_ip_addr != $host->external_ip_addr) {
                row2(tra("External IP address"), $host->external_ip_addr);
            }
        } else {
            row2(tra("IP address"), "<a href=show_host_detail.php?hostid=$host->id&ipprivate=1>".tra("Show IP address")."</a>");
        }
        row2(tra("Domain name"), $host->domain_name);
        if ($host->product_name) {
            row2(tra("Product name"), $host->product_name);
        }
        $x = $host->timezone/3600;
        if ($x >= 0) $x="+$x";
        row2(tra("Local Standard Time"), tra("UTC %1 hours", $x));
    } else {
        $owner = BoincUser::lookup_id($host->userid);
        if ($owner && $owner->show_hosts) {
            row2(tra("Owner"), user_links($owner, BADGE_HEIGHT_MEDIUM));
        } else {
            row2(tra("Owner"), tra("Anonymous"));
            $anonymous = true;
        }
    }
    row2(tra("Created"), time_str($host->create_time));
    if (!NO_STATS) {
        row2(tra("Total credit"), format_credit_large($host->total_credit));
        row2(tra("Average credit"), format_credit($host->expavg_credit));
        if (!$anonymous) {
            row2(tra("Cross project credit"), cross_project_links($host));
        }
    }
    row2(tra("CPU type"), "$host->p_vendor <br> $host->p_model");
    row2(tra("Number of cores"), $host->p_ncpus);
    $misc = json_decode($host->misc);
    row2(tra("Coprocessors"), gpu_desc($misc));
    row2(tra("Virtualization"), vbox_desc($misc));
    row2(tra("Docker"), docker_desc($misc));
    row2(tra("Operating System"), "$host->os_name <br> $host->os_version");
    row2(tra("BOINC version"), boinc_version($misc));
    $x = $host->m_nbytes/GIGA;
    $y = round($x, 2);
    row2(tra("Memory"), tra("%1 GB", $y));
    if ($host->m_cache > 0) {
        $x = $host->m_cache/KILO;
        $y = round($x, 2);
        row2(tra("Cache"), tra("%1 KB", $y));
    }

    if ($user) {
        $x = $host->m_swap/GIGA;
        $y = round($x, 2);
        row2(tra("Swap space"), tra("%1 GB", $y));
        $x = $host->d_total/GIGA;
        $y = round($x, 2);
        row2(tra("Total disk space"), tra("%1 GB", $y));
        $x = $host->d_free/GIGA;
        $y = round($x, 2);
        row2(tra("Free Disk Space"), tra("%1 GB", $y));
    }
    $x = $host->p_fpops/1e9;
    $y = round($x, 2);
    row2(tra("Measured floating point speed"), tra("%1 billion ops/sec", $y));
    $x = $host->p_iops/1e9;
    $y = round($x, 2);
    row2(tra("Measured integer speed"), tra("%1 billion ops/sec", $y));
    $x = $host->n_bwup/MEGA;
    $y = round($x, 2);
    if ($y > 0) {
        row2(tra("Average upload rate"), tra("%1 MB/sec", $y));
    } else {
        row2(tra("Average upload rate"), tra("Unknown"));
    }
    $x = $host->n_bwdown/MEGA;
    $y = round($x, 2);
    if ($y > 0) {
        row2(tra("Average download rate"), tra("%1 MB/sec", $y));
    } else {
        row2(tra("Average download rate"), tra("Unknown"));
    }

    // This code is used by Science United,
    // which knows about hosts but doesn't compute
    if (!NO_COMPUTING) {
        $x = $host->avg_turnaround;
        if ($x) {
            row2(tra("Average turnaround time"), time_diff($x));
        }
        row2(tra("Application details"),
            "<a href=host_app_versions.php?hostid=$host->id>".tra("Show")."</a>"
        );
        $show_results = parse_bool($config, "show_results");
        if ($show_results) {
            $nresults = host_nresults($host);
            if ($nresults) {
                $results = "<a href=results.php?hostid=$host->id>$nresults</a>";
            } else {
                $results = "0";
            }
            row2(tra("Tasks"), $results);
        }
        if (defined('BATCH_ACCEL') && BATCH_ACCEL) {
            row2('Low turnaround time?',
                $host->error_rate?'yes':'no'
            );
        }
    }

    if ($user) {
        row2(tra("Number of times client has contacted server"), $host->rpc_seqno);
        row2(tra("Last time contacted server"), sched_log_link($host->rpc_time));
        row2(tra("Fraction of time BOINC is running"), number_format(100*$host->on_frac, 2)."%");
        if ($host->connected_frac > 0) {
            row2(tra("While BOINC is running, fraction of time computer has an Internet connection"), number_format(100*$host->connected_frac, 2)."%");
        }
        row2(tra("While BOINC is running, fraction of time computing is allowed"), number_format(100*$host->active_frac, 2)."%");
        row2(tra("While is BOINC running, fraction of time GPU computing is allowed"), number_format(100*$host->gpu_active_frac, 2)."%");
        if ($host->cpu_efficiency) {
            row2(tra("Average CPU efficiency"), $host->cpu_efficiency);
        }
        if (!NO_COMPUTING) {
            if ($host->duration_correction_factor) {
                row2(tra("Task duration correction factor"), $host->duration_correction_factor);
            }
        }
        row2(tra("Location"), location_form($host));
        row2(tra("Merge duplicate records of this computer"), "<a class=\"btn btn-primary btn-sm\" href=host_edit_form.php?hostid=$host->id>".tra("Merge")."</a>");
        if ($show_results && $nresults == 0) {
            row2('',
                button_text(
                    sprintf(
                        'host_delete.php?hostid=%d%s',
                        $host->id,
                        url_tokens($user->authenticator)
                    ),
                    tra("Delete this computer")
                )
            );
        }
    } else {
        row2(tra("Number of times client has contacted server"), $host->rpc_seqno);
        row2(tra("Last contact"), date_str($host->rpc_time));
    }
    echo "</table>\n";

}

// the following is used for list of top hosts
//
function top_host_table_start($sort_by) {
    global $host_sites;
    shuffle($host_sites);
    start_table('table-striped');
    $x = array(
        tra("Info"),
        tra("Rank"),
        tra("Owner"),
    );
    if (!NO_STATS) {
        if ($sort_by == 'total_credit') {
            $x[] = "<a href=top_hosts.php?sort_by=expavg_credit>".tra("Avg. credit")."</a>";
            $x[] = tra("Total credit");
        } else {
            $x[] = tra("Recent average credit");
            $x[] = "<a href=top_hosts.php?sort_by=total_credit>".tra("Total credit")."</a>";
        }
    }
    $x[] = tra("BOINC version");
    $x[] = tra("CPU");
    $x[] = tra("GPU");
    $x[] = tra("Operating system");
    $s = 'style="text-align:right;"';
    $a = array("", "", "", $s, $s, "", "", "", "");
    row_heading_array($x, $a, "bg-default");
}

function host_nresults($host) {
    return BoincResult::count("hostid=$host->id");
}

function vbox_desc($misc) {
    if (empty($misc->vbox)) return '---';
    $v = $misc->vbox;
    return sprintf(
        'Virtualbox %s<br>HW acceleration: %s<br>Enabled: %s',
        $v->version,
        $v->hw_accel?'yes':'no',
        $v->hw_accel_enabled?'yes':'no'
    );
}

function docker_desc($misc) {
    if (empty($misc->docker)) return '---';
    $d = $misc->docker;
    $x = sprintf('%s version %s',
        $d->type==1?"Docker":"Podman",
        $d->version
    );
    if (!empty($d->wsl_distro)) {
        $x .= sprintf(
            '<br>WSL distro: %s ver %d',
            $d->wsl_distro, $d->boinc_buda_runner_version
        );
    }
    if (!empty($misc->config)) {
        $c = $misc->config;
        if (!empty($c->dont_use_wsl)) {
            $x .= "<br>Config: don't use WSL";
        }
        if (!empty($c->dont_use_docker)) {
            $x .= "<br>Config: don't use Docker";
        }
    }
    return $x;
}


// return a human-readable version of the GPU info
//
function gpu_desc($misc) {
    if (empty($misc->gpus)) return '---';
    $gpus = [];
    foreach ($misc->gpus as $g) {
        $x = sprintf(
            '%d %s; %dMB RAM',
            $g->count, $g->model, $g->ram_mb
        );
        if (!empty($g->driver_version)) {
            $x .= "; Driver: $g->driver_version";
        }
        if (!empty($g->opencl_version)) {
            $x .= "; OpenCL: $g->opencl_version";
        }
        $gpus[] = $x;
    }
    return implode('<br>', $gpus);
}

// Given the same string as above, return the BOINC version
//
function boinc_version($misc) {
    if (empty($misc->client_version)) {
        return '---';
    }
    $x = $misc->client_version;
    if (!empty($misc->client_brand)) {
        $x .= " ($misc->client_brand)";
    }
    return $x;
}

function cpu_desc($host) {
    return "$host->p_vendor<br>$host->p_model<br>".tra("(%1 cores)", $host->p_ncpus)."\n";
}

// If private is true, we're showing the host to its owner,
// so it's OK to show the domain name etc.
// If private is false, show the owner's name only if they've given permission
//
function show_host_row($host, $i, $private, $show_owner, $any_product_name) {
    $anonymous = false;
    if (!$private) {
        if ($show_owner) {
            $user = BoincUser::lookup_id($host->userid);
            if ($user && $user->show_hosts) {
            } else {
                $anonymous = true;
            }
        }
    }
    echo "<tr><td>ID: $host->id
        <br><a href=show_host_detail.php?hostid=$host->id>".tra("Details")."</a>
    ";
    if (!NO_COMPUTING) {
        echo "
        | <a href=results.php?hostid=$host->id>".tra("Tasks")."</a>
        ";
    }
    if (!NO_STATS) {
        if (!$anonymous) {
            echo "
                <br><nobr><small>".tra("Cross-project stats:")."</small></nobr><br>".cross_project_links($host);
        }
    }
    echo "
        </td>
    ";
    if ($private) {
        echo "<td>$host->domain_name</td>\n";
        if ($any_product_name) {
            echo "<td>$host->product_name</td>\n";
        }
        echo "<td>$host->venue</td>\n";
    } else {
        echo "<td>$i</td>\n";
        if ($show_owner) {
            if ($anonymous) {
                echo "<td>".tra("Anonymous")."</td>\n";
            } else {
                echo "<td>", user_links($user, BADGE_HEIGHT_MEDIUM), "</td>\n";
            }
        }
    }
    $misc = json_decode($host->misc);
    if ($show_owner) {
        // This is used in the "top computers" display
        //
        if (!NO_STATS) {
            printf("
                <td align=right>%s</td>
                <td align=right>%s</td>",
                format_credit($host->expavg_credit),
                format_credit_large($host->total_credit)
            );
        }
        printf("
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s <br> %s</td>",
            boinc_version($misc),
            cpu_desc($host),
            gpu_desc($misc),
            $host->os_name, $host->os_version
        );
    } else {
        // This is used to show the computers of a given user
        //
        if (!NO_STATS) {
            printf("
                <td align=right>%s</td>
                <td align=right>%s</td>",
                format_credit($host->expavg_credit),
                format_credit_large($host->total_credit)
            );
        }
        printf("
            <td>%s</td>
            <td>%s</td>
            <td>%s</td>
            <td>%s<br><small>%s</small></td>
            <td>%s</td>
            ",
            boinc_version($misc),
            cpu_desc($host),
            gpu_desc($misc),
            $host->os_name, $host->os_version,
            sched_log_link($host->rpc_time)
        );
    }

    echo "</tr>\n";
}

// Logic for deciding whether two host records might actually
// be the same machine, based on CPU info
//
// p_vendor is typically either AuthenticAMD or GenuineIntel.
// Over time we've changed the contents of p_model.
// Some examples:
// Intel(R) Core(TM)2 Duo CPU  E7300  @ 2.66GHz [Family 6 Model 23 Stepping 6]
// AMD Athlon(tm) II X2 250 Processor [Family 16 Model 6 Stepping 3]
// Intel(R) Xeon(R) CPU X5650 @ 2.67GHz [x86 Family 6 Model 44 Stepping 2]
// Intel(R) Core(TM) i5-2500K CPU @ 3.30GHz [Intel64 Family 6 Model 42 Stepping 7]
//
// in the last 2 cases, let's call x86 and Intel64 the "architecture"
//
// so, here's the policy:
//
// if p_ncpus different, return false
// if p_vendor different, return false
// if both have family/model/stepping info
//    if info disagrees, return false
//    if both have GHz info, and they disagree, return false
//    if both have architecture, and they disagree, return false
//    return true
// if p_model different, return false
// return true
//

// parse p_model to produce the following structure:
// x->speed     "3.00GHz" etc. or null
// x->arch      "x86" etc. or null
// x->info      "Family 6 Model 23 Stepping 6" etc. or null
//
function parse_model($model) {
    $y = explode(" ", $model);
    $x = new StdClass;
    $x->speed = null;
    $x->arch = null;
    $x->info = null;
    foreach ($y as $z) {
        if (strstr($z, "GHz")) $x->speed = $z;
        if (strstr($z, "MHz")) $x->speed = $z;
    }
    $pos1 = strpos($model, '[');
    if ($pos1 === false) return $x;
    $pos2 = strpos($model, ']');
    if ($pos2 === false) return $x;
    $a = substr($model, $pos1+1, $pos2-$pos1-1);
    $y = explode(" ", $a);
    if (count($y) == 0) return $x;
    if ($y[0] == "Family") {
        $x->info = $a;
    } else {
        $x->arch = $y[0];
        $x->info = substr($a, strlen($y[0])+1);
    }
    return $x;
}

function cpus_compatible($host1, $host2) {
    if ($host1->p_ncpus != $host2->p_ncpus) return false;
    if ($host1->p_vendor != $host2->p_vendor) return false;
    $x1 = parse_model($host1->p_model);
    $x2 = parse_model($host2->p_model);
    if ($x1->info && $x2->info) {
        if ($x1->info != $x2->info) return false;
        if ($x1->speed && $x2->speed) {
            if ($x1->speed != $x2->speed) return false;
        }
        if ($x1->arch && $x2->arch) {
            if ($x1->arch != $x2->arch) return false;
        }
        return true;
    }
    if ($host1->p_model != $host2->p_model) return false;
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
function hosts_compatible($host1, $host2, $show_detail) {
    // A host is "new" if it has no credit and no results.
    // Skip disjoint-time check if one host or other is new
    //
    $new1 = !$host1->total_credit && !host_nresults($host1);
    $new2 = !$host2->total_credit && !host_nresults($host2);
    if (!$new1 && !$new2) {
        if (!times_disjoint($host1, $host2)) {
            if ($show_detail) {
                $c1 = date_str($host1->create_time);
                $r1 = date_str($host1->rpc_time);
                $c2 = date_str($host2->create_time);
                $r2 = date_str($host2->rpc_time);
                echo "<br>".tra("Host %1 has overlapping lifetime:", $host2->id)." ($c1 - $r1), ($c2 - $r2)";
            }
            return false;
        }
    }
    if (!os_compatible($host1, $host2)) {
        if ($show_detail) {
            echo "<br>".tra("Host %1 has an incompatible OS:", $host2->id)." ($host1->os_name, $host2->os_name)\n";
        }
        return false;
    }
    if (!cpus_compatible($host1, $host2)) {
        if ($show_detail) {
            echo "<br>".tra("Host %1 has an incompatible CPU:", $host2->id)." ($host1->p_vendor $host1->p_model, $host2->p_vendor $host2->p_model)\n";
        }
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

    $results = BoincResult::enum("hostid=$hostid order by received_time");
    foreach($results as $result) {
        if ($result->granted_credit <= 0) continue;
        $total += $result->granted_credit;

        update_average(
            $result->received_time,
            $result->sent_time,
            $result->granted_credit,
            $avg,
            $avg_time
        );

        //echo "<br>$avg\n";
    }

    // do a final decay
    //
    $now = time();
    update_average(now, 0, 0, $avg, $avg_time);

    $host = new BoincHost();
    $host->id = hostid;
    $host->update("total_credit=$total, expavg_credit=$avg, expavg_time=$now");
}

// decay a host's average credit
//
function host_decay_credit($host) {
    $avg = $host->expavg_credit;
    $avg_time = $host->expavg_time;
    $now = time();
    update_average($now, 0, 0, $avg, $avg_time);
    $host->update("expavg_credit=$avg, expavg_time=$now");
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
        return tra("same host");
    }
    if (!hosts_compatible($old_host, $new_host, false)) {
        return tra("Can't merge host %1 into %2 - they're incompatible", $old_host->id, $new_host->id);
    }

    echo "<br>".tra("Merging host %1 into host %2", $old_host->id, $new_host->id)."\n";

    // decay the average credit of both hosts
    //
    $now = time();
    update_average($now, 0, 0, $old_host->expavg_credit, $old_host->expavg_time);
    update_average($now, 0, 0, $new_host->expavg_credit, $new_host->expavg_time);

    // update the database:
    // - add credit from old to new host
    // - change results to refer to new host
    // - put old host in "zombie" state (userid=0, rpc_seqno=new host ID)
    //
    $total_credit = $old_host->total_credit + $new_host->total_credit;
    $recent_credit = $old_host->expavg_credit + $new_host->expavg_credit;
    $result = $new_host->update("total_credit=$total_credit, expavg_credit=$recent_credit, expavg_time=$now");
    if (!$result) {
        return tra("Couldn't update credit of new computer");
    }
    $result = BoincResult::update_aux("hostid=$new_host->id where hostid=$old_host->id");
    if (!$result) {
        return tra("Couldn't update results");
    }

    $result = $old_host->update("total_credit=0, expavg_credit=0, userid=0, rpc_seqno=$new_host->id");
    if (!$result) {
        return tra("Couldn't retire old computer");
    }
    echo "<br>".tra("Retired old computer %1", $old_host->id)."\n";
    return 0;
}

//////////////// helper functions for hosts_user.php ////////////////

function link_url($sort, $rev, $show_all) {
    global $userid;
    $x = $userid ? "&userid=$userid":"";
    return "hosts_user.php?sort=$sort&rev=$rev&show_all=$show_all$x";
}

function link_url_rev($actual_sort, $sort, $rev, $show_all) {
    if ($actual_sort == $sort) {
        $rev = 1 - $rev;
    }
    return link_url($sort, $rev, $show_all);
}

function more_or_less($sort, $rev, $show_all) {
    echo "<p>";
    if ($show_all) {
        $url = link_url($sort, $rev, 0);
        echo tra("Show:")." ".tra("All computers")." &middot; <a href=$url>".tra("Only computers active in past 30 days")."</a>";
    } else {
        $url = link_url($sort, $rev, 1);
        echo tra("Show:")." <a href=$url>".tra("All computers")."</a> &middot; ".tra("Only computers active in past 30 days");
    }
    echo "<p>";
}

function user_host_table_start(
    $private, $sort, $rev, $show_all, $any_product_name
) {
    start_table('table-striped');
    $x = array();
    $a = array();

    $url = link_url_rev($sort, "id", $rev, $show_all);
    $x[] = "<a href=$url>".tra("Computer ID")."</a>";
    $a[] = '';

    if ($private) {
        $url = link_url_rev($sort, "name", $rev, $show_all);
        $x[] = "<a href=$url>".tra("Name")."</a>";
        $a[] = null;
        $url = link_url_rev($sort, "venue", $rev, $show_all);
        if ($any_product_name) {
            $x[] = tra("Model");
            $a[] = null;
        }
        $x[] = "<a href=$url>".tra("Location")."</a>";
        $a[] = null;
    } else {
        $x[] = tra("Rank");
        $a[] = null;
    }
    if (!NO_STATS) {
        $url = link_url_rev($sort, "expavg_credit", $rev, $show_all);
        $x[] = "<a href=$url>".tra("Avg. credit")."</a>";
        $a[] = ALIGN_RIGHT;
        $url = link_url_rev($sort, "total_credit", $rev, $show_all);
        $x[] = "<a href=$url>".tra("Total credit")."</a>";
        $a[] = ALIGN_RIGHT;
    }
    $x[] = tra("BOINC<br>version");
    $a[] = null;
    $url = link_url_rev($sort, "cpu", $rev, $show_all);
    $x[] = "<a href=$url>".tra("CPU")."</a>";
    $a[] = null;
    $x[] = tra("GPU");
    $a[] = null;
    $url = link_url_rev($sort, "os", $rev, $show_all);
    $x[] = "<a href=$url>".tra("Operating System")."</a>";
    $a[] = null;
    $url = link_url_rev($sort, "rpc_time", $rev, $show_all);
    $x[] = "<a href=$url>".tra("Last contact")."</a>";
    $a[] = null;
    row_heading_array($x, $a, "bg-default");
}

function show_user_hosts($userid, $private, $show_all, $sort, $rev) {
    $desc = false;  // whether the sort order's default is decreasing
    switch ($sort) {
    case "total_credit": $sort_clause = "total_credit"; $desc = true; break;
    case "expavg_credit": $sort_clause = "expavg_credit"; $desc = true; break;
    case "name": $sort_clause = "domain_name"; break;
    case "id": $sort_clause = "id"; break;
    case "cpu": $sort_clause = "p_vendor"; break;
    case "os": $sort_clause = "os_name"; break;
    case "venue": $sort_clause = "venue"; break;
    default:
        // default value -- sort by RPC time
        $sort = "rpc_time";
        $sort_clause = "rpc_time";
        $desc = true;
    }

    if ($rev != $desc) {
        $sort_clause .= " desc";
    }
    more_or_less($sort, $rev, $show_all);

    $now = time();
    $old_hosts=0;
    $i = 1;
    $hosts = BoincHost::enum("userid=$userid order by $sort_clause limit 100");
    $any_product_name = false;
    foreach ($hosts as $host) {
        if ($host->product_name) {
            $any_product_name = true;
            break;
        }
    }
    user_host_table_start($private, $sort, $rev, $show_all, $any_product_name);
    foreach ($hosts as $host) {
        $is_old=false;
        if (($now - $host->rpc_time) > 30*86400) {
            $is_old=true;
            $old_hosts++;
        }
        if (!$show_all && $is_old) continue;
        show_host_row($host, $i, $private, false, $any_product_name);
        $i++;
    }
    end_table();

    if ($old_hosts>0) {
        more_or_less($sort, $rev, $show_all);
    }

    if ($private) {
        echo "
            <a href=merge_by_name.php>".tra("Merge computers by name")."</a>
        ";
    }
}

// remove user-specific info from a user's hosts
//
function anonymize_hosts($user) {
    $hosts = BoincHost::enum("userid=$user->id");
    foreach ($hosts as $h) {
        $h->update("domain_name='deleted', last_ip_addr=''");
    }
}


?>
