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

function upload_error_description($errno) {
    switch($errno) {
        case UPLOAD_ERR_INI_SIZE:
            return "The uploaded file exceeds upload_max_filesize of php.ini."; break;
        case UPLOAD_ERR_FORM_SIZE:
            return "The uploaded file exceeds the MAX_FILE_SIZE specified in the HTML form."; break;
        case UPLOAD_ERR_PARTIAL:
            return "The uploaded file was only partially uploaded."; break;
        case UPLOAD_ERR_NO_FILE:
            return "No file was uploaded."; break;
        case UPLOAD_ERR_NO_TMP_DIR:
            return "Missing a temporary folder."; break;
        case UPLOAD_ERR_CANT_WRITE:
            return "Failed to write file to disk."; break;
        case UPLOAD_ERR_EXTENSION:
            return "A PHP extension stopped the file upload."; break;
    }
}

function query_files($r) {
    xml_start_tag("query_files");
    list($user, $user_submit) = authenticate_user($r, null);
    $absent_files = array();
    $now = time();
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $i = 0;
    $md5s= array();
    foreach($r->md5 as $f) {
        $md5 = (string)$f;
        $md5s[] = $md5;
    }
    $md5s = array_unique($md5s);
    foreach($md5s as $md5) {
        $fname = job_file_name($md5);
        $path = dir_hier_path($fname, project_dir() . "/download", $fanout);

        // if the job_file record is there,
        // update the delete time first to avoid race condition
        // with job file deleter
        //
        $job_file = BoincJobFile::lookup_md5($md5);
        if ($job_file && $job_file->delete_time < $delete_time) {
            $retval = $job_file->update("delete_time=$delete_time");
            if ($retval) {
                xml_error(-1, "job_file->update() failed: ".BoincDb::error());
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
                if (!$jf_id) {
                    xml_error(-1, "query_file(): BoincJobFile::insert($md5) failed: ".BoincDb::error());
                }
            }
            // create batch association if needed
            //
            if ($batch_id) {
                $ret = BoincBatchFileAssoc::insert(
                    "(batch_id, job_file_id) values ($batch_id, $jf_id)"
                );
                if (!$ret) {
                    xml_error(-1,
                        "BoincBatchFileAssoc::insert() failed: ".BoincDb::error()
                    );
                }
            }
        } else {
            if ($job_file) {
                $ret = $job_file->delete();
                if (!$ret) {
                    xml_error(-1,
                        "BoincJobFile::delete() failed: ".BoincDb::error()
                    );
                }
            }
            $absent_files[] = $i;
        }
        $i++;
    }
    echo "<absent_files>\n";
    foreach ($absent_files as $i) {
        echo "<file>$i</file>\n";
    }
    echo "</absent_files>
        </query_files>
    ";
}

function upload_files($r) {
    xml_start_tag("upload_files");
    list($user, $user_submit) = authenticate_user($r, null);
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    //print_r($_FILES);
    $upload_error = "";
    $files_md5 = array();
    $files_upl = array();
    foreach ($r->md5 as $cs) {
        $files_md5[] = (string)$cs;
    }

    foreach ($_FILES as $f) {
        $name = $f['name'];
        $tmp_name = $f['tmp_name'];

        if ($f['error'] != UPLOAD_ERR_OK) {
            $reason = upload_error_description($f['error']);
            $upload_error .= "$name upload failed because: $reason; ";
            unlink($tmp_name);
            continue;
        }
        if (!is_uploaded_file($tmp_name)) {
            $upload_error .= "$name was not uploaded correctly; ";
            continue;
        }
        $md5 = md5_file($tmp_name);
        if (!in_array($md5, $files_md5)) {
            $upload_error .= "$name md5 value ($md5) missing in request XML; ";
            unlink($tmp_name);
            continue;
        } else {
            // remove md5 from array so we can check if all files are uploaded
            $files_md5 = array_diff($files_md5, array($md5));
        }
        $files_upl[] = array("name" => $name, "tmp_name" => $tmp_name, "size" => $f['size'], "md5" => $md5 );
    }

    if (count($files_md5) > 0) {
        $upload_error .= "More md5's specified in request XML than files uploaded; ";
        foreach ($files_upl as $f) {
            unlink($f['tmp_name']);
        }
    }

    if ($upload_error == "") {
        foreach ($files_upl as $f) {
            $tmp_name = $f['tmp_name'];
            $md5 = $f['md5'];
            $fname = job_file_name($md5);
            // TODO: apache should not have access to the whole download/ directory
            $path = dir_hier_path($fname, project_dir() . "/download", $fanout);
            if (!move_uploaded_file($tmp_name, $path)) {
                $upload_error .= "could not move $tmp_name to $path; ";
                unlink($tmp_name);
                continue;
            }
            $now = time();
            $jf_id = BoincJobFile::insert(
                "(md5, create_time, delete_time) values ('$md5', $now, $delete_time)"
            );
            if (!$jf_id) {
                $upload_error .= "BoincJobFile::insert($md5) failed: " . BoincDb::error() . " ";
                unlink($path);
                continue;
            }
            if ($batch_id) {
                // this is not considered serious but can not be reported right now
                BoincBatchFileAssoc::insert(
                    "(batch_id, job_file_id) values ($batch_id, $jf_id)"
                );
            }
        }
    }

    if ($upload_error != "") {
        // this will exit()
        xml_error(-1, "upload_files(): " . $upload_error);
    }
    echo "<success/>
        </upload_files>
    ";
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

$request_log = parse_config(get_config(), "<remote_submission_log>");
if ($request_log) {
    $request_log_dir = parse_config(get_config(), "<log_dir>");
    if ($request_log_dir) {
        $request_log = $request_log_dir . "/" . $request_log;
    }
    if ($file = fopen($request_log, "a+")) {
        fwrite($file, "\n<job_file date=\"" . date(DATE_ATOM) . "\">\n" . $_POST['request'] . "\n</job_file>\n");
        fclose($file);
    }
}

xml_header();
$req = $_POST['request'];
$r = simplexml_load_string($req);
if (!$r) {
    xml_error(-1, "can't parse request message: $req", __FILE__, __LINE__);
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
