<?php
require_once("docutil.php");
page_head("Disk space management");
echo "

This document describes the core client's policies
for managing disk space.
The goals are the following (highest to lowest priority):
<ol>
<li><b>Enforce user disk usage preferences.</b>
<li><b>Enforce resource shares.</b>
<li><b>Provide disk space for completed results.</b>
<li><b>Provide disk space for active results.</b>
<li><b>Provide disk space for queued results.</b>
<li><b>Provide disk space for project file storage ('sticky files').</b>
</ol>

<p><b>Total_Limit; </b>disk usage limit as determined by prefs, disk size, and non-BOINC usage 
<p><b>Core_Usage; </b>space currently being used by core client 
<p><b>all_Projects_limit [aPl] = </b> total_limit - core_usage, this is space that projects can use [aPL] 
<p><b>Project_usage(p) = </b>The size of all files associated with a project p, returns project-&gt;size in most cases; 
<p><b>all_Project_usage[aPu] = </b>{Project_usage(p)} for all Project p 
<p><b>Project_limit(p) = </b>aPl*resource_share(p) 
<p>The <b>free space </b>on a client is determined by 
<blockquote>free_space = all_Projects_Limit - all_Project_usage</blockquote>
<p>A project is an <b>offender</b> if 
<blockquote> project_usage(p) - project_limit(p) &gt; 0</blockquote>
<p>The <b>greatest offender </b> is the project with  
<blockquote>max {project_usage(p) - project_limit(p))} for all p. </blockquote>
<p>The client will always try to remove files from the greatest offender first before querying other projects. 
<h3>A Project_limit(p) Example</h3>
<p>
Consider a system participating two projects, A and B, with resource shares 
75% and 25%, respectively. After computing the aPl available, A would receive 
75% of the aPL as its project limit and B would receive 25% of the aPL as its 
project limit.  
<p>
If P_u(A) &lt; P_l(A), Project A is not utilizing all Project_limit(A). Therefore, 
B should be able to use the difference (P_u(A) - P_l(A)) if it needs to. This 
applies to all projects in any situation where a project is not utilizing all 
its Project_limit(p). This unused space will show up as free_space in most cases. 
<p>
When A wants to add a new file, if adding the file would cause all_Project_usage 
&gt;= aPL, a project must delete a file first. If A is not an offender, then 
files are deleted from an offender, in this case, Project B. Files will be deleted 
from B as described in <em>Adding a File to a Project</em> below. 

<h3>What BOINC does in problem situations</h3>
<ul>
<li>
The current project has 
a disk share large enough for all the workunits needed to keep itself busy, 
but there is not enough disk space available for all the files because of 
other projects.
<blockquote>
When the BOINC client requests work, it tells the scheduling server how 
much free space it has on the client, as well as how much space it could potentially 
free. It is then the projects decision on how much work to award the client 
based on these values. When the files are downloaded, others are deleted as 
described below
</blockquote>
<li>
A large file was moved somewhere into the BOINC directory structure and begins to 
contribute to the space BOINC takes up. 
<blockquote>
Because of the way BOINC calculates the program size, this kind of error is unavoidable.
The client will notice that BOINC has violated its disk share and will do the necessary 
steps described above (see first priority in Maintaining Project Disk Share). The user will 
have to find the problematic file and remove it in order for BOINC to reclaim that space.
Deleted files will not be restored.
</blockquote>
<li>
A compution creates large temp files and stores them in its slot directory. These temp files 
are so large that they fill up the project disk size and the client no longer has 
enough space to run any computations .
<blockquote>
The client will try to create space for these temporary files as much it can, to the point 
where all files except those running a computation for a project are deleted. 
Because BOINC makes computation a priority over storage at this time, this 
is a very bad situation as it will delete all files in all other projects 
to ensure the work continues. If the total space for the BOINC client is 
still larger than the preferences allow, the client will suspend all activities 
and notify the user of the issue. The user can either drop the offending 
project or increase its disk size for BOINC in their preferences.
</blockquote>
</ul>

<h2>Adding a file to a project</h2>
<p>
The algorithm is run whenever:
<ul>
<li>A file is added to a project via an RPC reply
</ul>

<p>
The client will first attempt to use all the PDS that is free.
When all the space for BOINC projects is used 
by some combination of projects, files must be deleted to make space.
  
