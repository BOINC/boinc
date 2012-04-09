<?php

// script for submitting batches of LAMMPS jobs
//
// When a batch is submitted, this script runs the first job
// until 1 time step is completed;
// this verifies that the input files are OK and gives us
// an estimate of the FLOPS needed for the batch.
//
// These test executions are done in the directory
// project/lammps_test/USERID.
// We assume that lammps_test exists and contains the LAMMPS executable

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/util.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/sandbox.inc");
$debug=0;

// test a LAMMPS job
//
// the current directory must contain
//      structure_file
//      lammps_script
//      cmd_variables
//      pot_files (zipped potential files)
//
// output: success flag, CPU time per step, est. disk usage per job
//
function lammps_est() {
    $avg_cpu = 0;
    $test_result = 0;
    $descs = array();
    $pipes = array();
    $options = file("cmd_variables");
    $options[0]=chop($options[0],"\n");
    $cmd = "../lmp_linux ".$options[0]."&>output";
    if($GLOBALS["debug"]) echo $cmd."<br>";
    system("unzip pot_files >/dev/null");
    $stime=time();
    $p = proc_open("$cmd", $descs, $pipes);
    while (1) {
        $ctime=time();
        if($ctime-$stime >=2 and ! file_exists("log.1")){
           if($GLOBALS["debug"]) echo "time out "."<br>";
           proc_terminate($p);
           break;
        } 
        if (file_exists("log.1")) {
            $avg_cpu = calc_step_cpu("log.1");
            if ($avg_cpu != 0) {
                 if($GLOBALS["debug"])echo "avg_cpu is ".$avg_cpu."<br>";
                proc_terminate($p);
                $test_result = 1;
                break;
            }
        }
        //echo "sleeping\n";
        sleep(1);
    }
    $total_steps = get_total_steps("cmd_variables");
    $disk_space = calc_est_size(
        "lammps_script", "structure_file", "cmd_variables"
    );
    $total_cpu = $total_steps*$avg_cpu;
    return array($test_result, $total_cpu, $disk_space);
}

function get_total_steps($cmd_file) {
    $fd = fopen($cmd_file,"r");
    if (!$fd) {
        echo "can not open file $cmd_file\n";
        exit(-1);
    }
    $loopno = 1;
    $looprun = 1;
    while (!feof($fd)) {
        $line = fgets($fd,4096);
        if (preg_match("/loopnumber\s+\d+/", $line, $matches)
            && preg_match("/\d+/", $matches[0], $no)
        ) {
            $loopno=$no[0];
        }
        if (preg_match("/looprun\s+\d+/", $line, $matches)
            and preg_match("/\d+/", $matches[0], $no)
        ) {
            $looprun=$no[0];
        }
    }
    fclose($fd);
    $total_steps = $loopno*$looprun;
     if($GLOBALS["debug"])print "total_steps = ".$total_steps."<br>";
    return $total_steps;
}

function calc_step_cpu($filename) {
    $fd = fopen("$filename", "r");
    $start_line = "Step CPU ";
    $start = 0;
    $start_step = 1;
    $cur_step = 1;
    $avg_cpu = 0;
    if (!$fd) {
        echo "fail to open file log.1";
        exit(-1);
    }
    $count=0;
    while (!feof($fd)) {
        $line = fgets($fd,4096);
        if (preg_match('/^Step\s+CPU/',$line)) {
            //echo $line."\n";
            $start = 1;
            continue;
        }
        if (!$start) continue;
        $arr = preg_split("/\s+/", $line);
        //print_r($arr);

        if (count($arr) <=6 || !is_numeric($arr[1])) {
            continue;
        }
        $step = (int)$arr[1];
        $cpu = (float)$arr[2];
        //echo "step=".$step." cpu=".$cpu."\n";
        if ($cpu==0) {
           $count=0;
        } else {
            $count+=1;
            if($GLOBALS["debug"])echo "step=".$step." cpu=".$cpu."count=".$count."\n";
           if($count >= 10) {
                $avg_cpu=$cpu/$count;
                if($GLOBALS["debug"])echo "avg_cpu is ".$avg_cpu;
                break;
            }
        }
    }
    return $avg_cpu;
}

