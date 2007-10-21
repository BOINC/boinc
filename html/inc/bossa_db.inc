<?php

function bossa_lookup($id, $table, $classname) {
    $result = mysql_query("select * from $table where id='$id'");
    if (!$result) return null;
    $obj = mysql_fetch_object($result, $classname);
    mysql_free_result($result);
    return $obj;
}

function bossa_enum($table, $classname, $clause=null) {
    $x = array();
    if ($clause) {
        $result = mysql_query("select * from $table where $clause");
    } else {
        $result = mysql_query("select * from $table");
    }
    if (!$result) return null;
    while ($obj = mysql_fetch_object($result, $classname)) {
        $x[] = $obj;
    }
    mysql_free_result($result);
    return $x;
}

class BossaApp {
    function insert() {
        if (!$this->long_jobs) $this->long_jobs = 0;
        $now = time();
        $query = "insert into bossa_app (create_time, name, user_friendly_name, long_jobs, start_url) values ($now, '$this->name', '$this->user_friendly_name', $this->long_jobs, '$this->start_url')";
        $result = mysql_query($query);
        if (!$result) return false;
        $this->id = mysql_insert_id();
        return true;
    }

    static function lookup_name($name) {
        $result = mysql_query("select * from bossa_app where name='$name'");
        if (!$result) return null;
        $app = mysql_fetch_object($result, 'BossaApp');
        mysql_free_result($result);
        return $app;
    }

    static function lookup_id($id) {
        return bossa_lookup($id, 'bossa_app', 'BossaApp');
    }

    static function enum() {
        return bossa_enum('bossa_app', 'BossaApp');
    }
}

class BossaJob {
    function insert() {
        $now = time();
        $query = "insert into bossa_job (create_time, name, app_id, info, batch, time_estimate, time_limit, more_needed, npending, nsuccess, nsuccess_needed) values ($now, '$this->name', $this->app_id, '$this->info', $this->batch, $this->time_estimate, $this->time_limit, 1, 0, 0, $this->nsuccess_needed)";
        $result = mysql_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = mysql_insert_id();
        return true;
    }
    function update($clause) {
        return mysql_query("update bossa_job set $clause where id=$this->id");
    }
    static function lookup_id($id) {
        return bossa_lookup($id, 'bossa_job', 'BossaJob');
    }
    static function enum($clause) {
        return bossa_enum('bossa_job', 'BossaJob', $clause);
    }
}

class BossaJobInst {
    function insert() {
        $now = time();
        $query = "insert into bossa_job_inst (create_time, job_id, user_id) values ($now, $this->job_id, $this->user_id)";
        $result = mysql_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $this->id = mysql_insert_id();
        return true;
    }

    static function lookup_id($id) {
        return bossa_lookup($id, 'bossa_job_inst', 'BossaJobInst');
    }
    static function enum($clause) {
        return bossa_enum('bossa_job_inst', 'BossaJobInst', $clause);
    }

    function update($clause) {
        return mysql_query("update bossa_job_inst set $clause where id=$this->id");
    }

    // Assign a job from the given app to the given user.
    // Returns the job instance or NULL.
    //
    static function assign($app, $user) {
        // this query skips jobs for which this user
        // has already been assigned an instance
        //
        $query = "select bossa_job.* from bossa_job left join bossa_job_inst on bossa_job_inst.job_id = bossa_job.id where bossa_job.more_needed<>0 and bossa_job_inst.user_id is null limit 1";
        $result = mysql_query($query);
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
