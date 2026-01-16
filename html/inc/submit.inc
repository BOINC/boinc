<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

// This file contains a PHP binding of a web-service interface
// for submitting jobs to a BOINC server.
//
// Functions:
//      boinc_abort_batch(): abort a batch
//      boinc_estimate_batch(); estimate completion time of a batch
//      boinc_get_output_file(): get the URL for an output file
//      boinc_get_output_files(): get the URL for zipped batch output
//      boinc_query_batch(): get details of a batch
//      boinc_query_batches(): get list of batches
//      boinc_query_job(): get details of a job
//      boinc_retire_batch(): retire a batch; delete output files
//      boinc_submit_batch(): submit a batch
//      boinc_set_timeout($x): set RPC timeout to X seconds
//
//  See https://github.com/BOINC/boinc/wiki/RemoteJobs#php-interface

//// Implementation stuff follows

// Convert a request message from PHP object to XML string
//
function req_to_xml($req, $op) {
    if (!isset($req->batch_name)) {
        $req->batch_name = "batch_".time();
    }
    $x = "<$op>
    <authenticator>$req->authenticator</authenticator>
    <batch>
    <app_name>$req->app_name</app_name>
    <batch_name>$req->batch_name</batch_name>
";
    if ((isset($req->output_template_filename)) && ($req->output_template_filename)) {
        $x .= "    <output_template_filename>$req->output_template_filename</output_template_filename>
";
    }
    if ((isset($req->input_template_filename)) && ($req->input_template_filename)) {
        $x .= "    <input_template_filename>$req->input_template_filename</input_template_filename>
";
    }
    if ((isset($req->app_version_num)) && ($req->app_version_num)) {
        $x .= "    <app_version_num>$req->app_version_num</app_version_num>
";
    }
    if (!empty($req->allocation_priority)) {
        $x .= "    <allocation_priority/>
";
    }
    if (isset($req->priority)) {
        $x .= "    <priority>$req->priority</priority>
";
    }
    foreach ($req->jobs as $job) {
        $x .= "    <job>
";
        if (!empty($job->name)) {
            $x .= "    <name>$job->name</name>
";
        }
        if (!empty($job->rsc_fpops_est)) {
            $x .= "    <rsc_fpops_est>$job->rsc_fpops_est</rsc_fpops_est>
";
        }
        if (!empty($job->command_line)) {
            $x .= "    <command_line>$job->command_line</command_line>
";
        }
        if (!empty($job->target_team)) {
            $x .= "        <target_team>$job->target_team</target_team>
";
        } elseif (!empty($job->target_user)) {
            $x .= "        <target_user>$job->target_user</target_user>
";
        } elseif (!empty($job->target_host)) {
            $x .= "        <target_host>$job->target_host</target_host>
";
        }
        if (isset($job->priority)) {
            $x .= "        <priority>$job->priority</priority>
";
        }
        foreach ($job->input_files as $file) {
            $x .= "        <input_file>\n";
            $x .= "            <mode>$file->mode</mode>\n";
            if ($file->mode == "remote") {
                $x .= "            <url>$file->url</url>\n";
                $x .= "            <nbytes>$file->nbytes</nbytes>\n";
                $x .= "            <md5>$file->md5</md5>\n";
            } else {
                $x .= "            <source>$file->source</source>\n";
            }
            $x .= "        </input_file>\n";
        }
        if (!empty($job->input_template)) {
            $x .= "    $job->input_template\n";
        }
        if (!empty($job->output_template)) {
            $x .= "    $job->output_template\n";
        }
        $x .= "    </job>
";
    }
    $x .= "    </batch>
</$op>
";
    return $x;
}

// check whether the PHP structure looks like a batch request object
//
function validate_request($req) {
    if (!is_object($req)) return "req is not an object";
    if (!property_exists($req, 'project')) return "missing req->project";
    if (!property_exists($req, 'authenticator')) return "missing req->authenticator";
    if (!property_exists($req, 'app_name')) return "missing req->app_name";
    if (!property_exists($req, 'jobs')) return "missing req->jobs";
    if (!is_array($req->jobs)) return "req->jobs is not an array";
    foreach ($req->jobs as $job) {
        // other checks
    }
    return null;
}

$rpc_timeout = 0;

// Given a request object and XML string, issue the HTTP POST request
//
function do_http_op($req, $xml, $op) {
    global $rpc_timeout;

    $ch = curl_init("$req->project/submit_rpc_handler.php");
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    if ($rpc_timeout) {
        curl_setopt($ch, CURLOPT_TIMEOUT, $rpc_timeout);
    }

// see if we need to send any files
//
$nfiles = 0;
$post = array();
$post["request"] = $xml;
$cwd = getcwd();
if ($op == "submit_batch") {
    foreach ($req->jobs as $job) {
        foreach ($job->input_files as $file) {
            if ($file->mode == "inline") {
                $path = realpath("$cwd/$file->path");
                $post["file$nfiles"] = $path;
                $nfiles++;
            }
        }
    }
}
curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    $reply = curl_exec($ch);
    curl_close($ch);
    if (!$reply) return array(null, "HTTP error");
    $r = @simplexml_load_string($reply);
    if (!$r) {
        return array(null, "Can't parse reply XML:\n$reply");
    }
    $e = (string)$r->error_msg;
    if ($e) {
        return array(null, "BOINC server: $e");
    } else {
        return array($r, null);
    }
}

