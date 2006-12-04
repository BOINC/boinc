<?php

require_once("docutil.php");

page_head("Getting started");

echo "
<h2>How it works</h2>
It's easy to participate in a BOINC project:
<a href=download.php>download</a> and install BOINC.</b>
You will be asked to enter the project's URL, your email address,
and a password.
That's it!
<p>
When you run BOINC on your PC, it does the following:
<br>
<center>
<img hspace=10 vspace=8 src=http://boinc.berkeley.edu/comm_simple.png>
</center>
<br>
<ol>
<li> Your PC gets a set of instructions from the project's
<b>scheduling server</b>.
The instructions depend on your PC: for example, 
the server won't give it work that requires more RAM than you have.
The instructions may include many multiple pieces of work.
Projects can support several <b>applications</b>,
and the server may send you work from any of them.
<li>
Your PC downloads executable and input files
from the project's <b>data server</b>.
If the project releases new versions of its applications,
the executable files are downloaded automatically to your PC.
<li> Your PC runs the application programs, producing output files.
<li> Your PC uploads the output files to the data server.
<li>
Later (up to several days later, depending on your
<a href=prefs.php>preferences</a>)
your PC reports the completed results to the scheduling server,
and gets instructions for more work.
</ol>
This cycle is repeated indefinitely.
BOINC does this all automatically; you don't have to do anything.

<a name=credit></a>
<h2>Credit</h2>
The project's server keeps track of how much work
your computer has done; this is called <b>credit</b>.
To ensure that credit is granted fairly,
most BOINC projects work as follows:
<ul>
<li> Each work unit may be sent to several computers.
<li> When a computer reports a result,
    it claims a certain amount of credit,
    based on how much CPU time was used.
<li> When at least two results have been returned,
    the server compares them.
    If the results agree, then users are granted
    the smaller of the claimed credits.
</ul>
<br>
<center>
<img src=http://boinc.berkeley.edu/credit.png>
</center>
<br>

Please keep in mind:
<ul>
<li> 
There may be a delay of several days between
when your computer reports a result
and when it is granted credit for the result.
Your User page shows you how much credit is 'pending'
(claimed but not granted).
<li>
The credit-granting process starts when your computer reports
a result to the server
(not when it finishes computing the result
or uploading the output files).
<li>
In rare cases (e.g. if errors occur on one or more computers)
you may never receive credit for a computation.
</ul>

<a name=software></a>
<h2>How the software works</h3>
<p>
The BOINC client software looks and acts like a single program,
but it's actually made up of several separate programs :
<br>
<center>
<img src=client.png>
</center>
<br>
<ul>
<li> The <b>core client</b> (named boinc.exe on Windows) communicates
with external servers to get and report work.
It runs and controls applications.
<li> <b>Applications</b> are the programs that do scientific computing.
Several of them may run at the same time on a computer
with more than one CPU.
<li> The BOINC Manager, or <b>GUI</b>, (named boincmgr.exe on Windows)
provides a graphical interface that lets you control the core client -
for example, by telling it to suspend and resume applications.
The GUI communicates with the core client by a TCP connection.
Normally this is a local connection;
however, it's possible to control a core client remotely.
<li> The <b>screensaver</b> runs when you're away from the computer.
It communicates with the core client by local TCP,
instructing it to tell one of the applications to generate
screensaver graphics.
</ul>
";
page_tail();

?>
