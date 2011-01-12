<?php
// make scenarios algorithmically

class PROJECT {
    public $name;
    public $resource_share;
    public $has_cpu;        // whether it has a CPU app
    public $cpu_ncpus;      // # CPUS used by CPU app
    public $cpu_flops;      // speed of CPU app
    public $cpu_job_size;   // size of jobs
    public $cpu_latency;    // latency of CPU jobs

    public $has_gpu;
    public $gpu_ncpus;
    public $gpu_flops;
    public $gpu_job_size;
    public $gpu_latency;
}

function write_project($p) {
    echo "
<project>
    <master_url>$p->name</master_url>
    <project_name>$p->name</project_name>
    <resource_share>$p->resource_share</resource_share>
</project>
<app>
    <name>$p->name</name>
    <user_friendly_name>$p->name</user_friendly_name>
</app>
";
    if ($p->has_cpu) {
        echo "
<app_version>
    <app_name>$p->name</app_name>
    <avg_ncpus>$p->cpu_ncpus</avg_ncpus>
    <flops>$p->cpu_flops</flops>
</app_version>
<workunit>
    <name>".$p->name."_cpu</name>
    <app_name>$p->name</app_name>
    <version_num>114</version_num>
    <rsc_fpops_est>$p->cpu_job_size</rsc_fpops_est>
    <rsc_fpops_bound>1e18</rsc_fpops_bound>
</workunit>
<result>
    <name>".$p->name."_cpu</name>
    <wu_name>".$p->name."_cpu</wu_name>
    <report_deadline>$p->cpu_latency</report_deadline>
    <received_time>0</received_time>
</result>
";
    }

    if ($p->has_gpu) {
        echo "
<app_version>
    <app_name>$p->name</app_name>
    <avg_ncpus>$p->gpu_ncpus</avg_ncpus>
    <plan_class>cuda</plan_class>
    <coproc>
        <type>CUDA</type>
        <count>1</count>
    </coproc>
    <flops>$p->gpu_flops</flops>
</app_version>
<workunit>
    <name>".$p->name."_gpu</name>
    <app_name>$p->name</app_name>
    <version_num>114</version_num>
    <rsc_fpops_est>$p->gpu_job_size</rsc_fpops_est>
    <rsc_fpops_bound>1e18</rsc_fpops_bound>
</workunit>
<result>
    <name>".$p->name."_gpu</name>
    <wu_name>".$p->name."_gpu</wu_name>
    <plan_class>cuda</plan_class>
    <report_deadline>$p->gpu_latency</report_deadline>
    <received_time>0</received_time>
</result>
";
    }
}

function write_state($pp) {
    echo "
<client_state>
<host_info>
    <p_ncpus>4</p_ncpus>
    <p_fpops>2e9</p_fpops>
    <coprocs>
        <coproc_cuda>
            <count>1</count>
            <peak_flops>100e9</peak_flops>
        </coproc_cuda>
    </coprocs>
</host_info>
<time_stats>
    <on_frac>1</on_frac>
    <connected_frac>1.000000</connected_frac>
    <active_frac>1</active_frac>
    <gpu_active_frac>1</gpu_active_frac>
</time_stats>
";
    foreach ($pp as $p) {
        write_project($p);
    }
    echo "
</client_state>
";
}

function big_scenario() {
    $pp = array();
    for ($i=0; $i<1; $i++) {
        $p = new PROJECT;
        $p->name = "C_$i";
        $p->resource_share = 100;
        $p->has_cpu = true;
        $p->cpu_ncpus = 1;
        $p->cpu_flops = 1e9;
        $p->cpu_job_size = 1000e9;
        $p->cpu_latency = 864000;
        $pp[] = $p;
    }
    for ($i=0; $i<1; $i++) {
        $p = new PROJECT;
        $p->name = "G_$i";
        $p->resource_share = 100;
        $p->has_gpu = true;
        $p->gpu_ncpus = 1;
        $p->gpu_flops = 10e9;
        $p->gpu_job_size = 10000e9;
        $p->gpu_latency = 864000;
        $pp[] = $p;
    }
    for ($i=0; $i<1; $i++) {
        $p = new PROJECT;
        $p->name = "CG_$i";
        $p->resource_share = 100;
        $p->has_gpu = true;
        $p->gpu_ncpus = 1;
        $p->gpu_flops = 1e9;
        $p->gpu_job_size = 10000e9;
        $p->gpu_latency = 864000;
        $p->has_cpu = true;
        $p->cpu_ncpus = 1;
        $p->cpu_flops = 1e9;
        $p->cpu_job_size = 10000e9;
        $p->cpu_latency = 864000;
        $pp[] = $p;
    }
    write_state($pp);
}

big_scenario();

?>
