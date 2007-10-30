<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

class BoltDb extends DbConn {
    public static $instance;

    static function get() {
        if (!isset($instance)) {
            $config = get_config();
            $user = parse_config($config, '<bolt_db_user>');
            $passwd = parse_config($config, '<bolt_db_passwd>');
            $host = parse_config($config, '<bolt_db_host>');
            $name = parse_config($config, '<bolt_db_name>');
            if ($host == null) {
                $host = "localhost";
            }
            $instance = new DbConn();
            $retval = $instance->init_conn($user, $passwd, $host, $name);
            if (!$retval) return null;
        }
        return $instance;
    }
}

class BoltCourse {
    function insert() {
        $db = BoltDb::get();
        $now = time();
        $query = "insert into DBNAME.bolt_course (create_time, short_name, name, description, doc_file) values ($now, '$this->short_name', '$this->name', '$this->description', '$this->doc_file')";
        $result = $db->do_query($query);
        if (!$result) return false;
        $this->id = $db->insert_id();
        return true;
    }

    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_course', 'BoltCourse');
    }

    static function enum() {
        $db = BoltDb::get();
        return $db->enum('bolt_course', 'BoltCourse');
    }
}

class BoltEnrollment {
    function insert() {
        $db = BoltDb::get();
        $now = time();
        $query = "insert into DBNAME.bolt_enrollment (create_time, user_id, course_id) values ($now, $this->user_id, $this->course_id)";
        return $db->do_query($query);
    }
    function lookup($userid, $courseid) {
        $db = BoltDb::get();
        return $db->lookup('bolt_enrollment', 'BoltEnrollment', "user_id=$userid and course_id=$courseid");
    }
    function update_aux($clause) {
        $db = BoltDb::get();
        $db->update_aux('bolt_enrollment', $clause);
    }
}

class BoltView {
    function insert() {
        $db = BoltDb::get();
        $now = time();
        $query = "insert into DBNAME.bolt_view (user_id, course_id, item_name, start_time) values ($this->user_id, $this->course_id, '$this->item_name', $now)";
        $result = $db->do_query($query);
        if (!$result) return false;
        $this->id = $db->insert_id();
        return true;
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_view', 'BoltView');
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update($this, 'bolt_view', $clause);
    }
}

?>
