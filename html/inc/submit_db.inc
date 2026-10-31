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

// Tables related to job submission

require_once("../inc/common_defs.inc");

#[AllowDynamicProperties]
class BoincBatch {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'batch', 'BoincBatch');
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        $name = BoincDb::escape_string($name);
        return $db->lookup('batch', 'BoincBatch', "name='$name'");
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('batch', 'BoincBatch', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('batch', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'batch', $clause);
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('batch', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'batch');
    }
    function get_cpu_time() {
        $db = BoincDb::get();
        $x = $db->get_double(
            "select sum(result.cpu_time) as total_cpu_time from workunit join result on workunit.id = result.workunitid where workunit.batch=$this->id",
            "total_cpu_time"
        );
        return $x;
    }
}

#[AllowDynamicProperties]
class BoincUserSubmit {
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('user_submit', 'BoincUserSubmit', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('user_submit', $clause);
        if (!$ret) return false;
        return true;
    }
    static function lookup_userid($user_id) {
        $db = BoincDb::get();
        return $db->lookup('user_submit', 'BoincUserSubmit', "user_id=$user_id");
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('user_submit', "$clause where user_id=$this->user_id");
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('user_submit', $clause);
    }

    static function delete_user($user_id) {
        $db = BoincDb::get();
        return $db->delete_aux('user_submit', "user_id = $user_id");
    }

}

#[AllowDynamicProperties]
class BoincUserSubmitApp {
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('user_submit_app', 'BoincUserSubmitApp', $clause);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('user_submit_app', 'BoincUserSubmitApp', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('user_submit_app', $clause);
        if (!$ret) return false;
        return true;
    }
    static function delete_user($user_id) {
        $db = BoincDb::get();
        return $db->delete_aux('user_submit_app', "user_id=$user_id");
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('user_submit_app', "$clause where user_id=$this->user_id and app_id=$this->app_id");
    }
}

#[AllowDynamicProperties]
class BoincJobFile {
    // Coordinate filesystem changes with uploads and queries, including when
    // the job_file row does not exist yet. Lock names are database-specific and
    // fit MySQL's 64-character limit. The connection also releases them on exit.
    private static function lock_name($name) {
        $db = BoincDb::get();
        return hash('sha256', "job_file:$db->db_name\0$name");
    }
    static function lock($name, $timeout=30) {
        $db = BoincDb::get();
        $key = self::lock_name($name);
        $timeout = (int)$timeout;
        $result = $db->do_query("select get_lock('$key', $timeout)");
        if (!$result) {
            throw new RuntimeException("could not lock job file $name");
        }
        $row = $result->fetch_row();
        $result->free();
        if (!$row || $row[0] === null) {
            throw new RuntimeException("could not lock job file $name");
        }
        return $row[0] == 1;
    }
    static function unlock($name) {
        $db = BoincDb::get();
        $key = self::lock_name($name);
        $result = $db->do_query("select release_lock('$key')");
        if (!$result) {
            throw new RuntimeException("could not unlock job file $name");
        }
        $row = $result->fetch_row();
        $result->free();
        if (!$row || $row[0] != 1) {
            throw new RuntimeException("could not unlock job file $name");
        }
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('job_file', $clause);
        if (!$ret) return false;
        return $db->insert_id();
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        $name = BoincDb::escape_string($name);
        return $db->lookup('job_file', 'BoincJobFile', "binary name='$name'");
    }
    static function enum($clause) {
        $db = BoincDb::get();
        $files = $db->enum('job_file', 'BoincJobFile', $clause);
        if ($db->db_conn->errno) {
            throw new RuntimeException("could not enumerate job files");
        }
        return $files;
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'job_file');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'job_file', $clause);
    }
}

#[AllowDynamicProperties]
class BoincBatchFileAssoc {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('batch_file_assoc', $clause);
        if (!$ret) return false;
        return true;
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('batch_file_assoc', 'BoincBatchFileAssoc', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete_aux('batch_file_assoc',
            "job_file_id=$this->job_file_id and batch_id=$this->batch_id"
        );
    }
    static function delete_batch($batch_id) {
        $db = BoincDb::get();
        return $db->delete_aux('batch_file_assoc',
            "batch_id=$batch_id"
        );
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('batch_file_assoc', $clause);
    }
}

?>
