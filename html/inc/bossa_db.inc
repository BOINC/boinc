<?php

ini_set('display_errors', 'stdout');
error_reporting(E_ALL);

require_once("../inc/util.inc");

class DbConn {
    private $db_conn;
    private $db_name;

    function init_conn($user_tag, $passwd_tag, $host_tag, $name_tag) {
        $config = get_config();
        $user = parse_config($config, $user_tag);
        $pass = parse_config($config, $passwd_tag);
        $host = parse_config($config, $host_tag);
        if ($host == null) {
            $host = "localhost";
        }
        $this->db_conn = mysql_pconnect($host, $user, $pass);
        if (!$this->db_conn) {
            error_page("can't connect to database");
        }
        $this->db_name = parse_config($config, $name_tag);
    }

    function do_query($query) {
        $q = str_replace('DBNAME', $this->db_name, $query);
        //echo $q;
        return mysql_query($q, $this->db_conn);
    }

    function lookup($table, $classname, $clause) {
        $query = "select * from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        if (!$result) {
            echo $query;
            echo mysql_error();
            return null;
        }
        $obj = mysql_fetch_object($result, $classname);
        mysql_free_result($result);
        return $obj;
    }

    function lookup_id($id, $table, $classname) {
        return $this->lookup($table, $classname, "id=$id");
    }

    function enum($table, $classname, $clause=null) {
        $x = array();
        if ($clause) {
            $query = "select * from DBNAME.$table where $clause";
        } else {
            $query = "select * from DBNAME.$table";
        }
        $result = $this->do_query($query);
        if (!$result) return null;
        while ($obj = mysql_fetch_object($result, $classname)) {
            $x[] = $obj;
        }
        mysql_free_result($result);
        return $x;
    }
    function update($obj, $table, $clause) {
        $query = "update DBNAME.$table set $clause where id=$obj->id";
        return $this->do_query($query);
    }
    function insert_id() {
        return mysql_insert_id($this->db_conn);
    }
}

class BossaDb extends DbConn {
    public static $instance;

    static function get() {
        if (!isset($instance)) {
            $instance = new DbConn();
            $instance->init_conn(
                "<bossa_db_user>", "<bossa_db_passwd>",
                "<bossa_db_host>", "<bossa_db_name>"
            );
        }
        return $instance;
    }
}

class BossaApp {
    function insert() {
        $db = BossaDb::get();
        if (!isset($this->long_jobs)) $this->long_jobs = 0;
        $now = time();
        $query = "insert into DBNAME.bossa_app (create_time, name, user_friendly_name, long_jobs, start_url) values ($now, '$this->name', '$this->user_friendly_name', $this->long_jobs, '$this->start_url')";
        $result = $db->do_query($query);
        if (!$result) return false;
        $this->id = $db->insert_id();
        return true;
    }

    static function lookup_name($name) {
        $db = BossaDb::get();
        return $db->lookup('bossa_app', 'BossaApp', "name='$name'");
    }

    static function lookup_id($id) {
        $db = BossaDb::get();
        return $db->lookup_id($id, 'bossa_app', 'BossaApp');
    }

    static function enum() {
        $db = BossaDb::get();
        return $db->enum('bossa_app', 'BossaApp');
    }
}

class BossaJob {
    function insert() {
        $db = BossaDb::get();
        $now = time();
        $query = "insert into DBNAME.bossa_job (create_time, name, app_id, info, batch, time_estimate, time_limit, more_needed, npending, nsuccess, nsuccess_needed) values ($now, '$this->name', $this->app_id, '$this->info', $this->batch, $this->time_estimate, $this->time_limit, 1, 0, 0, $this->nsuccess_needed)";
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
        return $db->update('bossa_job', $clause);
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
        $query = "insert into DBNAME.bossa_job_inst (create_time, job_id, user_id) values ($now, $this->job_id, $this->user_id)";
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
        $db = BossaDb::get();
        $query = "select bossa_job.* from DBNAME.bossa_job left join DBNAME.bossa_job_inst on bossa_job_inst.job_id = bossa_job.id where bossa_job.more_needed<>0 and bossa_job_inst.user_id is null limit 1";
        $result = $db->do_query($query);
        if (!$result) return null;
        $job = mysql_fetch_object($result, 'BossaJob');
        if (!$job) return null;
        mysql_free_result($result);

        $ji = new BossaJobInst();
        $ji->user_id = $user->id;
        $ji->job_id = $job->id;

        if (!$ji->insert()) {
            echo mysql_error();
            return null;
        }
        return $ji;
    }

    // The given job instance has completed
    //
    function completed($job) {
        $this->finish_time = time();
        $this->update("finish_time=$this->finish_time, info='$this->info'");

        $job->npending--;
        $job->nsuccess++;
        $job->more_needed = ($job->npending+$job->nsuccess < $job->nsuccess_needed);
        return $job->update("npending=$job->npending, nsuccess=$job->nsuccess, more_needed=$job->more_needed");
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
