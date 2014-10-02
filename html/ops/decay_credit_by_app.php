#! /usr/bin/env php

<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// script to decay exponential average per-app credit

require_once("../inc/credit.inc");
require_once("../inc/util.inc");

function decay($is_user) {
    $now = time();
    if ($is_user) {
        $cs = BoincCreditUser::enum("");
    } else {
        $cs = BoincCreditTeam::enum("");
    }
    foreach ($cs as $c) {
        update_average(
            $now, 0, 0, $c->expavg, $c->expavg_time
        );
        if ($is_user) {
            $c->update(
                "expavg=$c->expavg, expavg_time=$c->expavg_time where userid=$c->userid and appid=$c->appid"
            );
        } else {
            $c->update(
                "expavg=$c->expavg, expavg_time=$c->expavg_time where teamid=$c->teamid and appid=$c->appid"
            );
        }
    }
}

echo "Starting: ", time_str(time()), "\n";
decay(true);
decay(false);
echo "Ending: ", time_str(time()), "\n";

?>