<p>
The client maintains the project share sizes and workunit queues by:
<ul>
<li> Try deleting files up to the point where a project is no longer an offender
<li>Starting deletion with the lowest priority files first 
    <ul>
      <li>
	    Being the greatest offender doesn't guarantee that it will be the first 
        to loose files, will delete lower priority files from other offenders 
        first.
    </ul>
  
  <li>Never deleting workunit 
    files associated with the requesting project, give up this point
  <li>Never delete results that have finished computation, let them be uploaded 
        and recorded properly.
</ul>

<p>
The client's method of creating free_space ensures the above: 
<ol>
  <li>If there enough free_space in the aPl, 
    <ul>
      <li>If yes, return <i>true</i> and <i>increase project-&gt;size by file-&gt;nbytes</i></li>
    </ul>
  </li>
  <li>Else check if there are any projects that are offending their project share, 
    <ul>
      <li>If yes, delete files starting with the greatest offender and the lowest 
        priority/expired files</li>
      <li>Delete these priority/expired files until the project size is below 
        the project's share size</li>
      <li>If there is not enough space made, increase the priority to delete and 
        repeat</li>
    </ul>
  </li>
  <li>Else check if there can be room made in the project by deleting any non-workunit 
    files, 
    <ul>
      <li>If yes, delete them.</li>
      <li>If there is now enough free space, return <i>true</i> and <i>increase 
        project-&gt;size by file-&gt;nbytes</i></li>
    </ul>
  </li>
  <li>Else, return false, there is no way to associate the file with the project 
    and guarantee the statements above</li>
</ol>

<h3>Pseudo Code</h3>
<pre>
PROJECT:
   double size
	double share_size
	double resource_share
FILE_INFO:
	double nbytes
	
get_more_disk_space():
   for some PROJECT p and space_needed = number of bytes required<br>	init total_space = 0
	
	total_space = free space in the project disk size
   if (total_space &gt; space_needed):
   	return true

	mark all projects as unchecked
	
	while(total_space &lt; space_needed):
		g_of = the greatest_offender
   	if(couldn't find one or g_of == p):
			increase priority to delete from
   		if can't increase anymore, return false   	
   	mark g_of as checked
		only delete low priority files up to the point when it isn't an offender
	return true


associate_file():
	for some FILE_INFO fip and a PROJECT p
   init space_made = 0	
   // check offenders
   if(get_more_disk_space(p, fip-&gt;size)):
   	p.size += fip.nbytes
  		return true
	// check self
   is there any free space?
   try and delete expendable files from p
   if(space_made &gt; fip-&gt;nbytes):
   	return true
	// if hasn't return true yet, failure
   return false</pre>
   
<h2>Violating User Disk Usage Preferences</h2>
<p>
This checking is done in the data_manager_poll() which is placed at the top 
of the client's FSM hierarchy. If there is no space violation, it takes no action 
and returns false. If BOINC is larger than the Total_Limit, the client will 
reduce Project_usages using the following method: 
<ol>
  <li>Check all the offending projects and delete files from them until they are 
    all not offenders or no more files can be deleted</li>
  <li>Cycle through each project, deleting one files at a time from each project, 
    starting with the lowest priority and expired files, until only referenced 
    files are left for the project.</li>
  <li>Cycle through each project, deleting one result at a time from each project 
    until there are only results that are waiting for thier 'server ack' or are 
    part of a current computation. This will removed references to files and hence 
    mark them available for deletion. </li>
</ol>
<p>
If all three conditions fail, all computation is suspended, a messsage is sent 
to the user explaining the problem and the function returns true. If at any 
time in the function, the total used space falls below the allowed disk usage 
set by the user preferences, the function returns false. 

<h2>Work Fetch Policy</h2>
<p> 
In conjunction with the CPU Scheduler's work fetch policy, the data manager's 
work fetch policy's goals are to: 
<ul>
  <li>Calculate total free space.</li>
  <li>Calculate total free space if lowest priority files were deleted. </li>
  <li>Calculate total free space if highest priority files were deleted.</li>
</ul>
<p>
When an RPC request is made, the client communicates to the server the values 
described above and the server can make a decision on how much work to send 
to the client. 
<h3>Calculating free space</h3>
<h3>Psuedocode</h3>
<pre>
anything_free():
	init total_size = 0
	foreach p in projects:
   	total_size += p.size
    get project disk size
	free space = project disk size - total_size

// get the total number of bytes that would be free
// if files with priority &lt; pr were deleted from all other projects
// and low priority files were deleted from this project
//

total_potentially_free():
	for some project my_p and some priority pr
	init tps = anything_free();

	ref_count all files in file_infos
    foreach p in projects
    if(p != my_p):
	    tps += potentially free space from p with priority less than pr


   foreach fip in file_infos
   if(fip.project == my_p, is permantent, and not part of a computation): 	
		and if(fip has lowest priority or is expired):
   		tps += fip-&gt;nbytes
 
potentially_free():
    for a project p and some priority pr
    if it is not an offender:
	    return 0;
	foreach fip in file_infos:
    if(fip.project == p, is permantent, and not part of a computation): 	
	    and if(fip.priority &lt;= pr or is expired):
   		tps += fip-&gt;nbytes   }
    return tps
