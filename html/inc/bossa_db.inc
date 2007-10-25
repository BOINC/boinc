<?php

require_once("../inc/util.inc");

class BossaDb {
    public static $db_conn;
    public static $db_name;

    static function init_conn() {
        if ($db_conn) return;
        $config = get_config();
        $user = parse_config($config, "<bossa_db_user>");
        $pass = parse_config($config, "<bossa_db_passwd>");
        $host = parse_config($config, "<bossa_db_host>");
        if ($host == null) {
            $host = "localhost";
        }
        self::$db_conn = mysql_pconnect($host, $user, $pass);
        if (!self::$db_conn) {
            error_page("can't connect to database");
        }
        self::$db_name = parse_config($config, "<bossa_db_name>");
    }

    static function lookup_id($id, $table, $classname) {
        self::init_conn();
        $query = "select * from ".self::$db_name.".$table where id=$id";
        $result = mysql_query($query , self::$db_conn);
        if (!$result) {
            echo $query;
            print_r($db_conn);
            echo mysql_error();
            return null;
        }
        $obj = mysql_fetch_object($result, $classname);
        mysql_free_result($result);
        return $obj;
    }

    static function enum($table, $classname, $clause=null) {
        self::init_conn();
        $x = array();
        if ($clause) {
            $result = mysql_query("select * from $db_name.$table where $clause", $db_conn);
        } else {
            $result = mysql_query("select * from $db_name.$table", $db_conn);
        }
        if (!$result) return null;
        while ($obj = mysql_fetch_object($result, $classname)) {
            $x[] = $obj;
        }
        mysql_free_result($result);
        return $x;
    }
    function update($table, $clause) {
        self::init_conn();
        return mysql_query(
            "update ".parent::$db_name.".$table set $clause where id=$this->id", parent::$db_conn
        );
    }
}

class BossaApp extends BossaDb {
    function insert() {
        parent::init_conn();
        if (!$this->long_jobs) $this->long_jobs = 0;
        $now = time();
        $query = "insert into ".parent::$db_name.".bossa_app (create_time, name, user_friendly_name, long_jobs, start_url) values ($now, '$this->name', '$this->user_friendly_name', $this->long_jobs, '$this->start_url')";
        $result = mysql_query($query, parent::$db_conn);
        if (!$result) return false;
        $this->id = mysql_insert_id();
        return true;
    }

    static function lookup_name($name) {
        parent::init_conn();
        $result = mysql_query("select * from ".parent::$db_name.".bossa_app where name='$name'", parent::$db_conn);
        if (!$result) return null;
        $app = mysql_fetch_object($result, 'BossaApp');
        mysql_free_result($result);
        return $app;
    }

    static function lookup_id($id) {
        return parent::lookup_id($id, 'bossa_app', 'BossaApp');
    }

    static function enum() {
        return parent::enum('bossa_app', 'BossaApp');
    }
}

class BossaJob extends BossaDb {
    function insert() {
        parent::init_conn();
        $now = time();
        $query = "insert into ".parent::$db_name.".bossa_job (create_time, name, app_id, info, batch, time_estimate, time_limit, more_needed, npending, nsuccess, nsuccess_needed) values ($now, '$this->name', $this->app_id, '$this->info', $this->batch, $this->time_estimate, $this->time_limit, 1, 0, 0, $this->nsuccess_needed)";
        $result = mysql_query($query, parent::$db_conn);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = mysql_insert_id();
        return true;
    }
    function update($clause) {
        return parent::update('bossa_job', $clause);
    }
    static function lookup_id($id) {
        return parent::lookup_id($id, 'bossa_job', 'BossaJob');
    }
    static function enum($clause) {
        return parent::enum('bossa_job', 'BossaJob', $clause);
    }
}

class BossaJobInst extends BossaDb {
    function insert() {
        parent::init_conn();
        $now = time();
        $query = "insert into ".parent::$db_name.".bossa_job_inst (create_time, job_id, user_id) values ($now, $this->job_id, $this->user_id)";
        $result = mysql_query($query, parent::$db_conn);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = mysql_insert_id();
        return true;
    }

    static function lookup_id($id) {
        return parent::lookup_id($id, 'bossa_job_inst', 'BossaJobInst');
    }
    static function enum($clause) {
        return parent::enum('bossa_job_inst', 'BossaJobInst', $clause);
    }

    function update($clause) {
        return parent::update('bossa_job_inst', $clause);
    }

    // Assign a job from the given app to the given user.
    // Returns the job instance or NULL.
    //
    static function assign($app, $user) {
        // this query skips jobs for which this user
        // has already been assigned an instance
        //
        parent::init_conn();
        $query = "select bossa_job.* from ".parent::$db_name.".bossa_job left join ".parent::$db_name.".bossa_job_inst on bossa_job_inst.job_id = bossa_job.id where bossa_job.more_needed<>0 and bossa_job_inst.user_id is null limit 1";
        $result = mysql_query($query, parent::$db_conn);
        if (!$result) return null;
        $job = mysql_fetch_object($result, 'BossaJob');
        if (!$job) return null;
        mysql_free_result($result);

        echo "<hr>";
        print_r($job);
        echo "<hr>";
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
        return BossaJob::update("npending=$job->npending, nsuccess=$job->nsuccess, more_needed=$job->more_needed");
    }

    // The given job instance has timed out.
    //
    function timed_out($job) {
        $job->npending--;
        $job->more_needed = ($job->npending+$job->nsuccess < $job->nsuccess_needed);
        return BossaJob::update("npending=$job->npending, more_needed=$job->more_needed");
    }
}

?>
