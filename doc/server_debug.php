<?php

require_once("docutil.php");

page_head("Debugging server components");
echo "
<p>
A grab-bag of techniques for debugging BOINC server software:

<h2>Log files</h2>
Most error conditions are reported in the log files.
Make sure you know where these are.
If you're interested in the history of a particular WU or result,
grep for WU#12345 or RESULT#12345 (12345 represents the ID)
in the log files.
The html/ops pages also provide an interface for this.

<h2>Database query tracing</h2>
If you uncomment the symbol SHOW_QUERIES in db/db_base.C,
and recompile everything,
all database queries will be written to stderr
(for daemons, this goes to log files;
for command-line apps it's written to your terminal).
This is verbose but extremely useful for tracking down
database-level problems.

<h2>Scheduler single-stepping</h2>
The scheduler is a CGI program.
It reads from stdin and writes to stdout,
so you can also run it with a command-line debugger like gdb:

<ul>
<li>
Copy the \"scheduler_request_X.xml\" file from a client to the
   machine running the scheduler.  (X = your project URL)

<li>
Run the scheduler under the debugger, giving it this file as stdin,
   i.e.:
 
   <pre>
   gdb cgi
   (set a breakpoint)
   r &lt; scheduler_request_X.xml
   </pre>
                      
<li>
 You may have to doctor the database as follows:
 <pre>
   update host set rpc_seqno=0, rpc_time=0 where hostid=N
 </pre>
   to keep the scheduler from rejecting the request.
</ul>
This is useful for figuring out why your project is generating
'no work available' messages.

As an alternative to this, edit handle_request.C, and put
a call to debug_sched(sreq, sreply, \"../debug_sched\") just
before sreply.write(fout).
Then, after recompiling, touch a file called 'debug_sched' in
the project root directory.
This will cause transcripts of all subsequent scheduler requests and
replies to be written to the cgi-bin/ directory with separate
small files for each request.  The file names are sched_request_H_R
where H=hostid and R=rpc sequence number.
This verbose debugging output can be turned off by simply removing
the '../debug_sched()' call.


<h2>MySQL interfaces</h2>
You should become familiar with MySQL tools such as
<ul>
<li> mytop: like 'top' for MySQL
<li> the mysql interpreter ('mysql') and in particular
the 'show processlist;' query.
<li> MySQLadmin: general-purpose web interface to MySQL
</ul>

";

page_tail();