function calc_est_size($lammps_script, $structure_file, $cmd_file){
    $dump_types = 0;
    $fd = fopen($lammps_script,"r");
    if (!$fd){
        echo "can not open file $lammps_script\n";
        exit(-1);
    }
    while (!feof($fd)){
        $line = fgets($fd, 4096);
        if (preg_match("/^\s*dump/", $line)
            and preg_match_all("/dump\S+\.\w{3}/", $line, $matches, PREG_PATTERN_ORDER)
        ) {
            $dump_types=count($matches[0]);
            break;
        }
    }
    fclose($fd);
    if($GLOBALS["debug"])print "dump_types= ".$dump_types."<br>";

    $structure_file_size = filesize($structure_file);
    $fd = fopen($cmd_file,"r");
    if (!$fd){
        echo "can not open file $cmd_file\n";
        exit(-1);
    }
     if($GLOBALS["debug"]) print "structure_file_size=".$structure_file_size."<br>";
   
    $loopno=1;
    while (!feof($fd)){
        $line = fgets($fd,4096);
        if(preg_match("/loopnumber\s+\d+/", $line, $matches)){
            if(preg_match("/\d+/", $matches[0], $no)){
                $loopno=$no[0];
            }
        }
    }
    fclose($fd);
    if($GLOBALS["debug"])print "loopno=".$loopno."<br>";

    $est_size = $loopno*$structure_file_size*0.8*$dump_types;
    return $est_size;
}


function area_select() {
    return "
        <select name=area>
        <option value=\"Air filtration\">Air filtration</option>
        <option value=\"Water filtration\">Water filtration</option>
        <option value=\"Ultra-low friction\">Ultra-low friction</option>
        </select>
    ";
}

function show_submit_form($user) {
    page_head("Submit LAMMPS jobs");
    echo "
        <form action=lammps.php>
        <input type=hidden name=action value=prepare>
    ";
    start_table();
    row2("<strong>structure_file</strong><br><span class=note>structure_file*</span>", sandbox_file_select($user, "structure_file"));
    row2("<strong>lammps_script</strong><br><span class=note>lammps_script*</span>", sandbox_file_select($user, "lammps_script"));
    row2("<strong>cmdline_file</strong><br><span class=note>cmdline_file*</span><span class=note> ( List of command lines, one per job )</span>", sandbox_file_select($user, "cmdline_file"));
    row2("<strong>pot.zip</strong><br><span class=note>*.zip</span><span class-note> ( Zipped Potential files )</span>", sandbox_file_select($user, "zip"));
    row2("<strong>Area</strong>", area_select());
    row2("", "<input type=submit value=Prepare>");
    end_table();
    echo "</form>
        <p>
        <a href=sandbox.php><strong> File_Sandbox </strong></a>
        <a href=lammps.php><strong> Job_Submit </strong></a>
        <a href=submit.php><strong> Job_Control </strong></a>
    ";
    
    page_tail();
}

// verify that an input file exists in sandbox, and return its physical path
//
function get_file_path($user, $name) {
    $fname = get_str($name);

    // verify that the files exist in sandbox
    //
    $sbdir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$sbdir/$fname");
    if ($error) error_page("no $name file");

    return sandbox_physical_path($user, $md5);
}

function project_flops() {
    $x = BoincUser::sum("expavg_credit");
    if ($x == 0) $x = 200;
    $y = 1e9*$x/200;
    return $y;
}

// Estimate how long a batch will take.
// Let N = # jobs, M = # hosts
// If N < M we can start all the jobs more or less now,
// so let T = F/H, where H is the median FLOPS of the hosts.
// If N > M we'll have to do the jobs in stages,
// so let T = ceil(N/M)*F/H.
//
// Note: these are both extremely optimistic estimates
//
function estimated_makespan($njobs, $flops_per_job) {
    $nhosts = BoincHost::count("expavg_credit > 1");
    if ($nhosts < 10) {
        $median_flops = 2e9;
    } else {
        $n = $nhosts/2;
        $hs = BoincHost::enum("expavg_credit>1 order by p_fpops limit 1");
        $h = $hs[0];
        $median_flops = $h->p_fpops;
    }

    if ($njobs < $nhosts) {
        return $flops_per_job/$median_flops;
    } else {
        $k = (int)(($njobs+$nhosts-1)/$nhosts);
        return $k*$flops_per_job/$median_flops;
    }
}

