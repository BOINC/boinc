<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// Web RPCs for managing input files for remote job submission
// These support systems where users - possibly lots of them -
// process jobs without logging on to the BOINC server.
//
// Issues:
//
// 1) how are files named on the server (i.e. physical names)?
//  That's up to the clients, but they must enforce immutability:
//  different files must have different physical names.
//  One way to achieve this is to include the MD5 in the name.
//
// 2) how does the server keep track of the files?
//  In the MySQL database, in a table called "job_file".
//  Each row describes a file currently on the server.
//  In addition, we maintain a table "batch_file_assoc" to record
//  that a file is used by a particular batch.
//  (Note: the association could be at the job level instead.
//  but this way is more efficient if many jobs in a batch use
//  a particular file.)
//
// 3) how does the server clean up unused files?
//  A daemon (job_file_deleter) deletes files for which
//  - the delete date (if given) is in the past, and
//  - there are no associations to active batches
//
// 4) what are the RPC operations?
//  query_files
//      in:
//          authenticator
//          list of physical names
//          batch ID (optional)
//          new delete time (optional)
//      out:
//          error message,
//          or list of files (indices in the name list) not present on server
//      action: for each name in the name list:
//          if present on server
//              update delete time
//              create batch/file association
//  upload_files
//      in:
//          authenticator
//          delete time (optional)
//          batch ID (optional)
//          list of names
//          files (as multipart attachments)
//      out:
//          error message, or success
//      action:
//          for each file in list
//              stage:
//                  move to project download dir w/ appropriate name
//                  generate .md5 file
//              create job_files record
//              create batch_file_assoc record if needed

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/xml.inc");
require_once("../inc/submit_util.inc");

function upload_error_description($errno) {
    switch ($errno) {
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
    $user = check_remote_submit_permissions($r, null);
    $absent_files = [];
    $now = time();
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $phys_names = [];
    foreach ($r->phys_name as $f) {
        $phys_names[] = (string)$f;
    }
    $i = 0;
    foreach ($phys_names as $fname) {
        if (!is_valid_filename($fname)) {
            xml_error(-1, 'bad filename');
        }
        $path = dir_hier_path($fname, project_dir()."/download", $fanout);

        // if the job_file record is there,
        // update the delete time first to avoid race condition
        // with job file deleter
        //
        $job_file = BoincJobFile::lookup_name($fname);
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
                    "(name, create_time, delete_time) values ('$fname', $now, $delete_time)"
                );
                if (!$jf_id) {
                    xml_error(-1, "query_files(): BoincJobFile::insert($fname) failed: ".BoincDb::error());
                }
            }
            // create batch association if needed
            //
            if ($batch_id) {
                BoincBatchFileAssoc::insert(
                    "(batch_id, job_file_id) values ($batch_id, $jf_id)"
                );
                // this return error if assoc already exists; ignore
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

// if an error occurs, delete the uploaded temp files
//
function delete_uploaded_files() {
    foreach ($_FILES as $f) {
        unlink($f['tmp_name']);
    }
}

function upload_files($r) {
    xml_start_tag("upload_files");
    $user = check_remote_submit_permissions($r, null);
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $delete_time = (int)$r->delete_time;
    $batch_id = (int)$r->batch_id;
    //print_r($_FILES);

    if (count($_FILES) != count($r->phys_name)) {
        delete_uploaded_files();
        xml_error(-1,
            sprintf("# of uploaded files (%d) doesn't agree with request (%d)",
                count($_FILES), count($r->phys_name)
            )
        );
    }

    $phys_names = array();
    foreach ($r->phys_name as $cs) {
        $fname = (string)$cs;
        if (!is_valid_filename($fname)) {
            xml_error(-1, 'bad filename');
        }
        $phys_names[] = $fname;
    }

    foreach ($_FILES as $f) {
        $name = $f['name'];
        if (!is_valid_filename($fname)) {
            xml_error(-1, 'bad FILES filename');
        }
        $tmp_name = $f['tmp_name'];

        if ($f['error'] != UPLOAD_ERR_OK) {
            delete_uploaded_files();
            $reason = upload_error_description($f['error']);
            xml_error(-1, "$name upload failed because: $reason");
        }

        if (!is_uploaded_file($tmp_name)) {
            delete_uploaded_files();
            xml_error(-1, "$name was not uploaded correctly");
        }
    }

    $i = 0;
    $now = time();
    foreach ($_FILES as $f) {
        $tmp_name = $f['tmp_name'];
        $fname = $phys_names[$i];
        $path = dir_hier_path($fname, project_dir()."/download", $fanout);

        // see if file is in download hierarchy
        //
        switch (check_download_file($tmp_name, $path)) {
        case 0:
            // file is already there
            // note: check_download_file() generates .md5 in cases 1 and 2
            break;
        case 1:
            // file is not there; move
            //
            if (!move_uploaded_file($tmp_name, $path)) {
                xml_error(-1, "could not move $tmp_name to $path");
            }
            touch("$path.md5");
            break;
        case -1:
            // file is there but different contents
            //
            xml_error(-1, "file immutability violation for $fname");
        case -2:
            xml_error(-1, "file operation failed; check permissions in download/*");
        }

        $jf_id = BoincJobFile::insert(
            "(name, create_time, delete_time) values ('$fname', $now, $delete_time)"
        );
        if (!$jf_id) {
            xml_error(-1, "BoincJobFile::insert($fname) failed: ".BoincDb::error());
        }
        if ($batch_id) {
            BoincBatchFileAssoc::insert(
                "(batch_id, job_file_id) values ($batch_id, $jf_id)"
            );
        }
        $i++;
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
        $request_log = $request_log_dir."/".$request_log;
    }
    if ($file = fopen($request_log, "a+")) {
        fwrite($file, "\n<job_file date=\"".date(DATE_ATOM)."\">\n".$_POST['request']."\n</job_file>\n");
        fclose($file);
    }
}

xml_header();
$req = $_POST['request'];
$r = simplexml_load_string($req);
if (!$r) {
    xml_error(-1, "can't parse request message: ".htmlspecialchars($req), __FILE__, __LINE__);
}

switch ($r->getName()) {
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
