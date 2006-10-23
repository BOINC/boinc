<?php

require_once("docutil.php");

page_head("Configuring MySQL for BOINC");
echo "
<h2>Introduction</h2>

The note discusses how MySQL may be configured for BOINC Projects.
BOINC-based projects have varying DB traffic characteristics
and this note relates to our experiences with SETI@home,
so it may not be entirely applicable to all projects.
SETI@home currently uses MySQL 4.0+
and we expect to upgrade to 4.1 shortly and 5.0 later.
Our project uses only a single instantiation of the MySQL code file
and this note does not discuss the operation of multiple instances of MySQL
on a single server.

<p> 

All MySQL products and documentation are available at http://www.mysql.com/.
Our experience has been of using MySQL with Sun Solaris and Linux OSes.
MySQL on MS Windows or Mac OS X may be somewhat different.


<h2>MySQL DB Engines (or Table Types)</h2>
<h3>General</h3>

The MySQL software comprises a number of DB engines.
For SETI@home DB only 2 are used, Innodb and MyISAM.
They have different features and are used according to
the performance requirements of the project.
One can use all of the different engines (or table types)
or just a single one in a MySQL DB,
just depending on the query activity against each table in the project
among other.

<p> 
MySQL software  is available in 32 bit and 64 bit binaries for downloading.
Using 32 bit MySQL requires that all RAM resources
that are assigned to the various DB engines,
must sum to no more than 2GB of RAM.
There is no such limitation with 64 bit MySQL and large amounts
of RAM help Innodb performance. 

<h3>MyISAM</h3>

The MyISAM engine requires the least amount of computer resources
can be used where there is a low DB activity requirement.
For example with query rates lower that 5/sec this table type may be adequate.
Also if one does not have a dedicated DB server this may be a good choice
for all the tables since it consumes much less computer resources.
It has the advantages of allowing long text indices against
tables which Innodb does not allow.

<p>
MyISAM creates an OS file for each table and one for all the
indices related to the specific table (and another for the table format info).

<p> 
On the other hand it tends to suffer from consistency glitches
so will occasionally trash indices and will need rebuilding.
In commercial banking environments it would not be a good idea
to keep account balances in this table type since there is
no guarantee that transactions even if completed and printed will
remain in the DB.
MyISAM updates its tables synchronously and uses memory locks to avoid data
collisions.
In SETI@home, MyISAM  is used for the forum tables and logging
that have relatively low query rates.

<h3>Innodb</h3>

The Innodb engine is used for most of the tables in SETI@home project.
It processes multiple simultaneous queries against its tables.
It is a versioning DB engine that holds an image of the table
at the start of a query and maintains it until that query is completed.
Other updates are allowed during queries and in general for short queries
there is no problem.
Innodb uses the Innodb log  to store changes to its tables until
it flushes these changes to the actual tables at syncpoints.
If for any reason there is a server event that causes a system failure,
Innodb will use this log to recover the Innodb tables to consistency.
There are a minimum of 2 transaction log files
with a total maximum size of 4GB.

<p> 
Innodb tables/indices are usually stored in large OS physical files
and the tables and indices are managed internally within these OS/Innodb files.
It is important that these files are located on high performance devices.
The transaction log files should be located on independent high performance
media (away from the Innodb files) for sustained high transaction rates.
At DB shutdown all modified buffers have to be flushed into the transaction
logs before MySQL goes away, so slow performance drives for
the transaction log could delay shutdown for over 30 minutes
when there are a large number of .modified buffers. to be flushed.

 
<h2>Physical Requirements</h2>
<h3>CPU</h3>

Assuming the need for more than 70,000 users and 250K hosts
with an average workunit turnaround of about 10 hours
then one should get an Opteron dual-core class CPU.
It is a 64-bit architecture and can access up to 32GB of RAM.
It is qualified to run Solaris, Linux and Windows XP 64-bit (?) .
There are 64-bit versions of MySQL for Linux and Solaris OSes.

<p>
This is by no means the only hardware that will work with BOINC/MySQL,
however SETI@home uses this type of hardware and serves over 350K user
and over 630K hosts.
If your requirements are smaller,
then many 32bit hardware and OSes may be perfectly adequate.

<h3>RAM</h3>

The RAM requirement is related to the number of active subscribers
who are expected to volunteer for the project and the number
of threads that will be connected to the MySQL server.
We recommend a minimum of 2GB dedicated to MySQL for about 20,000 .
30,000 volunteers growing to servers with much larger RAM sizes,
say 6GB for up to 450K volunteers.
This is also related to disk IO rates that are available for use
by the data and log files.
For example Innodb will store modified data in RAM until a syncpoint
at which time data is flushed to disk;
during this time update transactions are paused until the flush is completed.
If there is large RAM and slow disk IO,
the pause can last for several minutes.
A similar delay can be noted when attempting to shutdown the
project database when all the modified buffers must be flushed
to disk before MySQL will shutdown, this delay could be 30 minutes or more.

<h3>IO Subsystem</h3>