// do a batch op (estimate or submit)
//
function do_batch_op($req, $op) {
    $retval = validate_request($req);
    if ($retval) return array(null, $retval);
    $xml = req_to_xml($req, $op);
    return do_http_op($req, $xml, $op);
}

// convert a batch description from XML string to object
//
function batch_xml_to_object($batch) {
    $b = new StdClass;
    $b->id = (int)($batch->id);
    $b->create_time = (double)($batch->create_time);
    $b->est_completion_time = (double)($batch->est_completion_time);
    $b->njobs = (int)($batch->njobs);
    $b->fraction_done = (double) $batch->fraction_done;
    $b->nerror_jobs = (int)($batch->nerror_jobs);
    $b->state = (int)($batch->state);
    $b->completion_time = (double)($batch->completion_time);
    $b->credit_estimate = (double)($batch->credit_estimate);
    $b->credit_canonical = (double)($batch->credit_canonical);
    $b->name = (string)($batch->name);
    $b->app_name = (string)($batch->app_name);
    $b->total_cpu_time = (double)($batch->total_cpu_time);
    return $b;
}

// if RPC had a fatal error, return the message
//
function get_error($reply, $outer_tag) {
    $name = $reply->getName();
    if ($name != $outer_tag) {
        return "Bad reply outer tag";
    }
    foreach ($reply->error as $error) {
        if (isset($error->error_num)) {
            return (string)$error->error_msg;
        }
    }
    return null;
}

//// API functions follow

function boinc_set_timeout($x) {
    global $rpc_timeout;
    $rpc_timeout = $x;
}

function boinc_ping($req) {
    $req_xml = "<ping>
    <authenticator>$req->authenticator</authenticator>
</ping>
";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return array(null, $errmsg);
    return array($reply, null);
}

function boinc_estimate_batch($req) {
    list($reply, $errmsg) = do_batch_op($req, "estimate_batch");
    if ($errmsg) return array(0, $errmsg);
    if ($x = get_error($reply, "estimate_batch")) {
        return array(null, $x);
    }
    return array((string)$reply->seconds, null);
}

function boinc_submit_batch($req) {
    list($reply, $errmsg) = do_batch_op($req, "submit_batch");
    if ($errmsg) return array(0, $errmsg);
    if ($x = get_error($reply, "submit_batch")) {
        return array(null, $x);
    }
    return array((int)$reply->batch_id, null);
}

function boinc_query_batches($req) {
    $req_xml = "<query_batches>
    <authenticator>$req->authenticator</authenticator>
";
    if (!empty($req->get_cpu_time)) {
        $req_xml .= "   <get_cpu_time>1</get_cpu_time>\n";
    }
    $req_xml .= "</query_batches>\n";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return array(null, $errmsg);
    if ($x = get_error($reply, "query_batches")) {
        return array(null, $x);
    }
    $batches = array();
    foreach ($reply->batch as $batch) {
        $b = batch_xml_to_object($batch);
        $batches[] = $b;
    }
    return array($batches, null);
}

function boinc_query_batch($req) {
    $req_xml = "<query_batch>
    <authenticator>$req->authenticator</authenticator>
    <batch_id>$req->batch_id</batch_id>
";
    if (!empty($req->get_cpu_time)) {
        $req_xml .= "   <get_cpu_time>1</get_cpu_time>\n";
    }
    if (!empty($req->get_job_details)) {
        $req_xml .= "   <get_job_details>1</get_job_details>\n";
    }
    $req_xml .= "</query_batch>\n";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return array(null, $errmsg);
    if ($x = get_error($reply, "query_batch")) {
        return array(null, $x);
    }
    $jobs = array();
    foreach ($reply->job as $job) {
        $j = new StdClass;
        $j->id = (int)($job->id);
        $j->canonical_instance_id = (int)($job->canonical_instance_id);
        $jobs[] = $j;
    }
    $r = batch_xml_to_object($reply);
    $r->jobs = $jobs;
    return array($r, null);
}

