<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// Web RPCs for managing job input files on the server.
//
// Issues:
//
// 1) how are files named?
//  Their name is a function of their MD5.
//  This eliminates issues related to file immutability
//
// 2) how do we keep track of the files?
//  In the MySQL database, in a table called "job_file".
//  Each row describes a file currently on the server.
//  In addition, we maintain a table "batch_file_assoc" to record
//  that a file is used by a particular batch.
//  (Note: the association could be at the job level instead.
//  but this way is more efficient if many jobs in a batch use
//  a particular file.)
//
// 3) how do we clean up unused files?
//  A daemon (job_file_deleter) deletes files for which
//  - the delete date (if given) is in the past, and
//  - there are no associations to active batches
//
// 4) what are the RPC operations?
//  query_files
//      in:
//          authenticator
//          list of MD5s
//          batch ID (optional)
//          new delete time (optional)
//      out:
//          error message,
//          or list of files (indices in the MD5 list) not present on server
//      action: for each MD5 in in the input list:
//          if present on server
//              update delete time
//              create batch/file association
//              add MD5 to output list
//  upload_files
//      in:
//          authenticator
//          delete time (optional)
//          batch ID (optional)
//          list of MD5s
//          files (as multipart attachments)
//      out:
//          error message, or success
//      action:
//          for each file in list
//              move to project download dir w/ appropriate name
//              create job_files record
//              create batch_file_assoc record if needed

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/xml.inc");
require_once("../inc/submit_util.inc");

function query_files($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $absent_files = array();
    $now = time();
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $i = 0;
    foreach($r->md5 as $f) {
        $md5 = (string)$f;
        echo "processing $md5\n";
        $fname = job_file_name($md5);
        $path = dir_hier_path($fname, "../../download", $fanout);

        // if the job_file record is there,
        // update the delete time first to avoid race condition
        // with job file deleter
        //
        $job_file = BoincJobFile::lookup_md5($md5);
        if ($job_file && $job_file->delete_time < $delete_time) {
            $retval = $job_file::update("delete_time=$delete_time");
            if ($retval) {
                xml_error(-1, "job_file::update() failed: "+mysql_error());
            }
        }
        if (file_exists($path)) {
            // create the DB record if needed
            //
            if ($job_file) {
                $jf_id = $job_file->id;
            } else {
                $jf_id = BoincJobFile::insert(
                    "(md5, create_time, delete_time) values ('$md5', $now, $delete_time)"
                );
            }
            // create batch association if needed
            //
            if ($batch_id) {
                $ret = BoincBatchFileAssoc::insert(
                    "(batch_id, job_file_id) values ($batch_id, $jf_id)"
                );
                if (!$ret) {
                    xml_error(-1,
                        "BoincBatchFileAssoc::insert() failed: "+mysql_error()
                    );
                }
            }
        } else {
            if ($job_file) {
                $job_file->delete();
            }
            $absent_files[] = $i;
        }
        $i++;
    }
    echo "<absent_files>\n";
    foreach ($absent_files as $i) {
        echo "<file>$i</file>\n";
    }
    echo "</absent_files>\n";
}

function upload_files($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    //print_r($_FILES);
    $i = 0;
    foreach ($r->md5 as $f) {
        $md5 = (string)$f;
        $name = "file_$i";
        $tmp_name = $_FILES[$name]['tmp_name'];
        if (!is_uploaded_file($tmp_name)) {
            xml_error(-1, "$tmp_name is not an uploaded file");
        }
        $fname = job_file_name($md5);
        $path = dir_hier_path($fname, "../../download", $fanout);
        rename($tmp_name, $path);
        $now = time();
        $jf_id = BoincJobFile::insert(
            "(md5, create_time, delete_time) values ('$md5', $now, $delete_time)"
        );
        if (!$jf_id) {
            xml_error(-1, "BoincJobFile::insert($md5) failed: "+mysql_error());
        }
        if ($batch_id) {
            BoincBatchFileAssoc::insert(
                "(batch_id, job_file_id) values ($batch_id, $jf_id)"
            );
        }
        $i++;
    }
    echo "<success/>\n";
}

if (0) {
$r = simplexml_load_string("<query_files>\n<batch_id>0</batch_id>\n   <md5>80bf244b43fb5d39541ea7011883b7e0</md5>\n   <md5>a6037b05afb05f36e6a85a7c5138cbc1</md5>\n</query_files>\n ");
submit_batch($r);
exit;
}
if (0) {
    $r = simplexml_load_string("<upload_files>\n<authenticator>157f96a018b0b2f2b466e2ce3c7f54db</authenticator>\n<batch_id>1</batch_id>\n<md5>80bf244b43fb5d39541ea7011883b7e0</md5>\n<md5>a6037b05afb05f36e6a85a7c5138cbc1</md5>\n</upload_files>");
    upload_files($r);
    exit;
}

xml_header();
$r = simplexml_load_string($_POST['request']);
if (!$r) {
    xml_error(-1, "can't parse request message");
}

switch($r->getName()) {
case 'query_files':
    query_files($r);
    break;
case 'upload_files':
    upload_files($r);
    break;
default:
    xml_error(-1, "no such action");
}

?>
