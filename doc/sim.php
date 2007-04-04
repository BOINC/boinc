<?php
require_once("docutil.php");

page_head("Simulator input format");

echo "
This document describes an input file format for BOINC client simulators.
The input consists of three files.

<h3>sim_projects.xml</h3>
This describes the set of attached projects.

".html_text("
<projects>
    <project>
        <project_name>proj 1</project_name>
        <resource_share>100</resource_share>
        <app>
            <latency_bound>100</latency_bound>
            <fpops>
                <mean>100000000</mean>
                <var>100000</var>
            </fpops>
            <working_set>100000000</working_set>
        </app>
        <available>
            <frac>.7</frac>
            <lambda>1000</lambda>
        </available>
    </project>
</projects>
")."

A project has one or more applications.
Each application has a given latency bound and working-set size.
The number of FP ops is a truncated normal distribution
with the given mean and variance.

<p>
The availability of the projects
(i.e. the periods when scheduler RPCs succeed)
is modeled with two parameters:
the duration of available periods are exponentially distributed
with the given mean,
and the unavailable periods are exponentially distributed
achieving the given available fraction.
<h3>sim_host.xml</h3>
This describes the host hardware and availability.
".html_text("
<host>
    <p_ncpus>x</p_ncpus>
    <p_fpops>x</p_fpops>
    <m_nbytes>x</m_nbytes>
    <available>
        <frac>.7</frac>
        <lambda>1000</lambda>
    </available>
    <idle>
        <frac>.7</frac>
        <lambda>1000</lambda>
    </idle>
</host>
")."
The available periods (i.e., when BOINC is running)
and the idle periods (i.e. when there is no user input)
are modeled as above.
<h3>sim_prefs.xml</h3>
Same format as the global_prefs.xml file.
";
page_tail();
?>
