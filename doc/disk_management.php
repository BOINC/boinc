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
<li><b>Provide disk 
space for project file storage ('sticky files').</b>
</ol>

<h3>Project Disk Space (PDS)</h3>
<p>
BOINC uses a portion of the host's disk.
The size of this portion is determined by a user's preferences. 
This portion is divided into what the data manager sees as 'project disk space' 
and everything else.
The project disk space, or PDS, is all the files associated 
with projects, including their applications running in slot directories

<h3>Project Size</h3>
<p>
The actual total number 
of bytes of all the files in a project's directory and any associated applications 
running in slot directories.

<h3>Project Share</h3>
<p>
After the project disk space's 
size has been determined, the space is divided up among the projects.
Each project receives a 'project share' that is a percentage of the PDS.
This percentage 
is the resource percentage awarded to each project in the user's preferences. 
  
<p>
For example, consider a 
system participating two projects, A and B, with resource shares 75% and 25%, 
respectively.
After computing the PDS available to the projects, p_A would receive 
75% of the PDS as its project share and p_B would receive 25% of the PDS as 
its project share. 

<p>
If p_A is only using X% 
of its awarded share, p_B can use the extra space in p_A's share in order to 
grow larger.
Eventually all the space in the PDS could be filled.
p_A may only 
have Y% of the its share filled, while p_B has grown to fill its own share and 
the remainder of p_A's share.
When p_A wants to add a new file, it will ask 
p_B to delete files to make space. p_A can continue adding files and asking 
for space until p_B is only occupying 25% of the PDS, then the project shares 
will be balanced.
At this point, p_A and p_B will have to delete their own files 
to continue adding new files.
If for some reason space become available in p_A's 
share again, p_B is allowed to use that space.

<h3>Offender</h3>
<p>
An 'offending project' is 
a project who's size has grown larger than its share.
This situation is acceptable 
and encouraged until there is no more space in the PDS.
The <b>greatest offender 
</b> is the offender with the maximum number of bytes over their share size.At 
this time, the greatest offenders will be asked to remove some files to make 
space for projects that have not utilized their own project share. 

<p>
The client will always try 
and remove files from the greatest offender first before querying other projects 
for file deletion.

<h3>What BOINC does in problem situations</h3>
<ul>
<li>
The current project has 
a disk share large enough for all the workunits needed to keep itself busy, 
but there is not enough disk space available for all the files because of 
other projects.
<p>
When the BOINC client 
requests work, it tells the scheduling server how much free space it has on 
the client, as well as how much space could potentially free up.
It is then 
the projects decision on how much work to award the client based on these 
values.
When the files are downloaded, files from other projects that are 
taking up too much disk space are deleted to make way for the new workunits. 

<li>
A large file was moved 
somewhere into the BOINC directory structure and begins to contribute to the 
space BOINC takes up. 
<p>
Because of the way BOINC calculates the program size, this kind of error 
is unavoidable.
The client will notice that BOINC has violated its disk 
share and will do the necessary steps described above (see first priority 
in Maintaining Project Disk Share).
The user will have to find the problematic 
file and remove it in order for BOINC to reclaim that space.
Deleted files 
will not be restored but the space they had will be available to new files.

<li>
A compution creates large 
temp files and stores them in its slot directory.
These temp files are so 
large that they fill up the project disk size and the client no longer has 
enough space to run any computations .
<p>
The client will 
try to create space for these temporary files as much it can, to the point 
where all files except those running a computation for a project are deleted. 
Because BOINC makes computation a priority over storage at this time, this 
is a very bad situation as it will delete all files in all other projects 
to ensure the work continues.
If the total space for the BOINC client is 
still larger than the preferences allow, the client will suspend all activities 
and notify the user of the issue. The user can either drop the offending 
project or increase its disk size for BOINC in their preferences.
  
<li> 
Two scheduler request 
are made at the same time, so both projects think there is a larger amount 
of space available to each of them.
Both projects get RPC replies which 
have files to be downloaded, but the total file size is larger than each 
of their shares.
<p>
The first RPC reply 
to be returned will be given priority.
That project will delete any potential 
space used by offending projects and then start deleting its own files to 
make room for the new files.
When the next RPC reply is received, it is assumed 
that no files were deleted so the project will have to make room in its own 
share space for the new files.
This could have the result of deleting high 
priority files or those that haven't expired.
In the worst case, it will start 
to delete files from other projects that are not offending their disk share. 
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
      <li>Being the greatest 
        offender doesn't guarantee that it will be the first to loose files if 
        they are all non-expired or of a high priority, will delete lower priority 
        files from other offenders first
    </ul>
  
  <li>Never deleting workunit 
    files associated with the requesting project, give up this point
  <li>Deleting workunit files 
    associated with other projects only when there are no other files to delete 
    from any other project
</ul>

