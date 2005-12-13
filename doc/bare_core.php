<?php
require_once("docutil.php");
page_head("Installing the command-line client");
echo "
This type of installation requires that you be familiar with the
UNIX command-line interface.

<p>
After downloading the file:
<ul>
<li> Use gunzip to uncompress the file
if your browser has not done it for you.
<li> chmod +x the executable.
<li> put the executable into a separate directory (say, boinc/).
<li> run the executable.
    The first time you run it you'll need to attach it to a project, e.g.:
    <pre>
    boinc_client -attach_project http://setiathome.berkeley.edu df0d93e384a2700f70e0bc45a1b2c96c
    </pre>
    (substitute the actual name of the core client for
     <b>boinc_client</b> in the above example.)
</ul>

<p>
The core client has a number of other
<a href=client_unix.php>command-line options</a>.
<p>
You may want to
<a href=auto_start.php>automatically start the core client</a>
at boot time.
<p>
To control a running BOINC client, use the
<a href=boinc_cmd.php>BOINC command tool</a>.

<h2> Special cautions for Mac OS X</h2>
<p>
Normally, you should not install the separate command client if you are running the BOINC Manager.  
The BOINCManager.app bundle contains an embedded copy of the core client.  The installer adds the 
BOINC Manager to the installing user's list of <b>Login Items</b>  so it launches automatically whenever that user logs in.    
To have BOINC Manager run automatically when other users log in, you can manually add the BOINC Manager to each user's <b>Login Items</b>.  
For details, see <a href=mac_advanced.php>here</a>.
<p>
On Mac OS X, the core client executable is named <b>boinc</b>.
<p>
If you wish to run the embedded core client without launching the Manager, a typical command is:
    <pre>
    /Applications/BOINCManager.app/Contents/resources/boinc -redirectio -dir /Library/Application\\ Support/BOINC\\ Data/
    </pre>
Normally, quitting the BOINC Manager also quits the embedded core client.  But if you launch the core client (as in the above example) 
before launching the BOINC Manager, then it will continue to run even after the user quits the Manager.  
<p>
On Mac OS X, the BOINC screen saver will also launch the Manager's embedded core client, and will kill it when the screen saver is dismissed, 
but only if the core client was not already running.
<p>
If you must mix the stand-alone core client and the BOINC Manager on the same Mac OS X system, be careful of the following:
<ul>
<li>The BOINC Manager installer sets the set_user_id permission bit (S_ISUID) for the BOINC Manager and its embedded core client executable files.  
This causes BOINC to always run with the effective user ID set to the installing user.  This may cause permission problems if the 
stand-alone core client tries to access the same data as a different user.
<li>By default, the stand-alone core client expects the data to be in the same directory containing the executable.  But the BOINC Manager 
sets the current directory to \"/Library/Application Support/BOINC Data/\" before launching the embedded core client.
</ul>
If you want BOINC to operate on a separate set of data for each user on a Mac OS X system, then the stand-alone core client may be appropriate.  
But to avoid permission problems, make sure that users who have their own core client don't run the BOINC Manager.

";
page_tail();
?>
