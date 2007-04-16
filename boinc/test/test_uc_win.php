#! /usr/local/bin/php
<?php
    // set up a test for the windows client
    // set BOINC_PLATFORM to "windows_intelx86"

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);
    $platform->name = "windows_intelx86";
    $platform->user_friendly_name = "Windows";
    $app_version->platform = $platform;
    array_push($app_version->exec_names, "upper_case.exe");

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 2;
    array_push($work->input_files, "input");

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->add_platform($platform);
    $project->install();
    $project->install_feeder();
    $project->install_make_work($work,20,5);

    echo "adding work\n";
    $work->install($project);

    $project->start_servers();

    echo "Go run the client\n";
    /*echo "Hit any key to stop the server\n";

    $project->stop();

    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");
    */
?>
