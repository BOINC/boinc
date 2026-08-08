<?php

// identify possible botnet accounts:
// those with > MAX_HOSTS hosts
//
// could:
//      add other criteria like # of countries (based on IP addr)
//      consider only hosts with recent last RPC time

define ('MAX_HOSTS', 100);

require_once('../inc/boinc_db.inc');

// when hosts are merged, userid is set to zero
// and the rpc_seqno field is used to link to the non-zombie host.
// Follow links and return the latter.
//
function follow_links($id) {
    $orig_id = $id;
    $host = BoincHost::lookup_id($id);
    while ($host->userid == 0) {
        $host = BoincHost::lookup_id($host->rpc_seqno);
        if (!$host) {
            echo "(broken link in chain from host $orig_id)\n";
            return null;
        }
    }
    return $host;
}

function main() {
    $hosts = BoincHost::enum_fields('id, userid, last_ip_addr', '');
    $users = [];
    foreach ($hosts as $host) {
        if ($host->userid == 0) {
            $host = follow_links($host->id);
            if (!$host) continue;
        }
        if (array_key_exists($host->userid, $users)) {
            $users[$host->userid]++;
        } else {
            $users[$host->userid] = 1;
        }
    }

    foreach ($users as $id => $nhosts) {
        if ($nhosts < MAX_HOSTS) continue;
        $user = BoincUser::lookup_id($id);
        if ($user) {
            echo "$user->name has $nhosts hosts\n";
        } else {
            echo "unknown user $id has $nhosts hosts\n";
        }
    }
}

main();

?>
