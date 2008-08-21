<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

define('BOLT_PHASE_STUDY', 1);
    // sequential progress through course
define('BOLT_PHASE_REVIEW', 2);
	// review and repeat of an exercise set
define('BOLT_PHASE_REFRESH', 3);
    // timed repeat of exercise set, and possibly review and repeat

define('BOLT_MODE_LESSON', 1);
define('BOLT_MODE_SHOW', 2);
define('BOLT_MODE_SCORE', 3);
define('BOLT_MODE_ANSWER', 4);
define('BOLT_MODE_FINISHED', 5);

define('BOLT_ACTION_NONE', 0);
define('BOLT_ACTION_NEXT', 1);
define('BOLT_ACTION_PREV', 2);
define('BOLT_ACTION_SUBMIT', 3);
define('BOLT_ACTION_QUESTION', 4);
define('BOLT_ACTION_COURSE_HOME', 5);
define('BOLT_ACTION_REVIEW', 6);
define('BOLT_ACTION_REPEAT', 7);

define('BOLT_COURSE_NOT_STARTED', 1);
define('BOLT_COURSE_STARTED', 2);
define('BOLT_COURSE_FINISHED', 3);

// bits in bolt_user.flags
define('BOLT_FLAGS_DEBUG', 1);   // print debugging info in output pages
define('BOLT_FLAGS_SHOW_ALL', 2);    // show hidden courses

class BoltDb extends DbConn {
    static $instance;

    static function get() {
        if (web_stopped()) {
            if ($generating_xml) {
                xml_error(-183);
            } else {
                page_head("Page not available");
                echo "This page requires database access.
                    Our database server is temporarily shut down for maintenance.
                    Please try again later.
                ";
                page_tail();
            }
            exit();
        }
        if (!isset($instance)) {
            $config = get_config();
            $name = parse_config($config, '<bolt_db_name>');
            if (!$name) {
                $name = parse_config($config, '<db_name>');
                $user = parse_config($config, '<db_user>');
                $passwd = parse_config($config, '<db_passwd>');
                $host = parse_config($config, '<db_host>');
            } else {
                $user = parse_config($config, '<bolt_db_user>');
                $passwd = parse_config($config, '<bolt_db_passwd>');
                $host = parse_config($config, '<bolt_db_host>');
            }
            if ($host == null) {
                $host = "localhost";
            }
            $instance = new DbConn();
            $retval = $instance->init_conn($user, $passwd, $host, $name);
            if (!$retval) return null;
        }
        return $instance;
    }
    static function escape_string($string) {
        $db = self::get();
        return $db->base_escape_string($string);
    }
}

class BoltUser {
    static $cache;
    static function lookup_userid($id) {
        $db = BoltDb::get();
        return $db->lookup('bolt_user', 'BoltUser', "user_id=$id");
    }
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_user', $clause);
    }
    static function lookup(&$user) {
        if (!$user) return;
        if (isset($user->bolt)) return;
        if (isset(self::$cache[$user->id])) {
            $bolt = self::$cache[$user->id];
        } else {
            $bolt = self::lookup_userid($user->id);
            if (!$bolt) {
                self::insert("(user_id) values ($user->id)");
                $bolt = self::lookup_userid($user->id);
            }
            self::$cache[$user->id] = $bolt;
        }
        $user->bolt = $bolt;
    }
    function update($clause) {
        $db = BoltDb::get();
        $clause = "$clause where user_id=$this->user_id";
        return $db->update_aux('bolt_user', $clause);
    }
}

class BoltCourse {
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_course', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_course', 'BoltCourse');
    }
    static function lookup_name($name) {
        $db = BoltDb::get();
        return $db->lookup('bolt_course', 'BoltCourse', "short_name='$name'");
    }
    static function enum() {
        $db = BoltDb::get();
        return $db->enum('bolt_course', 'BoltCourse');
    }
    function update($clause) {
        $db = BoltDb::get();
        return $db->update($this, 'bolt_course', $clause);
    }
    function doc_file() {
        $sn = $this->short_name;
        return "../inc/$sn.inc";
    }
}

class BoltEnrollment {
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_enrollment', $clause);
    }
    function lookup($user_id, $course_id) {
        $db = BoltDb::get();
        return $db->lookup('bolt_enrollment', 'BoltEnrollment', "user_id=$user_id and course_id=$course_id");
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update_aux('bolt_enrollment', "$clause where user_id=$this->user_id and course_id=$this->course_id");
    }
    static function delete($user_id, $course_id) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_enrollment', "user_id=$user_id and course_id=$course_id");
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_enrollment', $clause);
    }
}

class BoltView {
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_view', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_view', 'BoltView');
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update($this, 'bolt_view', $clause);
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_view', 'BoltView', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_view', $clause);
    }
}

class BoltResult {
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_result', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_result', 'BoltResult');
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_result', 'BoltResult', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_result', $clause);
    }
}

class BoltXsetResult {
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_xset_result', 'BoltXsetResult');
    }
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_xset_result', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_xset_result', 'BoltXsetResult', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_xset_result', $clause);
    }
}

class BoltRefreshRec {
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_refresh', 'BoltRefreshRec');
    }
    function lookup($clause) {
        $db = BoltDb::get();
        return $db->lookup('bolt_refresh', 'BoltRefreshRec', $clause);
    }
    static function replace($clause) {
        $db = BoltDb::get();
        return $db->replace('bolt_refresh', $clause);
    }
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_refresh', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_refresh', 'BoltRefreshRec', $clause);
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update($this, 'bolt_refresh', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_refresh', $clause);
    }
}

class BoltSelectFinished {
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_select_finished', $clause);
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_select_finished', 'BoltSelectFinished', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_select_finished', $clause);
    }
}

class BoltQuestion {
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_question', $clause);
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_question', 'BoltQuestion', $clause);
    }
    static function delete_aux($clause) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_question', $clause);
    }
}

// TODO: move this somewhere else, and think about whether it's correct
//
function bolt_course_status($course_id, $user_id) {
    $e = BoltEnrollment::lookup($user_id, $course_id);
    if (!$e) return BOLT_COURSE_NOT_STARTED;
    $view = BoltView::lookup_id($e->last_view_id);
    if ($view->fraction_done == 1) return BOLT_COURSE_FINISHED;
    return BOLT_COURSE_STARTED;
}

?>
