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
//
// "remote" means a machine other than the project's web server.
//
// In config.xml, hosts are identified by machine name (not domain name).
//
// The web server machine name may be specified by <www_host> in config.xml.
// If this is not specified, it's assumed to be the project's main host
// as specified by <host> in config.xml
//
// the specification of a daemon in config.xml may have a <host> element;
// if absent, it runs on the main host.
//
// the upload and download servers are specified by URL.
// To decide if these are the same as the web server,
// we compare the host part of the URLs with the host part of the master URL.

require_once("../inc/util.inc");

// see if a web server is running at the given URL
//
function web_server_running($url) {
    $u = parse_url($url);
    $url = $u['scheme']."://".$u['host'];
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_NOBODY, true);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_exec($ch);
    $retcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    $retcode = (int)($retcode/100);
    return ($retcode == 2 || $retcode == 4);
}

// see if the given daemon is running on the host by checking its PID
//
function is_daemon_running($host, $d) {
    if ($d->pid_file) {
        $path = "../../pid_$host/".(string)$d->pid_file;
    } else {
        $cmd = explode(" ", $d->cmd);
        $prog = $cmd[0];
        $path = "../../pid_$host/$prog".".pid";
    }
    $pid = trim(@file_get_contents($path));
    if ($pid === false) return false;
    $cmd = "ssh $host 'ps $pid'";
    $out = exec($cmd);
    if (strstr($out, $pid)) return true;
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
    $master_url = (string) $config->master_url;
    $u = parse_url($master_url);
    $master_host = $u["host"];

    $url = (string) $config->download_url;
    $u = parse_url($url);
    $h = $u["host"];
    if ($h != $master_host) {
        $y = new StdClass;
        $y->cmd = 'Download server';
        $y->host = $h;
        $y->status = web_server_running($url)?1:0;
        $x[] = $y;
    }
    $url = (string) $config->upload_url;
    $u = parse_url($url);
    $h = $u["host"];
    if ($h != $master_host) {
        $y = new StdClass;
        $y->cmd = 'Upload server';
        $y->host = $h;
        $y->status = web_server_running($url)?1:0;
        $x[] = $y;
    }

    $main_host = (string)$c->config->host;
    if ($c->config->www_host) {
        $web_host = (string) $c->config->www_host;
    } else {
        $web_host = $main_host;
    }
    $daemons = $c->daemons;
    foreach ($daemons->daemon as $d) {
        if ((int)$d->disabled != 0) continue;
        $host = $d->host?(string)$d->host:$main_host;
        if ($host != $web_host) {
            $y = new StdClass;
            $y->cmd = (string)$d->cmd;
            $y->host = $host;
            if (is_daemon_running($host, $d)) {
                //echo "$host $d->cmd is running\n";
                $y->status = 1;
            } else {
                //echo "$host $d->cmd is not running\n";
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
