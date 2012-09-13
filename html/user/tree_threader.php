<?php

// Handler for TreeThreader remote job submission.
//
// Assumptions:
// - there is a file "tree_threader_templates_files" in the project root
//   containing (one per line) the names of files containing
//   gzipped collections of template files
// - These files are in the download hierarchy.

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

$app_name = "treeThreader";
$log = fopen("/tmp/tt_job.log","a+");

function error($s) {
    echo "<error>\n<message>$s</message>\n</error>\n";
    exit;
}

function handle_submit($r, $user, $app) {
    global $app_name,$log;

    $timestamp = date("Y-m-d H:i",time());
	// read the list of template filenames
	//
	$files = file("../../tree_threader_template_files");
	if ($files === false) { 
        fwrite($log,"$timestamp\t template file tree_threader_template_files\n");
        error("no templates file");
        
    }
	$njobs = sizeof($files);
	$now = time();
    $batch_id = BoincBatch::insert(
		"(user_id, create_time, njobs, name, app_id, state) values ($user->id, $now, $njobs, 'tree_threader batch', $app->id, ".BATCH_STATE_IN_PROGRESS.")"
    );
    if (!$batch_id) {
        $log_msg = "$timestamp\tfailed to create batch for user $user->id\n";
        fwrite($log, $log_msg);
        die("couldn't create batch\n");
    } else {
        $log_msg = "$timestamp\tcreated batch $batch_id for user $user->id\n";
        fwrite($log, $log_msg);
    }

    // move the sequence file to the download hier
    //
    $config = simplexml_load_string(file_get_contents("../../config.xml"));
    $fanout = (int)$config->config->uldl_dir_fanout;
    $download_dir = trim((string)$config->config->download_dir);

    $seq_fname = "three_threader_seq_$batch_id";
    $seq_path = dir_hier_path($seq_fname, $download_dir, $fanout);
    $tmp_name = $_FILES['seq_file']['tmp_name'];
    $ret = rename($tmp_name, $seq_path);
    if ($ret === false) {
        error("couldn't rename sequence file");
    }

    $i = 1;
	foreach ($files as $file) {
        $file = trim($file);
        $wu_name = "ICT_".$batch_id."_$i";

		$cmd = "cd ../..; ./bin/create_work --appname $app_name --batch $batch_id --wu_name $wu_name --wu_template templates/ICT_in --result_template templates/ICT_out $seq_fname $file";
        fwrite($log, "$timestamp\t$cmd\n");
        system($cmd, $ret);
		if ($ret != 0) {
            fwrite($log, "can not creat job $wu_name\n");
			error("can't create job");
        }
        $i++;
	}
	echo "<tt_reply>\n<batch_id>$batch_id</batch_id>\n</tt_reply>\n";
}

// Enumerate all the successfully completed WUs for this batch.
// Each output file is a .zip that unzips into a directory ali/.
// Combine their output files into a zip file in /tmp,
// make a symbolic link to this from /download,
// and return the resulting URL
//
function handle_get_output($r, $batch) {
    global $log;
	$wus = BoincWorkUnit::enum("batch=$batch->id");
	$outdir = "/tmp/tree_threader_output_".$batch->id;
    @mkdir($outdir);
	foreach ($wus as $wu) {
		if (!$wu->canonical_resultid) continue;
		$result = BoincResult::lookup_id($wu->canonical_resultid);
		if (!$result) continue;
		$paths = get_outfile_paths($result);
		if (sizeof($paths) < 1) continue;

		// there's only one output file
		//
		$path = $paths[0];

		// unzip it into a directory in /tmp
		//
		$dir = "/tmp/$wu->name";
        @mkdir($dir);
		$cmd = "cd $dir; unzip -q $path";
		system($cmd, $ret);
		if ($ret != 0) {
			error("can't unzip output file");
		}
		$cmd = "cp $dir/Aln/* $outdir";
		system($cmd, $ret);
		if ($ret != 0) {
			error("can't copy output files");
		}

        system("rm -rf $dir");
	}

	$cmd = "zip -r -q $outdir $outdir";
	system($cmd, $ret);
	if ($ret != $ret) {
		error("can't zip output files");
	}
	$fname = "tree_threader_output_".$batch->id.".zip";
    $treeThreader_dir="treeThreaderResult";
    if(!is_dir("../../download/$treeThreader_dir"))mkdir("../../download/$treeThreader_dir");
	@symlink("/tmp/$fname", "../../download/$treeThreader_dir/$fname");
    system("rm -fr $outdir");
    $config = simplexml_load_string(file_get_contents("../../config.xml"));
    $download_url = trim((string)$config->config->download_url);
	echo "<tt_reply>\n<url>$download_url/$treeThreader_dir/$fname</url>\n</tt_reply>\n";
}

xml_header();

if (1) {
    $r = simplexml_load_string($_POST['request']);
} else {
    $x = file_get_contents("xml_req");
    $r = simplexml_load_string($x);
}

if (!$r) {
    error("can't parse request message");
}

// authenticate the user
//
$auth = (string)$r->auth;
$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) error("invalid authenticator");
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error("no submit access");
$app = BoincApp::lookup("name='$app_name'");
if (!$app) error("no tree_threader app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error("no submit access");
    }
}

switch ((string)$r->action) {
    case 'submit':
        handle_submit($r, $user, $app);
        break;
    case 'get_output':
		$batch_id = (int)$r->batch_id;
		$batch = BoincBatch::lookup_id($batch_id);
		if (!$batch) error("no such batch");
		if ($batch->user_id != $user->id) error("not owner of batch");
		handle_get_output($r, $batch);
		break;
    default: error("bad command");
}

?>
