<?php

require_once("docutil.php");
page_head("Periodic tasks");
echo "
<b>Periodic tasks</b> are programs that are run periodically.
They are executed by the <a href=tool_start.php>bin/start --cron</a> program,
which you should run from cron.
(To do this, run crontab and add a line of the form
<pre>
0,5,10,15,20,25,30,35,40,45,50,55 * * * * HOME/projects/PROJECT/bin/start --cron
</pre>

Periodic tasks are short-running, but in case they aren't,
the 'start' script detect when an instance is still running
and won't start another instance.

<p>
Your project's periodic tasks are described in its
<a href=configuration.php>config.xml</a> file,
with elements of the following form:
";

echo html_text("
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
");
list_start();
list_item(
    "cmd",
    "The command used to perform the task.
    Must be a program in the project's /bin directory.
    <p>
    You can run PHP scripts as periodic tasks.
    These scripts must be in the html/ops/ directory,
    and can be run with a command of the form
    <pre>run_in_ops scriptname</pre>
    The script should be executable, and should have the form
    <pre>
#! /usr/bin/env php
&lt;?php
...
?&gt;
"
);
list_item("host",
    "Specifies where the daemon shoulr run.
    The default is the project's main host,
    as specified in config.xml."
);
list_item(
    "period",
    "The interval between executions,
    expressed as a single number (in units of minutes)
    or a number followed by 'seconds', 'minutes',
    'hours', or 'days' (may be abbreviated to unique initial string)."
);
list_item(
    "output",
    "Specifies the output file to output;
    and by default it is COMMAND_BASE_NAME.out.
    Output files are in log_X/, where X is the host."
);
list_item(
    "disabled",
    "Ignore this entry"
);
list_item(
    "always_run",
    "Run this task regardless of whether or not the project is enabled
    (for example, a script that logs the
    current CPU load of the host machine)."
);
list_end();
echo "
A project newly created by <a href=make_project.php>make_project</a>
has no periodic tasks.
Here are some programs you might want to run as periodic tasks:
";

list_start();
list_item("db_dump",
    "Write statistics data to XML files for export.
    Details are <a href=db_dump.php>here</a>.
    Recommended period: 7 days."
);
list_item("update_profile_pages.php",
    "Generate HTML files with lists of links to user profiles,
    organized alphabetically and by country.
    Recommended period: a few days."
);
list_item("update_stats -update_teams -update_users -update_hosts",
    "Update the recent average credit fields of
    users, teams, and hosts.
    This is important if you use send personalized
    <a href=recruit.php>mass emails</a> or
    <a href=reminder_email.php>reminder emails</a>,
    or use recent credit to enable message-board posting.
    Recommended period: every few days."
);
list_item("update_uotd.php",
    "Select a new User of the Day.
    Period: 1 day."
);
list_item("update_forum_activities.php",
    "Recompute 'popularity' estimages for threads and posts
    in the Questions and Answers message boards.
    Recommended period: 1 hour."
);

list_end();
page_tail();
?>
