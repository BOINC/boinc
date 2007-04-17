<?php

require_once("../html/inc/page_translate.inc");
find_translation("dg.html");

require_once("docutil.php");

page_head("Desktop grid computing with BOINC");

echo "
<h2>Desktop grid versus volunteer computing</h2>
<p>
<b>Desktop grid computing</b>
is a form of distributed computing in which
an organization (such as a business)
uses its existing desktop PCs to handle its own
long-running computational tasks.
This differs from volunteer computing in several ways:
<ul>
<li> The computing resources can be trusted;
i.e. one can assume that the PCs don't return results that are
wrong either intentionally or due to hardware malfunction,
and that they don't falsify credit.
Hence there is typically no need for redundant computing.
<li> There is no need for screensaver graphics;
in fact it may be desirable to have the computation
be completely invisible and out of the control of the PC user.
<li> Client deployment is typically automated.
</ul>

<h2>BOINC as a desktop grid computing platform</h2>
<p>
Although it was originally designed for volunteer computing,
BOINC works very well for desktop grid computing.
The steps in creating a desktop grid are:
<ul>
<li>
<a href=create_project.php>Set up a BOINC server</a>,
develop or port applications, and test them.
Set <a href=work.php>workunit parameters</a> to disable redundancy.
<li>
Create an account with the <a href=prefs.php>general preferences</a>
that you want enforced on your desktop grid.
<li>
<a href=project_options.php>Configure your project</a>
to disable account creation.
<li>
<a href=win_deploy.php>Create a custom installer</a>
that includes the desired
<a href=client_unix.php>configuration files</a>.
Typically, this would include an account file
that would attach each client to the account on your project.
You might also want to include files that allow
clients to be remotely monitored and controlled.

<li>
Deploy your installer; on Windows networks
this can be done using <a href=win_deploy.php>Active Directories</a>.
</ul>

<p>
To ensure that outside hosts can't participate in your project
or access its files,
configure your firewall to prevent HTTP access to your BOINC server.

<p>
For more information on desktop grid computing using BOINC,
and some useful pre-compiled software, visit
<a href=http://desktopgrid.hu/>Desktopgrid.hu</a>.

";

page_tail(true);
?>