function boinc_query_job($req) {
    $req_xml = "<query_job>
    <authenticator>$req->authenticator</authenticator>
    <job_id>$req->job_id</job_id>
</query_job>
";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return array(null, $errmsg);
    if ($x = get_error($reply, "query_job")) {
        return array(null, $x);
    }
    $instances = array();
    foreach ($reply->instance as $instance) {
        $i = new StdClass;
        $i->name = (string)($instance->name);
        $i->id = (int)($instance->id);
        $i->state = (string)($instance->state);
        $i->outfiles = array();
        foreach ($instance->outfile as $outfile) {
            $f = new StdClass;
            $f->size = (double)$outfile->size;
            $i->outfiles[] = $f;
        }
        $instances[] = $i;
    }
    $r = new StdClass;
    $r->instances = $instances;
    return array($r, null);
}

function boinc_abort_batch($req) {
    $req_xml = "<abort_batch>
    <authenticator>$req->authenticator</authenticator>
    <batch_id>$req->batch_id</batch_id>
</abort_batch>
";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return $errmsg;
    if ($x = get_error($reply, "abort_batch")) {
        return array(null, $x);
    }
    return array(true, null);
}

function boinc_get_output_file($req) {
    $auth_str = md5($req->authenticator.$req->instance_name);
    $name = $req->instance_name;
    $file_num = $req->file_num;
    return $req->project."/get_output.php?cmd=result_file&result_name=$name&file_num=$file_num&auth_str=$auth_str";
}

function boinc_get_output_files($req) {
    $auth_str = md5($req->authenticator.$req->batch_id);
    $batch_id = $req->batch_id;
    return $req->project."/get_output.php?cmd=batch_files&batch_id=$batch_id&auth_str=$auth_str";
}

function boinc_retire_batch($req) {
    $req_xml = "<retire_batch>
    <authenticator>$req->authenticator</authenticator>
    <batch_id>$req->batch_id</batch_id>
</retire_batch>
";
    list($reply, $errmsg) = do_http_op($req, $req_xml, "");
    if ($errmsg) return $errmsg;
    if ($x = get_error($reply, "retire_batch")) {
        return array(null, $x);
    }
    return array(true, null);
}

//// example usage follows
if (0) {
    $req = new StdClass;
    $req->project = "http://isaac.ssl.berkeley.edu/test/";
    $req->authenticator = trim(file_get_contents("test_auth"));
    $req->app_name = "uppercase";
    $req->batch_name = "batch_name_12";
    $req->app_version_num = 710;
    $req->jobs = array();


    $f = new StdClass;
    $f->mode = "remote";
    $f->url = "http://isaac.ssl.berkeley.edu/validate_logic.txt";
    $f->md5 = "eec5a142cea5202c9ab2e4575a8aaaa7";
    $f->nbytes = 4250;

if (0) {
    $f = new StdClass;
    $f->mode = "local";
    $f->source = "foobar";
    //$job->input_files[] = $f;
}

    $it = "
<input_template>
    <file_info>
    </file_info>
    <workunit>
        <file_ref>
            <open_name>in</open_name>
        </file_ref>
        <target_nresults>1</target_nresults>
        <min_quorum>1</min_quorum>
        <rsc_fpops_est>   60e9  </rsc_fpops_est>
        <rsc_fpops_bound> 60e12 </rsc_fpops_bound>
        <rsc_disk_bound>2e6</rsc_disk_bound>
        <rsc_memory_bound>1e6</rsc_memory_bound>
        <delay_bound>3600</delay_bound>
        <credit>1</credit>
    </workunit>
</input_template>
";
    $ot = "
<output_template>
    <file_info>
        <name><OUTFILE_0/></name>
        <generated_locally/>
        <upload_when_present/>
        <max_nbytes>6000000</max_nbytes>
        <url><UPLOAD_URL/></url>
    </file_info>
    <result>
        <file_ref>
            <file_name><OUTFILE_0/></file_name>
            <open_name>out</open_name>
        </file_ref>
    </result>
</output_template>
";
    for ($i=0; $i<2; $i++) {
        $job = new StdClass;
        $job->input_files = array();
        $job->input_files[] = $f;
        $job->name = $req->batch_name."_$i";
        //$job->rsc_fpops_est = $i*1e9;
        $job->command_line = "--t $i";
        $job->input_template = $it;
        $job->output_template = $ot;
        $req->jobs[] = $job;
    }

    if (0) {
        list($e, $errmsg) = boinc_estimate_batch($req);
        if ($errmsg) {
            echo "Error from server: $errmsg\n";
        } else {
            echo "Batch completion estimate: $e seconds\n";
        }
    } else {
        list($id, $errmsg) = boinc_submit_batch($req);
        if ($errmsg) {
            echo "Error from server: $errmsg\n";
        } else {
            echo "Batch ID: $id\n";
        }
    }
}

if (0) {
    list($batches, $errmsg) = boinc_query_batches($req);
    if ($errmsg) {
        echo "Error: $errmsg\n";
    } else {
        print_r($batches);
    }
}

if (0) {
    $req->batch_id = 20;
    list($jobs, $errmsg) = boinc_query_batch($req);
    if ($errmsg) {
        echo "Error: $errmsg\n";
    } else {
        print_r($jobs);
    }
}
?>
