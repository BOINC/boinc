<?php

require_once("docutil.php");

page_head("The project configuration file");

echo"
A project is described by a configuration file
named <b>config.xml</b> in the project's directory.
A config.xml file looks like this:
<pre>",
htmlspecialchars("
<boinc>
  <config>
    <host>main_host</host>     <!-- default host where tasks and daemons run -->
    <db_name>x</db_name>       <!-- database connection info -->
    <db_host>x</db_host>
    <db_passwd>x</db_passwd>
    <db_user>x</db_user>
    <shmem_key>x</shmem_key>   <!-- ID of scheduler shared memory -->
    <download_url>x</download_url>
    <download_dir>x</download_dir>   <!-- absolute path -->
    <upload_url>x</upload_url>
    <upload_dir>x</upload_dir>
    <cgi_url>x</cgi_url>
    <stripchart_cgi_url>x</stripchart_cgi_url>
    [ <one_result_per_user_per_wu/> ]
    [ <disable_account_creation/> ]
    [ <max_wus_to_send>10</max_wus_to_send ]
    <!-- optional; defaults as indicated: -->
    <project_dir>../</project_dir>     <!-- relative to location of 'start' -->
    <bin_dir>bin</bin_dir>             <!-- relative to project_dir -->
    <cgi_bin_dir>cgi-bin</cgi_dir>
    <log_dir>log</log_dir>
    <pid_dir>pid</pid_dir>
     ...
  </config>
  <daemons>
    <daemon>
      [ <host>foobar</host> ]
      [ <disabled/> ]
      <cmd>feeder -d 3</cmd>
    </daemon>
  </daemons>
  <tasks>
    <task>
      [ <host>foobar</host> ]
      [ <disabled/> ]
      <cmd>get_load</cmd>
      <output>get_load.out</output>
      <period>5 min</period>
    </task>
    <task>
      <cmd>echo \"HI\" | mail quarl</cmd>
      <output>/dev/null</output>
      <period>1 day</period>
    </task>
  </tasks>
</boinc>
"),
"</pre>
<b>Tasks</b> are periodic, short-running jobs.
&lt;cmd> and &lt;period> are required.
OUTPUT specifies the file to output and by default is COMMAND_BASE_NAME.out.
Commands are run in the &lt;bin_dir> directory
which is a path relative to &lt;project_dir> and output to &lt;log_dir>.

<p>
<b>Daemons</b> are continuously-running programs.
The process ID is recorded in the &lt;pid_dir> directory
and the process is sent a SIGHUP in a DISABLE operation.
<p>
Both tasks and daemons can run on a different host (specified by &lt;host>).
The default is the project's main host, which is specified in config.host
A daemon or task can be turned off by adding the &lt;disabled/> element.
";
page_tail();
?>
