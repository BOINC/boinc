<?php
require_once("docutil.php");
page_head("Choosing and joining projects");

echo "
<h2>Choosing projects</h2>
<p>
BOINC was developed at the University of California to support
<a href=http://setiathome.berkeley.edu>SETI@home</a>.
However, other distributed computing projects use BOINC.
BOINC allows you to participate in multiple projects,
and to control how your resources (such as computer time
and disk space) are divided among these projects.

<p>
Projects are independent, and each maintains its own servers.
The BOINC developers and the University of California
have no control over the creation of BOINC-based projects,
and in general do not endorse them.
The BOINC web sites lists some, but not all, projects.
A more complete list is on the
<a href=http://boinc-wiki.ath.cx/index.php?title=Catalog_of_BOINC_Powered_Projects>BOINC Wiki</a>.

<p>
When you participate in a project,
you entrust that project with the health of your
computer and the privacy of its data.
In deciding whether to participate in a project,
you should consider the following questions:

<ul>
<li> Do you trust the project to ensure that its applications
  won't damage your computer or cause security problems?
<li> Do you trust the project to use proper security practices on their servers?
<li> Does the project clearly describe its goals?
<li> Who owns the results of the computation?
  If the results are valuable, will they be freely available to the public
  or will they belong to a for-profit business?
</ul>

<h2>Joining a project</h2>
<p>
To participate in a BOINC project:
<ol>
<li> <a href=download.php>Download</a>,
install and run the BOINC client software.
<li>
When requested, enter the project's URL.
This is the address of the project's web site.
If you like, you can copy and paste it from your web browser.
<li>
When requested, enter your email address
and a password of your choosing.
<li>
BOINC will open a web page that allows you
to set other account data,
such as name, country, and preferences.
</ol>
<p>
That's it!

<h3>Multiple computers under one account</h3>
<p>
You can run BOINC on several computers under one account.
Once you have created an account as above,
you can add new computers in either of two ways:

<ol>
<li> Download and install the BOINC client software on each computer,
and follow the above procedure.

<li>
If the new computer is the same type as an existing computer,
create a BOINC home directory on the new computer
(the BOINC client software resides in a <b>BOINC home directory</b>.
If hosts share a network file system,
each host must have its own BOINC home directory).
Copy the core client executable and all 'account_*' files
from the existing computer to the new computer.
Do not copy 'client_state.xml'.
NOTE: this procedure does not work on Windows
because it does not copy registry entries.

</ol>

<h3>Older software versions</h3>
<p>
The procedure is a little different if you're using
a pre-5.0 version of BOINC,
or a command-line version of BOINC,
or the Mac OS X Menubar version of BOINC.
In these cases:
<ol>
<li> Visit the project's web site,
click on 'Create account' or similar link,
and fill out a web form.
<li> You will be sent an email containing an
'account key' (a long random string).
<li> Run the BOINC client software.
You will be asked for the project's URL and for your account key;
copy and paste this from the email you received.
</ol>

";
page_tail();
?>
