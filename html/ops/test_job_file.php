#! /usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

// Test file upload handler for remote job submission (user/job_file.php)
// Change filepaths and md5's to your own values

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/submit.inc");

// global test configuration
$req = new StdClass;
$req->project = parse_config(get_config(), "<master_url>");
//$req->project       = "https://PROJECT_MASTER_URL/"; // use this to override the above value
$req->authenticator = "xxxx"; // this user must have remote submit permissions
//$req->batch_id = "1"; // optional, if used with query_files files will be added to this batch if they exist
//$req->delete_time = now()+14*3600; // optional, if used with query_files this will be updated if the file exists

// files to be uploaded
$upld_files = array();
// copy the next lines in order to specify more than one upload file
//$f_realpath = realpath('/var/spool/sixtrack/input/SixIn-1472649201.zip');
//$f_md5 = md5_file($f_realpath);
//$f_md5 = "833d1654e6bfa2cd7c8ca217d210e52a";
//$upld_files[] = array("path" => $f_realpath, "md5" => $f_md5);

// example test cases:
//$upld_files[] = array("path" => "", "md5" => md5("jobfileuploadtest"); // Should produce an error
//$upld_files[] = array("path" => $f_realpath, "md5" => ""; // Should produce an error

// files to be queried
$query_files = array();
//$query_files[] = array("md5" => "a7d5cbd6ef395e8a79ef29228076d38d");
//$query_files[] = array("md5" => "8167c3f9973b7fc85b6cb623644122d5");
//$query_files[] = array("md5" => "401324352d30888a5df2e5cc65035b17");
//$query_files[] = array("md5" => "401324352d30888a5df2e5cc65035b18");

function send_request($req, $xml, $files) {
    $ch = curl_init("$req->project/job_file.php");
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    $post = array();
    $post["request"] = $xml;
    if ($req->type == "upload") {
        $i=0;
        foreach ($files as $f) {
            if ($f['path'] != "") {
                $post["file_$i"] = '@'.$f['path'];
                $i++;
            }
        }
    }

    curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    $reply = curl_exec($ch);
    if ($reply) {
        print $reply . "\n";
    } else {
        print curl_error($ch) . "\n";
    }
    curl_close($ch);
}

function upload_test($req, $files) {
    $req->type = "upload";

    $xml = "<upload_files>
        <authenticator>$req->authenticator</authenticator>\n";
    if (isset($req->delete_time)) {
        $xml .= "    <delete_time>$req->delete_time</delete_time>\n";
    }
    if (isset($req->batch_id)) {
        $xml .= "    <batch_id>$req->batch_id</batch_id>\n";
    }
    foreach ($files as $f) {
        if ($f['md5'] != "") {
            $xml .= "    <md5>".$f['md5']."</md5>\n";
        }
    }

    $xml .= "</upload_files>";

    send_request($req, $xml, $files);
}

function query_test($req, $files) {
    $req->type = "query";

    $xml = "<query_files>
        <authenticator>$req->authenticator</authenticator>\n";
    if (isset($req->delete_time)) {
        $xml .= "    <delete_time>$req->delete_time</delete_time>\n";
    }
    if (isset($req->batch_id)) {
        $xml .= "    <batch_id>$req->batch_id</batch_id>\n";
    }
    foreach ($files as $f) {
        $xml .= "    <md5>".$f['md5']."</md5>\n";
    }
    $xml .= "</query_files>";

    send_request($req, $xml, $files);
}

// main
if ($argc != 2) {
    print("Usage: ".$argv[0]." [upload|query]");
}

switch ($argv[1]) {
case "upload":
    upload_test($req, $upld_files);
    break;
case "query":
    query_test($req, $query_files);
    break;
default:
    print("Usage: ".$argv[0]." [upload|query]");
}

?>
