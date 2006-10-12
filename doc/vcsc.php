<?php

require_once("docutil.php");
page_head("Create a Virtual Campus Supercomputing Center (VCSC)");
echo "
Universities and other research institutions can use BOINC to create a
'Virtual Campus Supercomputing Center' (VCSC).
A VSCS can provide university researchers with the computational
power of a very large cluster, for a small fraction of the cost.

<p>
A VCSC is a BOINC project whose applications
are supplied by campus researchers.
The computing power is supplied by campus PCs -
computing lab machines,
desktop and laptops belonging to faculty, staff, and students,
and home PCs belonging to alumni.
<p>

As an example, suppose that a VSCS recruits the participation of 10,000 PCs,
running an average of 50% of the time.
In terms of computing power,
this is roughly equivalent to a 5,000-node cluster,
for which the initial hardware cost is roughly $5 million,
with ongoing yearly energy and maintenance costs of at least $1 million.
The VSCS, in contrast, has hardware costs of about $10K.

<h2>Creating a VCSC</h2>
<p>
A VCSC is typically created by a campus unit
involved in computing and information technology,
by a research unit, or as a collaboration between such units.
It involves the following functions,
which typically would be carried out by different staff:
<img align=right src=vcsc.png>
<ul>
<li> <b>Application identification and porting</b>:
This involves canvassing campus researchers,
identifying those with computationally-intensive problems
that map well to volunteer computing.
The applications used by those researchers are then ported to BOINC,
and mechanisms set up for the researchers to submit jobs and get results.
<li> <b>Server setup and maintenance</b>.
This involves setting up BOINC software,
and other required open-source software, on a Linux computer.
A low-end server (~$10K) is typically sufficient
even for large numbers (tens of thousands) of participating nodes.
<li> <b>Resource recruitment and client deployment</b>.
This involves several activities:
working with the campus IT department,
and with other units that manage campus desktops and PC labs,
to install BOINC on as many PCs as possible;
working with dormitory IT management to encourage
its deployment on student PCs;
creating PR events, such as competitions between
student groups to contribute the most computing power;
and working with alumni organizations to publicize the project
and encourage the participation of alumni home PCs.
This function also includes publicity,
via mass media, campus media, and Internet channels such
as periodic emails to volunteer participants.
</ul>
<br clear=all>

<h2>Benefits of a VCSC</h2>
The benefits of creating a VCSC include
<ul>
<li> It creates a new pool of computing power,
available at little or no cost to campus researchers.
This resource enables previously infeasible research,
and may attract prospective faculty.
<li> It creates a path by which computational scientists
can use distributed computing;
many of these scientists lack the expertise and resources
to do it themselves.
<li> It creates a new outlet for 'school spirit'
which directly contributes to the primary mission of the university.
<li> It generates PR for university research,
and for the university itself.
</ul>

<h2>Getting started</h2>
<p>
Technical information about porting applications
and setting up a BOINC server is <a href=create_project.php>here</a>.
Budget at least a few person-months for getting a working system running.
<p>
For more information about setting up a VCSC,
please <a href=contact.php>contact us</a>.

";
page_tail();
?>
