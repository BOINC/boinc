<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util_basic.inc");

define ('VALIDATE_STATE_INIT', 0);
define ('VALIDATE_STATE_VALID', 1);
define ('VALIDATE_STATE_INVALID', 2);

class BossaDb extends DbConn {
    public static $instance;

    static function get() {
        if (!isset($instance)) {
            $config = get_config();
            $name = parse_config($config, '<bossa_db_name>');
            if (!$name) {
                $name = parse_config($config, '<db_name>');
                $user = parse_config($config, '<db_user>');
                $passwd = parse_config($config, '<db_passwd>');
                $host = parse_config($config, '<db_host>');
            } else {
                $user = parse_config($config, '<bossa_db_user>');
                $passwd = parse_config($config, '<bossa_db_passwd>');
                $host = parse_config($config, '<bossa_db_host>');
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
    static function start_transaction() {
        $db = BossaDb::get();
        $db->do_query("start transaction");
    }
    static function commit() {
        $db = BossaDb::get();
        $db->do_query("commit");
    }
}

class BossaUser {
    static $cache;
    static function lookup_userid($id) {
        $db = BossaDb::get();
        return $db->lookup('bossa_user', 'BossaUser', "user_id=$id");
    }
    static function insert($clause) {
        $db = BossaDb::get();
        return $db->insert('bossa_user', $clause);
    }
    static function lookup(&$user) {
        if (!$user) return;
        if (isset($user->bossa)) return;
        if (isset(self::$cache[$user->id])) {
            $bossa = self::$cache[$user->id];
        } else {
            $bossa = self::lookup_userid($user->id);
            if (!$bossa) {
                self::insert("(user_id) values ($user->id)");
                $bossa = self::lookup_userid($user->id);
            }
            self::$cache[$user->id] = $bossa;
        }
        $user->bossa = $bossa;
    }
    function update($clause) {
        $db = BossaDb::get();
        $clause = "$clause where user_id=$this->user_id";
        return $db->update_aux('bossa_user', $clause);
    }
}

class BossaApp {
    static function insert($clause) {
        $db = BossaDb::get();
        return $db->insert('bossa_app', $clause);
    }

    static function lookup($clause) {
        $db = BossaDb::get();
        return $db->lookup('bossa_app', 'BossaApp', $clause);
    }

    static function lookup_id($id) {
        $db = BossaDb::get();
        return $db->lookup_id($id, 'bossa_app', 'BossaApp');
    }

    static function enum() {
        $db = BossaDb::get();
        return $db->enum('bossa_app', 'BossaApp');
    }
    function update($clause) {
        $db = BossaDb::get();
        return $db->update($this, 'bossa_app', $clause);
    }
}

// values for bossa_job.state
//
define("BOSSA_JOB_EMBARGOED", 0);
define("BOSSA_JOB_IN_PROGRESS", 1);
define("BOSSA_JOB_DONE", 2);
define("BOSSA_JOB_INCONCLUSIVE", 3);

class BossaJob {
    static function insert($clause) {
        $db = BossaDb::get();
        $ret = $db->insert('bossa_job', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }
    function update($clause) {
        $db = BossaDb::get();
        return $db->update($this, 'bossa_job', $clause);
    }
    static function lookup_id($id) {
        $db = BossaDb::get();
        return $db->lookup_id($id, 'bossa_job', 'BossaJob');
    }
    static function enum($clause) {
        $db = BossaDb::get();
        return $db->enum('bossa_job', 'BossaJob', $clause);
    }
    static function count($clause) {
        $db = BossaDb::get();
        return $db->count('bossa_job', $clause);
    }

    // app-callable:
    //
    function get_info() {
        return unserialize($this->info);
    }
    function set_priority($x) {
        return $this->update("priority_0=$x");
    }
    function get_instances() {
        return BossaJobInst::enum("job_id = $this->id");
    }
    function get_finished_instances() {
        return BossaJobInst::enum("job_id = $this->id and finish_time>0");
    }
    function update_state($s) {
        $this->update("state=$s");
    }
}

class BossaJobInst {
    function insert($clause) {
        $db = BossaDb::get();
        $ret = $db->insert('bossa_job_inst', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }

    static function lookup_id($id) {
        $db = BossaDb::get();
        return $db->lookup_id($id, 'bossa_job_inst', 'BossaJobInst');
    }
    static function enum($clause) {
        $db = BossaDb::get();
        return $db->enum('bossa_job_inst', 'BossaJobInst', $clause);
    }

    function update($clause) {
        $db = BossaDb::get();
        return $db->update($this, 'bossa_job_inst', $clause);
    }

    // Assign a job from the given app to the given user.
    // Returns the job instance or NULL.
    //
    static function assign($app, $user) {
        $db = BossaDb::get();

        // first look for unfinished jobs previously assigned to this user
        //
        $query = "select * from DBNAME.bossa_job_inst where app_id=$app->id and user_id=$user->id and finish_time=0 limit 1";
        $result = $db->do_query($query);
        if ($result) {
            $ji = mysql_fetch_object($result, 'BossaJobInst');
            mysql_free_result($result);
            if ($ji) return $ji;
        }

        if ($app->calibration_frac && drand() < $app->calibration_frac) {
            $query = "select * from DBNAME.bossa_job where app_id=$app->id and (select count(*) from DBNAME.bossa_job_inst where job_id=bossa_job.id and user_id=$user->id) = 0 and state=1 and calibration=1 order by $prio desc limit 1";
            $result = $db->do_query($query);
            if (!$result) return null;
            $job = mysql_fetch_object($result, 'BossaJob');
            if (!$job) return null;
            mysql_free_result($result);
        } else {
            if (isset($user->bossa->category)) {
                $prio = "priority_".$user->bossa->category;
            } else {
                $prio = "priority_0";
            }
            // skips jobs for which this user
            // has already been assigned an instance
            //
            $query = "select * from DBNAME.bossa_job where app_id=$app->id and (select count(*) from DBNAME.bossa_job_inst where job_id=bossa_job.id and user_id=$user->id) = 0 and state=1 and calibration=0 order by $prio desc limit 1";
            $result = $db->do_query($query);
            if (!$result) return null;
            $job = mysql_fetch_object($result, 'BossaJob');
            if (!$job) return null;
            mysql_free_result($result);
        }

        $now = time();
        $clause = "(create_time, app_id, job_id, user_id) values ($now, $app->id, $job->id, $user->id)";
        $id = BossaJobInst::insert($clause);
        return BossaJobInst::lookup_id($id);
    }

    // app-callable functions
    //
    function update_info($info) {
        $info = serialize($info);
        return $this->update("info='$info'");
    }
    function get_info() {
        return unserialize($this->info);
    }
}

class BossaBatch {
    function insert($clause) {
        $db = BossaDb::get();
        $ret = $db->insert('bossa_batch', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }
    static function enum($clause) {
        $db = BossaDb::get();
        return $db->enum('bossa_batch', 'BossaBatch', $clause);
    }
    static function lookup_id($id) {
        $db = BossaDb::get();
        return $db->lookup_id($id, 'bossa_batch', 'BossaBatch');
    }
}

?>