</pre>

<h2>Project Deletion Policy</h2>
<p>
There are three types of deletion policy that a project can specify in its 
config.xml 
<ol>
  <li><b>Priority deletion. </b>Files with the lowest priority level get deleted 
    first in the order they were introduced to the client (downloaded). </li>
  <li><b>Expiration deletion. </b> Whenever space runs out, all files that have 
    past their expiration date are deleted first. Any file who's the expiration_date 
    is less than the time now is deleted.</li>
  <li><b>Least Recently Used (LRU). </b>The DEFAULT method, the last file to be 
    downloaded/uploaded is deleted first. The LRU policy is always used to determine 
    the next file to delete. </li>
</ol>
<p>
The policies are invoked by including the following in the config.xml file 
<blockquote>
&lt;deletion_policy_priority/&gt;<br>
  &lt;deletion_policy_expire/&gt;<br>
  - the LRU policy in inbedded in the core-client as the default
</blockquote>
<p>
If any of these flags are present in the config.xml, a similar tag will be 
included in a successful RPC request. 

<h3>Using a Project Deletion Policy</h3>
<p> 
A FILE_INFO, when created, has the following default values related to a projects 
deletion policy. These values are created for every file. 
<blockquote> 
  priority = P_LOW;<br>
  time_last_used = time now<br>
  exp_date = 60 days from now 
</blockquote>
<p>
where P_LOW is defined in client_types.h as the following 
<blockquote>
  #define P_LOW 1<br>
  #define P_MEDIUM 3<br>
  #define P_HIGH 5</blockquote>
<p>
If using the defualts, files will not be guarenteed to survive more than sixty 
days if &lt;deletion_policy_expire&gt; is true.  
<p>
If a priority or exp_date other than the default is required, the priority 
must be set when the workunit is created. By including the following tags in 
a workunit or result template, the default information is replaced.  
<blockquote> 
&lt;priority&gt;(int; 1-5)&lt;priority&gt;<br>
&lt;exp_days&gt;(int; # of days to keep)&lt;exp_days&gt; 
</blockquote>

<h2>Scheduling Server Changes</h2>
<p>The client communicates three values of disk usage to the server. 
<ol>
  <li>The amount of free_space</li>
  <li>The amount of free_space if the client were to delete files from offending 
    projects</li>
  <li>The amount of free_space if the client were to delete files from offending 
    projects &amp; itself</li>
</ol>
<p>
The server will assign workunits normally using the first amount. If no workunits 
were assigned, a second pass of the database is made using the second amount. 
If no workunits were assigned and the following is in the config.xml:
<blockquote>&lt;delete_from_self/&gt;</blockquote>
<p>
the third amound of free_space is used and a third pass of the database is 
made. Return whatever workunits were deemed acceptable for the host. 
<p>
Under most circumstances, the amount of free_space will be enough to get workunits 
for a project. If a project has larger workunits (&gt; 1 gb) or the host is 
storing many files for a project, amounts 2 &amp; 3 become more important. The 
amount of free_space if files are deleted is found by: 
<ul>
  <li>For offending projects, marking files that could be deleted up to where 
    the project is no longer an offender</li>
  <li>For the requesing project, marking all files that could be deleted 
    <ul>
      <li>Files that are not expired are never included if <i>deletion_policy_expire</i> 
        is true</li>
    </ul>
  </li>
</ul>
<h2>TODO: Future Changes</h2>
<p>
There is currently a method for requesting a list of files from the project. 
There needs to be a way to communicate the information back to the project, 
such as an xml doc that can be parsed by the project.</p>
<p>
There also needs to be a database, separate from the scheduling database, which 
keeps track of the files on host's clients. </p>

";
page_tail();
?>

