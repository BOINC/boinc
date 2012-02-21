#!/usr/local/bin/php -q
<?php {
    // $Id$

    include_once("test.inc");

    test_msg("client checkpoint/restart mechanism");

    $project = new Project;
    $user = new User();
    $host = new Host();

    $project->add_user($user);
    $project->add_app_and_version("upper_case");
    $project->install();      // must install projects before adding to hosts

    $host->add_user($user, $project);
    $host->install();

    $work = new Work();
    $work->wu_template = "ucs_wu";
    $work->result_template = "uc_result";
    array_push($work->input_files, "small_input");
    $work->install($project);

    $project->install_feeder();
    $project->start_servers();

    verbose_echo(0, "Now run the client manually; start and stop it a few times");
    //compare_file("ucs_wu_0_0", "uc_small_correct_output");
} ?>