Assuming a high performance requirement of more than 200 DB queries/sec
there should be separate controllers for for the data and the log files.
In the case of Innodb log files it is very important that
they are on very reliable media for example mirrored (RAID 1) drives.
The tables and indices require wide band or high throughput disk configuration
such as RAID 10.

<p> 
Some consideration should be given to having online spare
disk drives since this will help to minimize down times in case of failures.

<h2>Normal Operations</h2>
<h3>General</h3>

For normal operations or production there are some considerations that
should be addressed to enable the project personnel to
provide reliable service.
For example there should be a reliable power supply with UPS protection
to avoid uncontrolled shutdowns.
The temperature of the hardware operations room should be regulated
to hardware specifications to avoid premature aging/failure
of hardware components.

<p>
And the MySQL software has to be set up to take advantage
of the hardware resources that are available.

<h3>Config File (my.cnf)</h3>

 The config file needs to be set up for production environment.
 MySQL has defaults for where it allocates the files that it needs;
 where they are placed depends on the OS on which it is running.
 For greater control, space management and performance the user should
 define where these files are assigned.
 For example the base data directory for MySQL tables etc
 in Linux is /var/lib/MySQL.
 For SETI@home we assigned this to directory to another data partition
 /mydisks/a/apps/mysql/data/,
 to ensure that there was enough space and performance.
 It made it easy to do physical backups without including
 additional files that were not related to the database. 

<p> 
Here are some other file directory assignments for the SETI@home environment:
<pre>
 

innodb_data_home_dir = /mydisks/a/apps/mysql/data/

innodb_data_file_path = ibdata1:16G;ibdata2:16G;ibdata3:16G; ibdata4:16G;ibdata5:16G; ibdata6:16G;ibdata7:16G;ibdata8:16G;ibdata9:16G;ibdata10:16G;ibdata11:16G;ibdata12:16G;

innodb_log_group_home_dir = /mydisks/a/apps/mysql/mysql_logs/innodb_logs/

innodb_log_arch_dir = /mydisks/a/apps/mysql/mysql_logs/innodb_logs/
</pre>
 

Example of a MySQL config file:
<pre>
 

[mysqld]

#datadir=/var/lib/mysql

#datadir=/home/mysql/data/

datadir=/mydisks/a/apps/mysql/data/

#log-bin      ##/// this comment line disables replication

log-slow-queries = /mydisks/a/apps/mysql/jocelyn_slow.log

server-id       =       13

socket=/tmp/mysql.sock

skip-locking

set-variable    = delay_key_write=all

set-variable    = key_buffer= 750M

set-variable    = max_allowed_packet=2M

set-variable    = table_cache=256

set-variable    = sort_buffer=2M

set-variable    = record_buffer=2M

set-variable    = myisam_sort_buffer_size=512M

set-variable    = query_cache_limit=2M

set-variable    = query_cache_size=16M

set-variable    = thread_cache=128

# Try number of CPU's*2 for thread_concurrency

set-variable    = thread_concurrency=8

set-variable    = max_connections=256

set-variable    = max_connect_errors=1000

 

## more changes for slave replicant

#master-host     = xxx.ssl.berkeley.edu

#master-user     = slavexxx11

#master-password = masterpwxxx11

#replicate-do-db        =       SETI_BOINC

#replicate-ignore-db    =       mysql

 

# Uncomment the following if you are using Innobase tables

innodb_data_home_dir = /mydisks/a/apps/mysql/data/

innodb_data_file_path = ibdata1:16G;ibdata2:16G;ibdata3:16G; ibdata4:16G;ibdata5:16G; ibdata6:16G;ibdata7:16G;ibdata8:16G;ibdata9:16G;ibdata10:16G;ibdata11:16G;ibdata12:16G;

innodb_log_group_home_dir = /mydisks/a/apps/mysql/mysql_logs/innodb_logs/

innodb_log_arch_dir = /mydisks/a/apps/mysql/mysql_logs/innodb_logs/

set-variable = innodb_mirrored_log_groups=1

set-variable = innodb_log_files_in_group=4

set-variable = innodb_log_file_size=1000M

set-variable = innodb_log_buffer_size=16M

set-variable = innodb_flush_method=O_DIRECT

set_variable = innodb_fast_shutdown=1

innodb_flush_log_at_trx_commit=0

innodb_log_archive=0

set-variable = innodb_buffer_pool_size=4584M

set-variable = innodb_additional_mem_pool_size=8M

set-variable = innodb_file_io_threads=64

set-variable = innodb_lock_wait_timeout=50

[mysql.server]

user=mysql

basedir=/mydisks/a/apps/mysql

 

[safe_mysqld]

err-log=/mydisks/a/apps/mysql/jocelyn.err

pid-file=/mydisks/a/apps/mysql/jocelyn.pid

</pre>


<h2>Monitoring</h2>

<h3>MYTOP</h3>

During normal operations it is useful to monitor the MySQL IO traffic,
memory usage and connection activity to various client applications.
Mytop application script give useful realtime status for the MySQL engine.
Here is a sample of the first lines of its output:
<pre>
 

