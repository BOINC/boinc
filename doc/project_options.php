<?php

require_once("docutil.php");
page_head("Project options [deprecated - wiki]");

function list_option($x, $y) {
    echo "<a name=$x></a>";
    list_item($x, $y);
}

echo "
The following elements in the &lt;config&gt; section
of your <a href=configuration.php>config.xml</a> file
control various aspects of your project.

<h3>Hosts, directories, and URLs</h3>
<p>
(These are created by make_project; normally you don't need to change them.)
";
echo html_text("
<master_url>            URL                     </master_url>
<long_name>             name                    </long_name>
<host>                  project.hostname.ip   </host>
<db_name>               databasename          </db_name>
<db_host>               database.host.ip      </db_host>
<db_user>               database_user_name    </db_user>
<db_passwd>             database_password     </db_passwd>
<shmem_key>             shared_memory_key     </shmem_key>
<download_url>          http://A/URL          </download_url>
<download_dir>          /path/to/directory    </download_dir>
<download_dir_alt>      /path/to/directory    </download_dir_alt>
<uldl_dir_fanout>       N                     </uldl_dir_fanout>
<upload_url>            http://A/URL          </upload_url>
<upload_dir>            /path/to/directory    </upload_dir>
<cgi_url>               http://A/URL          </cgi_url>
<!-- optional; defaults as indicated: -->
    <bin_dir>      bin      </bin_dir>      <!-- relative to project_dir -->
    <cgi_bin_dir>  cgi-bin  </cgi_dir>      <!-- relative to project_dir -->
[ <sched_lockfile_dir> path </sched_lockfile_dir> ]
");
list_start();
list_option("host",
    "name of project's main host, as given by Python's socket.hostname().
    Daemons and tasks run on this host by default."
);
list_option("db_name", "Database name");
list_option("db_host", "Database host machine");
list_option("db_user", "Database user name");
list_option("db_passwd", "Database password");
list_option("shmem_key", "ID of scheduler shared memory.  Must be unique on host.");
list_option("download_url", "URL of data server for download");
list_option("download_dir", "absolute path of download directory");
list_option("download_dir_alt",
    "absolute path of old download directory
    (see <a href=hier_dir.php>Hierarchical upload/download directories</a>)"
);
list_option("upload_url", "URL of file upload handler");
list_option("uldl_dir_fanout", "fan-out factor of upload and download directories
    (see <a href=hier_dir.php>Hierarchical upload/download directories</a>)"
);
list_option("upload_dir", "absolute path of upload directory");
list_option("cgi_url", "URL of scheduling server");
list_option("sched_lockfile_dir",
    "Enables scheduler locking (recommended) and specifies
    directory where scheduler lockfiles are stored.
    Must be writable to the Apache user.
");
list_end();
echo "
<h3>Web site features</h3>
";
echo html_text("
[ <profile_screening/> ]
[ <show_results/> ]
");
list_start();
list_option("profile_screening",
    "If present, don't show profile pictures until they've been
    screened and approved by project admins."
);
list_option("show_results",
    "Enable web site features that show results (per user, host, etc.)"
);
list_end();

echo "
<h3>Miscellaneous</h3>
";
echo html_text("
[ <disable_account_creation/> ]
[ <min_passwd_length> N </min_passwd_length> ]
");
list_start();
list_option("disable_account_creation",
    "If present, disallow account creation"
);
list_option("min_passwd_length",
    "Minimum length of user passwords.  Default is 6."
);
list_end();

echo "
<h3>Client control</h3>
";
echo html_text("
[ <verify_files_on_app_start/> ]
[ <symstore>URL</symstore> ]
[ <min_core_client_version_announced> N </min_core_client_version_announced> ]
[ <min_core_client_upgrade_deadline> N </min_core_client_upgrade_deadline> ]
[ <non_cpu_intensive> 0|1 </non_cpu_intensive> ]
");
list_start();
list_option("verify_files_on_app_start",
    "Before starting or restarting an app,
    check contents of input files and app version files
    by either MD5 or digital signature check.
    Detects user tampering with file
    (but doesn't really increase security,
    since user could also change MD5s or signatures in
    client state file)."
);
list_option("symstore",
    "URL of your project's symbol store,
    used for debugging Windows applications."
);
list_option("min_core_client_version_announced",
     "Announce a new version of the BOINC core client, which in the future
      will be the minimum required version.  In conjunction with the next
      tag, you can warn users with version below this to upgrade by a
      specified deadline.  Example value: 419."
);
list_option("min_core_client_upgrade_deadline",
    "Use in conjunction with the previous tag.  The value given here is the
     Unix epoch returned by time(2) until which hosts can update their
     core client.  After this time, they may be shut out of the project.
     Before this time, they will receive messages warning them to upgrade."
);
list_option("non_cpu_intensive",
    "If this flag is present,
    the project will be treated specially by the client:
    <ul>
    <li> The client will download one result at a time.
    <li> This result will be executed whenever computation is enabled
        (bypassing the normal scheduling mechanism).
    </ul>
    This is intended for
    <a href=non_cpu_intensive.php>applications that use little CPU time</a>,
    e.g. that do network or host measurements."
);
list_end();
echo "
<h3>Server logging</h3>
";
echo html_text("
[ <sched_debug_level> N </sched_debug_level> ]
[ <fuh_debug_level> N </fuh_debug_level> ]
");
list_start();
list_option("sched_debug_level",
    "Verbosity level for scheduler log output.
    1=minimal, 2=normal (default), 3=verbose."
);
list_option("fuh_debug_level",
    "Verbosity level for file upload handler log output.
    1=minimal, 2=normal (default), 3=verbose."
);

list_end();
echo "
<h3>Credit</h3>
(See also the command-line options of the <a href=validate.php>validator</a>).
";
echo html_text("
[ <fp_benchmark_weight> X </fp_benchmark_weight> ]
");
list_start();
list_option("fp_benchmark_weight",
    "The weighting given to the Whetstone benchmark
    in the calculation of claimed credit.
    Must be in [0 .. 1].
    Projects whose applications are floating-point intensive should use 1;
    pure integer applications, 0.
    Choosing an appropriate value will reduce the disparity
    in claimed credit between hosts.
    The script html/ops/credit_study.php,
    run against the database of a running project,
    will suggest what value to use."
);
list_end();
echo "
<h3>Scheduling options and parameters</h3>
";
echo html_text("
[ <one_result_per_user_per_wu/> ]
[ <max_wus_to_send> N </max_wus_to_send> ]
[ <min_sendwork_interval> N </min_sendwork_interval> ]
[ <daily_result_quota> N </daily_result_quota> ]
[ <ignore_delay_bound/> ]
[ <dont_generate_upload_certificates/> ]
[ <locality_scheduling/> ]
[ <locality_scheduling_wait_period> N </locality_scheduling_wait_period> ]
[ <min_core_client_version> N </min_core_client_version ]
[ <choose_download_url_by_timezone> 0|1 </choose_download_url_by_timezone> ]
[ <cache_md5_info> 0|1 </cache_md5_info> ]
[ <nowork_skip> 0|1 </nowork_skip> ]
[ <resend_lost_results> 0|1 </resend_lost_results> ]
[ <default_disk_max_used_gb> X </default_disk_max_used_gb> ]
[ <default_disk_max_used_pct> X </default_disk_max_used_pct> ]
[ <default_disk_min_free_gb> X </default_disk_min_used_pct> ]
[ <one_result_per_host_per_wu/> ]
[ <next_rpc_delay>x</next_rpc_delay> ]
");
list_start();
list_option("one_result_per_user_per_wu",
    "If present, send at most one result of a given workunit to a given user.
    This is useful for checking accuracy/validity of results.
    It ensures that the results for a given workunit are generated by
    <b>different</b> users.
    If you have a validator that compares different results
    for a given workunits to ensure that they are equivalent,
    you should probably enable this.
    Otherwise you may end up validating results from a given user
    with results from the <b>same</b> user."
);
list_option("max_wus_to_send",
    "Maximum results sent per scheduler RPC. Helps prevent hosts with
    trouble from getting too many results and trashing them.  But you
    should set this large enough so that a host which is only connected to
    the net at intervals has enough work to keep it occupied in between
    connections."
);
list_option("min_sendwork_interval",
    "Minimum number of seconds to wait after sending results to a given
    host, before new results are sent to the same host.  Helps prevent
    hosts with download or application problems from trashing lots of
    results by returning lots of error results.  But don't set it to be so
    long that a host goes idle after completing its work, before getting
    new work."
);
list_option("daily_result_quota",
    "Maximum number of results (per CPU) sent to a given host in a 24-hour
    period. Helps prevent hosts with download or application problems from
    returning lots of error results.  Be sure to set it large enough that
    a host does not go idle in a 24-hour period, and can download enough
    work to keep it busy if disconnected from the net for a few days. The
    maximum number of CPUS is bounded at four."
);
list_option("ignore_delay_bound",
    "By default, results are not sent to hosts too slow to complete them within delay bound.
    If this flag is set, this rule is not enforced."
);
list_option("dont_generate_upload_certificates",
    "Don't put upload certificates in results.
    This makes result generation a lot faster,
    since no encryption is done,
    but you lose protection against DoS attacks
    on your upload servers."
);
list_option("locality_scheduling",
    "When possible, send work that uses the same files that the host
     already has. This is intended for projects which have large data
     files, where many different workunits use the same data file. In
     this case, to reduce download demands on the server, it may be
     advantageous to retain the data files on the hosts, and send
     them work for the files that they already have.
     See <a href=sched_locality.php>Locality Scheduling</a>."
);
list_option("locality_scheduling_wait_period",
    "This element only has an effect when used in conjunction with the
     previous locality scheduling element. It tells the scheduler to
     use 'trigger files' to inform the project that more work is
     needed for specific files. The period is the number of seconds
     which the scheduler will wait to see if the project can create
     additional work. Together with project-specific daemons or
     scripts this can be used for 'just-in-time' workunit
     creation. See <a href=sched_locality.php>Locality Scheduling</a>."
);
list_option("min_core_client_version",
    "If the scheduler gets a request from a client with
    a version number less than this,
    it returns an error message and doesn't do any other processing."
);
list_option("choose_download_url_by_timezone",
     "When the scheduler sends work to hosts, it replaces the download
      URL appearing in the data and executable file descriptions with
      the download URL closest to the host's timezone.  The project
      must provide a two-column file called 'download_servers' in the
      project root directory.  This is a list of all download servers
      that will be inserted when work is sent to hosts.  The first column
      is an integer listing the server's offset in seconds from UTC.
      The second column is the server URL in the format such as
      http://einstein.phys.uwm.edu.  The download servers must
      have identical file hierarchies and contents, and the path to
      file and executables must start with '/download/...' as in 
      'http://X/download/123/some_file_name'."
);
list_option("cache_md5_info",
     "When creating work, keep a record (in files called foo.md5) of the
      file length and md5 sum of data files and executables.  This can
      greatly reduce the time needed to create work, if (1) these files
      are re-used, and (2) there are many of these files, and (3) reading
      the files from disk is time-consuming."
);
list_option("nowork_skip",
    "If the scheduling server has no work,
    it replies to RPCs without doing any database access
    (e.g., without looking up the user or host record).
    This reduces DB load, but it fails to update
    preferences when users click on Update.
    Use it if your server DB is overloaded."
);
list_option("resend_lost_results",
    "If set, and a &lt;other_results> list is present
    in scheduler request,
    resend any in-progress results not in the list.
    This is recommended;
    it may increase the efficiency of your project.
    For reasons that are not well understood,
    a BOINC client sometimes fails to receive the scheduler reply.
    This flag addresses that issue: it causes the SAME results to be resent
    by the scheduler, if the client has failed to receive them.
    "
);
list_option("send_result_abort",
    "If set, and the client is processing a result for a WU that has
    been cancelled or is not in the DB
    (i.e. there's no chance of getting credit)
    send &lt;result_abort>.
    If client is processing a result for a WU that has
    been assimilated or is overdue
    (i.e. there's a chance of not getting credit)
    send &lt;result_abort_if_not_started>.
");
list_option("default_disk_max_used_gb", "Sets the default value for
    the disk_max_used_gb preference so it's consistent between the
    scheduler and web pages.  The scheduler uses it when a request
    for work doesn't include preferences, or the preference is set
    to zero.  The web page scripts use it to set the initial value
    when displaying or editing preferences the first time, or when
    the user never saved them.  Default is 100.
");

list_option("default_disk_max_used_pct", "Sets the default value for
    the disk_max_used_pct preference so its consistent between the
    scheduler and web pages.  The scheduler uses it when a request
    for work doesn't include preferences, or the preference is set
    to zero.  The web page scripts use it to set the initial value
    when displaying or editing preferences the first time, or when
    the user never saved them. Default is 50.
");

list_option("default_disk_min_free_gb", "Sets the default value for
    the disk_min_free_gb preference so its consistent between the
    scheduler and web pages.  The scheduler uses it when a request
    for work doesn't include preferences.  The web page scripts use
    it to  set the initial value when displaying or editing 
    preferences the  first time, or when the user never saved them.
    Also, the scheduler uses this setting to override any smaller
    preference from the host, it enforces a 'minimum free disk space'
    to keep from filling up the drive.  Recommend setting this no
    smaller than .001 (1MB or 1,000,000 bytes).  Default is .001.
");
list_item("reliable_min_avg_credit<br> reliable_min_avg_turnaround",
    "Hosts for which
    expavg_credit/ncpus is at least <b>reliable_min_avg_credit</b>
    and whose average turnaround is at most
    <b>reliable_max_avg_turnaround</b> are considered 'reliable'.
");
list_item("reliable_time<br> reliable_reduced_delay_bound",
    "When the age of a workunit exceeds
    <b>reliable_time</b> (typically 2-3X the delay bound),
    send results only to reliable hosts,
    and multiply the delay bound by <b>reliable_reduced_delay_bound</b>
    (typically 0.5 or so).
");
list_item("reliable_on_priority <br> reliable_on_over <br> reliable_on_over_except_error",
    "Results with priority at least 'reliable_on_priority' will be sent
    only to reliable hosts;
    increase priority of duplicate results by 'reliable_on_over';
    increase priority of duplicates caused by timeout (not error)
    by 'reliable_on_over_except_error'.
    "
);
list_option("one_result_per_host_per_wu",
    "If present, send at most one result of a given workunit to a given host.
    This is weaker than one_result_per_user_per_wu;
    it is useful if you're using homogeneous redundancy and
    most of the hosts of a particular class belong to a single user."
);
list_option("next_rpc_delay",
    "Tell clients to do another scheduler RPC after X seconds."
);
list_end();

echo "
<h3>File deleter options</h3>
";
echo html_text("
[ <dont_delete_batches/> ]
");
list_start();
list_option("dont_delete_batches",
    "If this boolean is set,
    the file deleter won't delete any files for which
    the corresponding workunit or result record has
    a positive value of the the 'batch' field.
    This lets you keep files on disk until you're done with them.
    Create workunits with a positive batch number,
    and zero out (or negate) the batch number when you're done
    looking at the files
    (you can do this with a SQL query).
    If you use this option, replace the indices on
    file_delete_state with indices on (file_delete_state, batch)."
);
list_end();
echo "
<h3>Server status page options</h3>
";
echo html_text("
[ <www_host>hostname</www_host> ]
[ <sched_host>hostname</sched_host> ]
[ <sched_pid>path</sched_pid> ]
[ <uldl_host>hostname</uldl_host> ]
[ <uldl_pid>path</uldl_pid> ]
[ <ssh_exe>path</ssh_exe> ]
[ <ps_exe>path</ps_exe> ]
");
list_start();
list_option("www_host", "Host name of web server");
list_option("sched_host", "Host name of scheduling server");
list_option("sched_pid",
    "pid file of scheduling server (default: /etc/httpd/run/httpd.pid)"
);
list_option("uldl_host", "Host name of upload/download server");
list_option("sched_pid",
    "pid file of upload/download server (default: /etc/httpd/run/httpd.pid)"
);
list_option("ssh_exe", "path to ssh (default: /usr/bin/ssh)");
list_option("ps_exe", "path to ps (which supports \"w\" flag) (default: /bin/ps)");

list_end();

page_tail();
?>
