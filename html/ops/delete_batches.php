#! /usr/bin/env php

<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// delete retired batches with no WUs

require_once('../inc/boinc_db.inc');
require_once('../inc/submit_db.inc');
require_once('../inc/util.inc');

function main() {
    echo "------- Starting at ".time_str(time())."-------\n";
    $batches = BoincBatch::enum(sprintf('state=%d', BATCH_STATE_RETIRED));
    foreach ($batches as $batch) {
        $wus = BoincWorkunit::enum_fields(
            'id',
            "batch=$batch->id"
        );
        if (!$wus) {
            echo "deleting batch $batch->id\n";
            $batch->delete();
        }
    }
    echo "------- Finished at ".time_str(time())."-------\n";
}

// clean up results/.
// In theory should have to do this just once.
//
function cleanup_results() {
    foreach (scandir('../../results') as $d) {
        if ($d[0] == '.') continue;
        if (!is_numeric($d)) continue;
        $id = (int)$d;
        if (!$id) continue;
        $batch = BoincBatch::lookup_id($id);
        if ($batch) {
            if ($batch->state != BATCH_STATE_RETIRED) continue;
        }
        $dir = "../../results/$d";
        echo "deleting $dir\n";
        system("/bin/rm -rf $dir");
    }
}

main();
//cleanup_results();

?>