MySQL on localhost (4.0.23-max-log)
 up 18+00:32:55 [10:50:21]

 Queries: 641.7M  qps:  432 Slow:   71.4k         Se/In/Up/De(%):    51/01/43/03

             qps now:  382 Slow qps: 0.0  Threads:  413 (   2/  28) 43/01/46/09

 Cache Hits: 58.2M Hits/s: 39.2 Hits now:  17.3  Ratio: 17.9% Ratio now: 10.6%

 Key Efficiency: 99.4%  Bps in/out:  1.7k/ 1.6k   Now in/out: 63.5k/338.1k

</pre> 

It shows the historic queries/sec is 432 qps and the current
sample was measured at 382 qps.
The query cache hit rate is 17.9% historically and
for the current sample period it is 10.6% and the cache
fulfillment rate is 39.2 qps. 

<p>
Useful Innodb information from Mytop is shown towards the end of the
display for Innodb.
The buffer pools information in given in number
of pages that are 16KB in size.  See example below:
<pre>
 

--------

FILE I/O

--------

I/O thread 0 state: waiting for i/o request (insert buffer thread)

I/O thread 1 state: waiting for i/o request (log thread)

I/O thread 2 state: waiting for i/o request (read thread)

I/O thread 3 state: waiting for i/o request (write thread)

Pending normal aio reads: 0, aio writes: 0,

 ibuf aio reads: 0, log i/o's: 0, sync i/o's: 0

Pending flushes (fsync) log: 0; buffer pool: 0

1470930 OS file reads, 543461 OS file writes, 53800 OS fsyncs

1 pending preads, 0 pending pwrites

228.88 reads/s, 21594 avg bytes/read, 185.98 writes/s, 13.50 fsyncs/s

-------------------------------------

INSERT BUFFER AND ADAPTIVE HASH INDEX

-------------------------------------

Ibuf for space 0: size 335, free list len 283, seg size 619,

219535 inserts, 211776 merged recs, 45660 merges

Hash table size 9097667, used cells 2711301, node heap has 4751 buffer(s)

1573.54 hash searches/s, 5752.12 non-hash searches/s

---

LOG

---

Log sequence number 557 540674217

Log flushed up to   557 540451369

Last checkpoint at  556 4020363027

0 pending log writes, 0 pending chkp writes

39114 log i/o's done, 0.70 log i/o's/second

----------------------

BUFFER POOL AND MEMORY

----------------------

Total memory allocated 5032392104; in additional pool allocated 8386560

Buffer pool size   280576

Free buffers       0

Database pages     275825

Modified db pages  186393

Pending reads 1

Pending writes: LRU 129, flush list 0, single page 0

Pages read 2143598, created 23058, written 694488

301.17 reads/s, 4.40 creates/s, 216.68 writes/s

Buffer pool hit rate 991 / 1000

--------------

ROW OPERATIONS

--------------

6 queries inside InnoDB, 0 queries in queue

Main thread process no. 12155, id 1147140464, state: sleeping

Number of rows inserted 9780, updated 1039701, deleted 60084, read 159846476

0.10 inserts/s, 374.56 updates/s, 63.69 deletes/s, 1116.99 reads/s

----------------------------

END OF INNODB MONITOR OUTPUT

============================

</pre> 

<h3>IOSTAT:</h3>

Iostat is the UNIX type utility that provides a display of
the IO statistics for peripherals on a server or workstation.
For continuous displays of extended information for all devices.
Iostat should be invoke as follows:
<pre>
Iostat .x .k 5    ( this will produce an updated display every 5 seconds for all devices and give data in KB)
</pre>
 

<h3>MySQLAdmin</h3>

This program is making changes and getting the status of various MySQL
parameters.
It is not interactive but can be made to repeat a given
function by using number repeat option.  For example
<pre>
mysqladmin  extended-status  10
</pre>

 This will show the status display and repeat the display every 10 seconds.
 Adding the .r option will give followup displays that show delta
 differences with the first display values.

</h2>Performance Tweaking</h2>
<h3>General</h3>

An often overlooked area of performance is the requirement
for reliable power and air conditioning.
Power failures can eliminate all the benefits accrued
by careful planning for hardware and software installations.
Experience is that unreliable power can lead to days
of recovery with data loss and subscriber discontent.
Similarly, insufficient cooling accelerates the aging of
hardware components and can cause data corruption and
downtime more frequently than the one would expect given the hardware specs.
<pre>
There are several parameters in my.cnf that can be adjusted (within limits)
for better throughput.
Then the distribution of MySQL files to specified disk subsystems,
allocation of RAM and   Config: my.cnf options for files, RAM, IO options
</pre>

 
<h3>MySQL Configuration</h3>

Multi threads, query caching

<h3>Files. Distribution</h3>

Innodb files, transaction log files, bin-log files, MyISAM data/index files

<h3>Slow Query Log</h3>

Turn on Slow Query log to monitor slow queries.

<h3>RAM Allocation</h3>

Innodb vs MyISAM

 

";
page_tail();o

?>