function prepare_batch($user) {
    $structure_file_path = get_file_path($user, 'structure_file');
    $command_file_path = get_file_path($user, 'lammps_script');
    $cmdline_file_path = get_file_path($user, 'cmdline_file');
    $pot_files_path = get_file_path($user, 'zip');

    $info = null;
    $info->structure_file_path = $structure_file_path;
    $info->command_file_path = $command_file_path;
    $info->cmdline_file_path = $cmdline_file_path;
    $info->pot_files_path = $pot_files_path;
    $info->area = get_str("area");

    // get the directory in which to run the test,
    // clear it out,
    // and set up links to the input files
    //
    $test_dir = "../../lammps_test/$user->id";
    //echo "test_dir is ".$test_dir;
    if (!is_dir($test_dir)) {
        mkdir($test_dir);
    }
    if (!chdir($test_dir)) {
        error_page("Can't chdir");
    }
    system("rm *");
    symlink($structure_file_path, "structure_file");
    symlink($command_file_path, "lammps_script");
    symlink($cmdline_file_path, "cmd_variables");
    symlink($pot_files_path, "pot_files");
    list($error, $est_cpu_time, $disk) = lammps_est();
     if($GLOBALS["debug"])print "est_cpu_time is ".$est_cpu_time."<br>";
    if ($error==0) {
	$err_msgs=file("output");
	$err="Your test job <strong>failed</strong><br>Please refer to the following Error Message:<br><p>";
	foreach($err_msgs as $line){
	     $err=$err.$line."<br>";
	}
    $err=$err." <p>
                <a href=sandbox.php><strong> File_Sandbox </strong></a>
                <a href=lammps.php><strong> Job_Submit </strong></a>
                <a href=submit.php><strong> Job_Control </strong></a>
                ";
        error_page($err);
    }


    system("rm *");
    $info->rsc_fpops_est = $est_cpu_time * 1.5e9;
    $info->rsc_fpops_bound = $info->rsc_fpops_est * 20;
    
    if($disk==0){$info->rsc_disk_bound=1000000;}
    else{$info->rsc_disk_bound = $disk;}

    $tmpfile = tempnam("/tmp", "lammps_");
    file_put_contents($tmpfile, serialize($info));

    // get the # of jobs
    //
    $njobs = count(file($cmdline_file_path));
    $secs_est = estimated_makespan($njobs, $info->rsc_fpops_est);
    //assume the server's flops is 1.5G and the average client's flops is 1G
    $hrs_est = number_format($secs_est*1.5/60, 2);
    $client_mb = number_format($info->rsc_disk_bound/1e6, 1);
    $server_mb = number_format($njobs*$info->rsc_disk_bound/1e6, 1);

    page_head("Batch prepared");
    echo "
        Your batch has $njobs jobs.
        <p>
        Estimated time to completion: $hrs_est Minutes.
        <p>
        Estimated client disk usage: $client_mb MB
        <p>
        Estimated server disk usage: $server_mb MB
        <p>
    ";
    show_button("lammps.php?action=submit&tmpfile=$tmpfile", "Submit Batch");
    page_tail();
}

function submit_job($app, $batch_id, $info, $cmdline, $i) {
    $client_disk=$info->rsc_disk_bound*2;
    if($client_disk<50000000) $client_disk=50000000;
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id --rsc_fpops_est $info->rsc_fpops_est --rsc_fpops_bound $info->rsc_fpops_bound --rsc_disk_bound $client_disk";
    if ($cmdline) {
        $cmd .= " --command_line \"$cmdline\"";
    }
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    $cmd .= " ".basename($info->structure_file_path);
    $cmd .= " ".basename($info->command_file_path);
    $cmd .= " ".basename($info->pot_files_path);
    echo "<br> $cmd\n"; 

    $ret = system($cmd);
    //if ($ret === FALSE) {
    //    error_page("can't create job");
   // } else {
   //     header('Location: submit.php');
   // }
}

function submit_batch($user, $app) {
    $tmpfile = get_str('tmpfile');
    $x = file_get_contents("$tmpfile");
    $info = unserialize($x);

    $njobs=0;
    $cmdlines = file($info->cmdline_file_path);
    foreach ($cmdlines as $cmdline){
        if (preg_match("/^\s*-var/", $cmdline)) {
            $njobs++;
        }
    }
    
    $now = time();
    $batch_name = $info->area."_".date("Y-M-d D H:i:s");

    $batch_id = BoincBatch::insert(
        "(user_id, create_time, njobs, name, app_id, state) values ($user->id, $now, $njobs, '$batch_name', $app->id, ".BATCH_STATE_IN_PROGRESS.")"
    );
//    $batch_id=99;

    $i = 0;
    foreach ($cmdlines as $cmdline) {
        if (preg_match("/^\s*-var/", $cmdline)){
            submit_job($app, $batch_id, $info, $cmdline, $i);
            $i++;
        }
    }
}

$user = get_logged_in_user();
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error_page("no submit access");
$app = BoincApp::lookup("name='lammps'");
if (!$app) error_page("no lammps app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error_page("no submit access");
    }
}

$action = get_str('action', true);
switch ($action) {
case '': show_submit_form($user); break;
case 'prepare': prepare_batch($user); break;
case 'submit': submit_batch($user, $app); break;
default: error_page("no such action $action");
}
?>
