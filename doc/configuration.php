<?php

// MORE WORK IS NEEDED ON THIS IMPORTANT FILE.  THE DEPRECATED <FOO/> ELEMENTS SHOULD BE CHANGED TO <FOO> 1 </FOO>.
// MANY OF THE TAGS BELOW ARE NOT EXPLAINED IN THE DOCUMENTATION, AND THE LIST HERE IS PROBABLY NOT COMPLETE.

require_once("docutil.php");


page_head("The project configuration file");
echo "<!-- \$Id$ -->\n";

echo"
A project is described by a configuration file
named <b>config.xml</b> in the project's directory.
A config.xml file looks like this:
<pre>",
htmlspecialchars("
<boinc>

  <config>
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
    <stripchart_cgi_url>    http://A/URL          </stripchart_cgi_url>
    <log_dir>               /path/to/directory    </log_dir>

    [ <disable_account_creation/>                                                ]
    [ <show_results/>                                                            ]
    [ <one_result_per_user_per_wu/>                                              ]
    [ <max_wus_to_send>                  N    </max_wus_to_send>                 ]
    [ <min_sendwork_interval>            N    </min_sendwork_interval>           ]
    [ <daily_result_quota>               N    </daily_result_quota>              ]
    [ <ignore_delay_bound/>                                                     ]
    [ <locality_scheduling/>                                                     ]
    [ <locality_scheduling_wait_period>  N    </locality_scheduling_wait_period> ]
    [ <min_core_client_version>          N    </min_core_client_version          ]
    [ <choose_download_url_by_timezone/>                                         ]
    [ <cache_md5_info/>                                                          ]
    [ <min_core_client_version_announced> N </min_core_client_version_announced> ]
    [ <min_core_client_upgrade_deadline>  N </min_core_client_upgrade_deadline>  ]


    <!-- optional; defaults as indicated: -->
    <project_dir>  ../      </project_dir>  <!-- relative to location of 'start' -->
    <bin_dir>      bin      </bin_dir>      <!-- relative to project_dir -->
    <cgi_bin_dir>  cgi-bin  </cgi_dir>      <!-- relative to project_dir -->
  </config>

  <daemons>
    <daemon>
      <cmd>          feeder -d 3   </cmd>
      [ <host>       hostname.ip   </host>     ]
      [ <disabled>   1             </disabled> ]
    </daemon>
    <daemon>
    ...
    </daemon>
  </daemons>

  <tasks>
    <task>
      <cmd>          get_load        </cmd>
      <output>       get_load.out    </output>
      <period>       5 min           </period>
      [ <host>       host.ip         </host>       ]
      [ <disabled>   1               </disabled>   ]
      [ <always_run> 1               </always_run> ]
    </task>
    <task>
      <cmd>      echo \"HI\" | mail root@example.com     </cmd>
      <output>   /dev/null                             </output>
      <period>   1 day                                 </period>
    </task>
    <task>
    ...
    </task>
  </tasks>

</boinc>
"),
"</pre>
";

echo "<b>The general project configuration elements are:</b>";
list_start();
list_item("host", "name of project's main host, as given by Python's socket.hostname().  Daemons and tasks run on this host by default.");
list_item("db_name", "Database name");
list_item("db_host", "Database host machine");
list_item("db_user", "Database user name");
list_item("db_passwd", "Database password");
list_item("shmem_key", "ID of scheduler shared memory.  Must be unique on host.");
list_item("download_url", "URL of data server for download");
list_item("download_dir", "absolute path of download directory");
list_item("download_dir_alt",
    "absolute path of old download directory
    (see <a href=hier_dir.php>Hierarchical upload/download directories</a>)"
);
list_item("upload_url", "URL of file upload handler");
list_item("uldl_dir_fanout", "fan-out factor of upload and download directories
    (see <a href=hier_dir.php>Hierarchical upload/download directories</a>)"
);
list_item("upload_dir", "absolute path of upload directory");
list_item("cgi_url", "URL of scheduling server");
list_item("stripchart_cgi_url", "URL of stripchart server");
list_item("log_dir", "Path to the directory where the assimilator, feeder, transitioner and
cgi output logs are stored.  This allows you to change the default log
directory path.  If set explicitly, you can also use the 'grep logs'
features on the administrative pages.  Note: enabling 'grep logs' with
very long log files can hang your server, since grepping GB files can
take a long time.  If you enable this feature, be sure to rotate the
logs so that they are not too big.");
list_end();

echo "
    <b>The following control features that you may or may not want
    available to users.</b>
";
list_start();
list_item("disable_account_creation",
    "If present, disallow account creation"
);
list_item("show_results",
    "Enable web site features that show results (per user, host, etc.)"
);
list_end();

echo "
    <b>The following control the way in which results are scheduled, sent,
    and assigned to users and hosts.</b>
";
list_start();
list_item("one_result_per_user_per_wu",
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
list_item("max_wus_to_send",
    "Maximum results sent per scheduler RPC. Helps prevent hosts with
    trouble from getting too many results and trashing them.  But you
    should set this large enough so that a host which is only connected to
    the net at intervals has enough work to keep it occupied in between
    connections."
);
list_item("min_sendwork_interval",
    "Minimum number of seconds to wait after sending results to a given
    host, before new results are sent to the same host.  Helps prevent
    hosts with download or application problems from trashing lots of
    results by returning lots of error results.  But don't set it to be so
    long that a host goes idle after completing its work, before getting
    new work."
);
list_item("daily_result_quota",
    "Maximum number of results sent to a given host in a 24-hour
    period. Helps prevent hosts with download or application problems from
    returning lots of error results.  Be sure to set it large enough that
    a host does not go idle in a 24-hour period, and can download enough
    work to keep it busy if disconnected from the net for a few days."
);
list_item("ignore_delay_bound",
    "By default, results are not sent to hosts too slow to complete them within delay bound.
    If this flag is set, this rule is not enforced."
);
list_item("locality_scheduling",
    "When possible, send work that uses the same files that the host
     already has. This is intended for projects which have large data
     files, where many different workunits use the same data file. In
     this case, to reduce download demands on the server, it may be
     advantageous to retain the data files on the hosts, and send
     them work for the files that they already have.
     See <a href=sched_locality.php>Scheduling Locality</a>."
);
list_item("locality_scheduling_wait_period",
    "This element only has an effect when used in conjunction with the
     previous locality scheduling element. It tells the scheduler to
     use 'trigger files' to inform the project that more work is
     needed for specific files. The period is the number of seconds
     which the scheduler will wait to see if the project can create
     additional work. Together with project-specific daemons or
     scripts this can be used for 'just-in-time' workunit
     creation. See <a href=sched_locality.php>Scheduling Locality</a>."
);
list_item("min_core_client_version",
    "If the scheduler gets a request from a client with
    a version number less than this,
    it returns an error message and doesn't do any other processing."
);
list_item("choose_download_url_by_timezone",
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
      'http://einstein.phys.uwm.edu/download/123/some_file_name'."
);
list_item("cache_md5_info",
     "When creating work, keep a record (in files called foo.md5) of the
      file length and md5 sum of data files and executables.  This can
      greatly reduce the time needed to create work, if (1) these files
      are re-used, and (2) there are many of these files, and (3) reading
      the files from disk is time-consuming."
);
list_item("min_core_client_version_announced",
     "Announce a new version of the BOINC core client, which in the future
      will be the minimum required version.  In conjunction with the next
      tag, you can warn users with version below this to upgrade by a
      specified deadline.  Example value: 419."
);
list_item("min_core_client_upgrade_deadline",
    "Use in conjunction with the previous tag.  The value given here is the
     Unix epoch returned by time(2) until which hosts can update their
     core client.  After this time, they may be shut out of the project.
     Before this time, they will receive messages warning them to upgrade."
);
list_end();

// THE INFORMATION BELOW NEEDS TO BE ORGANIZED AND PUT INTO TABLES OR SOME OTHER LESS CRAMPED FORM
echo "
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
A daemon or task can be turned off by adding the &lt;disabled> element.
As well, there may be some tasks you wish to run via cron regardless of
whether or not the project is enabled (for example, a script that logs the
current CPU load of the host machine). You can do so by adding the
&lt;always_run> element (&lt;disabled> takes precedence over &lt;always_run>).
";
page_tail();
?>
