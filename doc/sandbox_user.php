<?php
require_once("docutil.php");
page_head("The Secure BOINC Client");
echo "
Version 5.5.4 of BOINC Manager for the Macintosh features new,
stricter security measures.
This additional security helps protect your computer data from potential theft
or accidental or malicious damage by limiting BOINC projects' access to your
system and data.  
Once we have tested this security implementation on the Mac,
we plan to extend it to other computer platforms, including Linux and Windows.

<p>
The installer sets special permission for the BOINC Manager and Client,
which allows them to write to the shared BOINC Data regardless
of which user is logged in.
If you <b>copy</b> BOINC Manager or the BOINC core client
without using the installer, it will not run properly.  
However, you can safely <b>move</b> the BOINC Manager within
the same disk drive or partition.
If you need multiple copies,
run the installer again after moving BOINC Manager;
this will create a fresh copy in the <code>/Applications</code> folder.

<p>
BOINC verifies that ownership and permissions are set properly
each time it is launched.
It will tell you to re-install BOINC if there is a problem.

<p>
If you experience problems with this software,
you can re-install a version of BOINC prior to 5.5.4;
this will automatically revert ownership and permissions to the earlier
implementation.

<h3>How it works</h3>
<p>
The new safeguards use the basic security protections built into UNIX
(the base underlying Mac OS X): permissions and ownership.

<p>
The administrator (usually the owner) of each computer creates
one or more users who can log in, can create private files,
and can share other files.
Some of these users are given administrative privileges,
some may not have these privileges.

<p>
There are also groups, which have one or more users as members.
For example, users with administrative privileges are usually members
of the 'admin' group.

<p>
In addition to these 'visible' users and groups,
the operating system contains a number of 'hidden'
users and groups which are used for various purposes.
A person cannot log in as one of these 'hidden' users.  

<p>
This structure of users and groups is used to provide security
by restricting what data and operations each person or application can use.
For example, many files belong to user 'system' (also called 'root')
and group 'wheel' so that non-privileged users can't modify them,
thus protecting the computer system from accidental or malicious harm.

<p>
Starting with version 5.5.4 of BOINC for the Macintosh,
the BOINC installer creates 2 new 'hidden'
users <b>boinc_master</b> and <b>boinc_project</b>,
and two new 'hidden' groups,
also named <b>boinc_master</b> and <b>boinc_project</b>
(unless they were created by a previous installation of BOINC.)

<p>
The installer automatically gives administrators
(users who are members of the 'admin' group)
membership in the two new groups,
so that they can manipulate BOINC files.
Non-admin users are denied direct access to these files,
protecting BOINC and its projects' files.
This is particularly useful where many people have access to the computer,
as in a school computer lab.

<p>
BOINC projects are given permission to access only project files,
protecting your computer in the event that someone downloads bad software
from a bogus project, or a legitimate project's
application has a bug that causes it to modify files erroneously.

<p>
Non-admin users can run the BOINC Manager,
but the Manager blocks non-admin users' access to certain functions,
such as Attach, Detach, Reset Project.
BOINC permits a non-admin user to override this restriction
by entering an administrator user name and password.

<p>
For technical details of the implementation, please see 
<a href=http://boinc.berkeley.edu/sandbox.php>http://boinc.berkeley.edu/sandbox.php</a>.
";
page_tail();
?>
