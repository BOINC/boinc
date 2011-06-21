<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// example code for making a remote job submission

$ch = curl_init("http://foo.edu/test/submit.php");
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, "request=
    <batch_submit>
        <authenticator>xxx</authenticator>
        <batch>
        <app_name>uppercase</app_name>
        <job>
            <rsc_fpops_est>100e9</rsc_fpops_est>
            <command_line>--t ALPHA</command_line>
            <input_file>
                <source>http://foo.edu/index.php</source>
                <physical_name>name</physical_name>
            </input_file>
        </job>
        </batch>
    </batch_submit>
");
$reply = curl_exec($ch);
if (!$reply) die("HTTP op failed\n");
echo "reply: $reply\n"; exit;

$r = simplexml_load_string($reply);
if (!$r) die("bad reply: $reply\n");
$name = $r->getName();
if ($name == 'estimate') {
    echo "estimate: ".(string)$r->seconds."\n";
} else if ($name == 'error') {
    foreach ($r->message as $msg) {
        echo "$msg\n";
    }
} else {
    die("bad reply\n");
}

?>
