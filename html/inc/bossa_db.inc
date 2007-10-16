<?php

class Bossa {
    function insert_app(&$app) {
        if (!$app->long_jobs) $app->long_jobs = 0;
        $now = time();
        $query = "insert into bossa_app (create_time, name, user_friendly_name, long_jobs, start_url, finish_url) values ($now, '$app->name', '$app->user_friendly_name', $app->long_jobs, '$app->start_url', '$app->finish_url')";
        echo $query;
        $result = mysql_query($query);
        if (!$result) return false;
        $app->id = mysql_insert_id();
        return true;
    }

    function app_lookup_name($name) {
        $result = mysql_query("select * from bossa_app where name='$name'");
        if (!$result) return null;
        $app = mysql_fetch_object($result);
        mysql_free_result($result);
        return $app;
    }

    function app_lookup_id($id) {
        $result = mysql_query("select * from bossa_app where id='$id'");
        if (!$result) return null;
        $app = mysql_fetch_object($result);
        mysql_free_result($result);
        return $app;
    }

    function insert_job(&$job) {
        $now = time();
        $query = "insert into bossa_job (create_time, name, app_id, info, batch, time_estimate, time_limit, more_needed, npending, nsuccess, nsuccess_needed) values ($now, '$job->name', $job->app_id, '$job->info', $job->batch, $job->time_estimate, $job->time_limit, 1, 0, 0, $job->nsuccess_needed)";
        $result = mysql_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $job->id = mysql_insert_id();
        return true;
    }

    function insert_job_inst(&$ji) {
        $now = time();
        $query = "insert into bossa_job_inst (create_time, job_id, user_id) values ($now, $ji->job_id, $ji->user_id)";
        $result = mysql_query($query);
        if (!$result) {
            echo "$query\n";
            return false;
        }
        $ji->id = mysql_insert_id();
        return true;
    }

    // Assign a job from the given app to the given user.
    // Returns the job instance or NULL.
    //
    function assign_job($app, $user) {
        // this query skips jobs for which this user
        // has already been assigned an instance
        //
        $query = "select bossa_job.* from bossa_job left join bossa_job_inst on bossa_job_inst.job_id = bossa_job.id where bossa_job.more_needed<>0 and bossa_job_inst.user_id is null limit 1";
        echo "$query\n";
        $result = mysql_query($query);
        if (!$result) return null;
        $job = mysql_fetch_object($result);
        mysql_free_result($result);

        echo "<hr>";
        print_r($job);
        echo "<hr>";
        $ji->user_id = $user->id;
        $ji->job_id = $job->id;

        if (!Bossa::insert_job_inst($ji)) {
            echo mysql_error();
            return null;
        }
        return $ji;
    }
}

?>
