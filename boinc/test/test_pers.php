#! /usr/local/bin/php
<?php

//This tests the persistent file transfers for download and upload. It interrupts them in the middle and makes sure that the filesize never decreases along interrupted transfers.

include_once("test.inc");
set_time_limit(10000000000);

$project = new Project;
$user = new User();
$host = new Host($user);
$app = new App("upper_case");
$app_version = new App_Version($app);

$project->add_user($user);
$project->add_app($app);
$project->add_app_version($app_version);
$project->install();      // must install projects before adding to hosts

$host->log_flags = "log_flags.xml";
$host->add_project($project);
$host->install();

echo "adding work\n";

$work = new Work($app);
$work->wu_template = "uc_wu";
$work->result_template = "uc_result";
$work->redundancy = 2;
array_push($work->input_files, "input");
$work->install($project);
$project->start_feeder();

//get the path for checking download
$source_dir = SRC_DIR;
$enc_url = strtr($project->master_url, "/", "_");
$enc_url = substr($enc_url,7,strlen($enc_url));
$path= "$host->host_dir/projects/$enc_url/upper_case";
print "\n the path for checking download is :".$path;

$pid = $host->run_asynch("-exit_when_idle -limit_transfer_rate 2048");
$client_pid = $host->get_new_client_pid(null);
assert($pid != -1);
$first = 0;
$file_size = 0;
//Check download

while(1)
{

  if(file_exists($path))
    {
      $temp = filesize($path);
      if($temp < $file_size)
	{
	  echo "\nfilesize dropped, problem downloading\n";
	  echo "temp is $temp, file_size is  $file_size\n";
      	  break;
	}

      else if($temp > $file_size)
	{
	  print "\n filesize increased, it is : ".$temp;
	  if(($temp  > 40000) && ($first ==0))
	    {
	      print "\n stopping and rerunning the client";
	      echo "\n now killing client_pid : $client_pid";
	      $host->kill($client_pid, null);
	      $host->run_asynch("-exit_when_idle -limit_transfer_rate 2048");
	      $client_pid = $host->get_new_client_pid($client_pid);
              echo "\nNow executing : $client_pid";
	      $first++;
	    }

	}

      $file_size = $temp;

      if($file_size == filesize("$source_dir/apps/upper_case"))
	{
	  echo "\n download test succeeded";
	  break;
	}

    }
}


$file_size = 0;
$path= "$project->project_dir/upload/uc_wu_0_0";
$first =0;
print "\nupload path is: ".$path;
echo "\n Now checking upload";


while(1)
{
  //  echo "\n checking upload";


  if(file_exists($path))
    {

      //  echo "\nfile exists is download";
      $temp = filesize($path);
      if($temp < $file_size)
	{
	  echo "\nfilesize dropped,  problem uploading\n";
	  echo "temp is $temp, file_size if $file_size\n";
	  break;
	}
      if($temp > $file_size)
	{
	  print "\n filesize increased, it is : ".$temp;
	  if(($temp  > 20000) && ($first ==0))
	    {
	      print "\n stopping and rerunning the client";
	      print "\nkilling $client_pid";
	      $host->kill($client_pid,null);
	      $host->run_asynch("-exit_when_idle -limit_transfer_rate 2048");
	      $client_pid = $host->get_new_client_pid($client_pid);
	      echo "\nnew client_pid is $client_pid";
	      $first++;
	    }

	}
      $file_size = $temp;
      if($file_size == filesize("$source_dir/test/uc_correct_output"))
	{
	  print "\n all of the files has been uploaded";
	  print "\n stopping and rerunning the client";
	  $host->kill($client_pid, null);
	  $host->run("-exit_when_idle");
	  break;
	}
    }
}


$project->stop();

$result->server_state = RESULT_SERVER_STATE_OVER;
$result->stderr_out = "APP: upper_case: starting, argc 1";
$result->exit_status = 0;
$project->check_results(2, $result);
$project->compare_file("uc_wu_0_0", "uc_correct_output");
$project->compare_file("uc_wu_1_0", "uc_correct_output");

?>