<p>
Clients method of checking for disk space which ensures the above:
<ol>
  <li>Check if there is enough 
    free space not currently used by any projects in the PDS? 
    <ul>
      <li> 
        and <em>increase project-&gt;size by file-&gt;nbytes</em>
    </ul>
  
  <li>Else check if there are 
    any projects that are offending their project share 
    <ul>
      <li>If yes, delete files 
        starting with the greatest offender and the lowest priority/expired files
      <li>Delete these priority/expired 
        files until the project size is below the project's share size
      <li>If there is not enough 
        space made, increase the priority to delete and repeat
    </ul>
  
  <li>Else check if there can 
    be room made in the project by deleting any non-workunit file 
    <ul>
      <li>
      <li>If there is now enough 
        free space, return <em>true</em> and <em>increase project-&gt;size by 
        file-&gt;nbytes</em>
    </ul>
  
  <li>Else check if there are 
    non-workunit files that can be deleted from other projects 
    <ul>
      <li>
      <li>If there is now enough 
        free space, return <em>true</em> and <em>increase project-&gt;size by 
        file-&gt;nbytes</em>
    </ul>
  
  <li>Else check if there are 
    any files that are not used in an active computation that can be deleted from 
    other projects 
    <ul>
      <li>
      <li>If there is now enough 
        free space, return <em>true</em> and <em>increase project-&gt;size by 
        file-&gt;nbytes</em>
    </ul>
  
  <li>Else, return false, there 
    is no way to associate the file with the project and guarantee the statements 
    above
</ol>

<h3>Pseudocode</h3>
<pre>
PROJECT:
   double size
	double share_size
	double resource_share
FILE_INFO:
	double nbytes
	
get_more_disk_space():
   for some PROJECT p and space_needed = number of bytes required
     	init total_space = 0
	
	total_space = free space in the project disk size
   if (total_space &gt; space_needed):
   	return true

	mark all projects as unchecked
	
	while(total_space &lt; space_needed):
		g_of = the greatest_offender
   	if(couldn't find one or g_of == p):
   		return false   	
   	mark g_of as checked
		if delete workunits:
			delete everything including non-active workunits from project
		else:
			if: delete any files it can, do that
			else: only delete files up to the point when it isn't an offender
	return true


associate_file():
	for some FILE_INFO fip and a PROJECT p
   init space_made = 0	
   // check offenders
   if(get_more_disk_space(p, fip-&gt;size, delete_only_offender, don't_delete_workunit)):
   	p.size += fip.nbytes
  		return true
	// check self
   is there any free space?
   try and delete expendable files from p
   if(space_made &gt; fip-&gt;nbytes):
   	return true
	// check other projects for any space
   if ((get_more_disk_space(p, fip-&gt;size, delete_anything, don't_delete_workunit)):
   	p.size += fip.nbytes;
   	return true
	// delete queued workunits in other projects
   if ((get_more_disk_space(p, fip-&gt;size, delete_anything, delete workunit)):
   	p.size += fip.nbytes;
   	return true
	// if hasn't return true yet, failure
   return false
</pre>

<h2>Work Fetch Policy</h2>
<p>
Because projects are allowed 
to fill up the project disk space entirely, the lack of space to run computation 
on workunits or ensure a constant supply (goals 2 &amp; 3) becomes an issue. 
In conjunction with the CPU Scheduler's work fetch policy, the data manager's 
work fetch policy's goals are to:
<ul>
<li>Calculate total free space.
<li>Calculate total free space if lowest priority files were deleted. 
<li>Calculate total free space if highest priority files were deleted.
</ul>
<p>
When an RPC request is made, 
the client communicates to the server the values described above and the server 
can make a decision on how much work to send to the client based on these values.

<h3>Calculating free space</h3>
<h3>Pseudocode</h3>
<pre>anything_free():
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

<h2>TODO: Future additions</h2>
<h3>Project deletion policy</h3>

<p>
There are three types of 
deletion policy that a project can specify in its config.xml
<ol>
<li><b>Priority deletion</b>.
Files with the lowest priority level get deleted first in the order they 
were introduced to the client (downloaded). 
<li><b>Expiration deletion</b>.
Whenever space runs out, all files that have past their expiration date 
are deleted first.
The expiration_date is a double value, any file who's accessed 
time plus the expiration_date is less than the time now is deleted.
<li><b>Least Recently Used (LRU)</b>.
The DEFAULT method, the last file to be downloaded/uploaded is deleted first. 
</ol>

<p>
Each method can be used simultaneously or separately.
The LRU policy is always used to determine the next file to delete. 
<p>
The policies are invoked by including the following in the config.xml file

<blockquote>&lt;deletion_policy_priority/&gt;<br>
  &lt;deletion_policy_expire/&gt;<br>
&lt;deletion_policy_lru/&gt;</blockquote>
<p>
If any of these flags are 
present in the config.xml, a similar flag will be passed to the client during 
an successful RPC request in which workunits were sent to the client.
If the priority deletion policy is invoked, remember to include a  
<blockquote> &lt;priority&gt;(int_value)&lt;/priority&gt; 
  </blockquote>
<p>
in all the file_infos in 
the workunit or result template if you want the priority of the file to be greater 
than one (default). 

<h3>Scheduling server changes</h3>
<p>
The client should communicate three values of disk space to the server. 
<ol>
<li>The amount of free PDS.
<li>The amount of free PDS if the client were to delete low priority files
<li>The amount of free PDS if the client were to delete high priority files
</ol>
<p>
The server can then make 
a more educated decision on how much work to send to a client.
It can also assume 
that this space is guaranteed because it was found doing the same process it 
would have used to delete the same amount of space.
";
?>
