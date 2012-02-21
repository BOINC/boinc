<?php {
    // $Id$

    include_once("test.inc");

    class ProjectUC extends Project
    {
        var $work;
        var $host;
        var $user;

        function ProjectUC() {
            $this->Project();
            $this->add_core_and_version();
            $this->add_app_and_version("upper_case");

            $this->user = new User();
            $this->user->project_prefs = "<project_specific>\nfoobar\n</project_specific>\n";
            $this->user->global_prefs = "<venue name=\"home\">\n".
                "<work_buf_min_days>0</work_buf_min_days>\n".
                "<work_buf_max_days>2</work_buf_max_days>\n".
                "<disk_interval>1</disk_interval>\n".
                "<run_on_batteries/>\n".
                "<max_bytes_sec_down>400000</max_bytes_sec_down>\n".
                "</venue>\n";

            $this->add_user($this->user);
            $this->install();      // must install projects before adding to hosts
            $this->install_feeder();

            $this->host = new Host();
            $this->host->add_user($this->user, $this);
            $this->host->install();

            $this->work = new Work();
            $this->work->wu_template = "uc_wu";
            $this->work->result_template = "uc_result";
            $this->work->redundancy = 2;
            $this->work->delay_bound = 10;
            // Say that 1 WU takes 1 day on a ref comp
            $this->work->rsc_fpops = 86400*1e9/2;
            $this->work->rsc_iops = 86400*1e9/2;
            $this->work->rsc_disk = 10e8;
            array_push($this->work->input_files, "input");
            $this->work->install($this);
        }

        function start_servers_and_host()
        {
            $this->start_servers();
            $this->host->run("-exit_when_idle -skip_cpu_benchmarks");
        }

        function validate_all_and_stop()
        {
            $this->validate($this->work->redundancy);
            $result->server_state = RESULT_SERVER_STATE_OVER;
            $result->stderr_out = "APP: upper_case: starting, argc 1
APP: upper_case: argv[0] is upper_case
APP: upper_case ending, wrote ";
            $result->exit_status = 0;
            $this->check_results(2, $result);
            $this->compare_file("uc_wu_0_0", "uc_correct_output");
            $this->compare_file("uc_wu_1_0", "uc_correct_output");

            $this->assimilate();
            $this->file_delete();

            $this->check_server_deleted("download/input");
            $this->check_server_deleted("upload/uc_wu_0_0");
            $this->check_server_deleted("upload/uc_wu_1_0");
            $this->stop();
        }
    }
} ?>
