<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

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
    function insert($clause) {
        $db = BossaDb::get();
        return $db->insert('bossa_app', $clause);
    }

    static function lookup_short_name($name) {
        $db = BossaDb::get();
        return $db->lookup('bossa_app', 'BossaApp', "short_name='$name'");
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

class BossaJob {
    function insert() {
        $db = BossaDb::get();
        $now = time();
        $int_max = 2147483647;
        $query = "insert into DBNAME.bossa_job (create_time, name, app_id, info, batch, time_estimate, time_limit, conf_needed, transition_time) values ($now, '$this->name', $this->app_id, '$this->info', $this->batch, $this->time_estimate, $this->time_limit, $this->conf_needed, $int_max)";
        $result = $db->do_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = $db->insert_id();
        return true;
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
}

class BossaJobInst {
    function insert() {
        $db = BossaDb::get();
        $now = time();
        $query = "insert into DBNAME.bossa_job_inst (create_time, job_id, user_id, confidence) values ($now, $this->job_id, $this->user_id, $this->confidence)";
        $result = $db->do_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = $db->insert_id();
        return true;
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
        // this query skips jobs for which this user
        // has already been assigned an instance
        //
        // TODO: put the following in a transaction
        $db = BossaDb::get();
        $query = "select * from DBNAME.bossa_job where (select count(*) from DBNAME.bossa_job_inst where job_id=bossa_job.id and user_id=$user->id) = 0 limit 1";
        $result = $db->do_query($query);
        if (!$result) return null;
        $job = mysql_fetch_object($result, 'BossaJob');
        if (!$job) return null;
        mysql_free_result($result);

        $ji = new BossaJobInst();
        $ji->user_id = $user->id;
        $ji->job_id = $job->id;
        $ji->confidence = $user->conf;

        if (!$ji->insert()) {
            echo mysql_error();
            return null;
        }

        $job->update("conf_needed=conf_needed-$user->conf");
        return $ji;
    }

    // The given job instance has completed
    //
    function completed($job) {
        $this->finish_time = time();
        $this->update("finish_time=$this->finish_time, info='$this->info'");

        $now = time();
        return $job->update("transition_time=$now");
    }

    // The given job instance has timed out.
    //
    function timed_out($job) {
        $job->npending--;
        $job->more_needed = ($job->npending+$job->nsuccess < $job->nsuccess_needed);
        return $job->update("npending=$job->npending, more_needed=$job->more_needed");
    }
}

?>
