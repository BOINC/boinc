#!/usr/bin/env php
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

// Find the status of remote server processes
// and write it to a serialized file.
// Run this as a periodic task; in general it can't run as a web script
// since the apache user can't ssh.

require_once("../inc/util.inc");

// see if a web server is running on the host
//
function check_web_server($host) {
    $url = "http://$host";
    $x = file_get_contents($url);
    if ($x) return true;
    return false;
}

// see if the given program is running on the host
//
function check_program($host, $program) {
    $cmd = "ssh $host 'ps -C $program'";
    $out = exec($cmd);
    if (strstr($out, $program)) return true;
    return false;
}

function main() {
    echo "--- Starting at ".time_str(time())." ---\n";
    $x = array();
    $c = simplexml_load_file("../../config.xml");
    if (!$c) {
        die("can't parse config file\n");
    }
    $config = $c->config;
    if ($config->uldl_host) {
        $y = new StdClass;
        $y->cmd = 'upload/download server';
        $y->host = (string)$config->uldl_host;
        if (check_web_server($config->uldl_host)) {
            //echo "uldl server running\n";
            $y->status = 1;
        } else {
            //echo "uldl server not running\n";
            $y->status = 0;
        }
        $x[] = $y;
    }
    $daemons = $c->daemons;
    foreach ($daemons->daemon as $d) {
        if ($d->host) {
            $cmd = explode(" ", $d->cmd);
            $prog = $cmd[0];
            $y = new StdClass;
            $y->cmd = (string)$d->cmd;
            $y->host = (string)$d->host;
            if (check_program($d->host, $prog)) {
                //echo "$d->host $d->cmd is running\n";
                $y->status = 1;
            } else {
                //echo "$d->host $d->cmd is not running\n";
                $y->status = 0;
            }
            $x[] = $y;
        }
    }
    $x = serialize($x);
    file_put_contents("../cache/remote_server_status", $x);
    echo "--- Finished at ".time_str(time())." ---\n";
}

main();
?>
