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

class BoincJobFile {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('job_file', $clause);
        if (!$ret) return false;
        return $db->insert_id();
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        return $db->lookup('job_file', 'BoincJobFile', "name='$name'");
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
}

?>
