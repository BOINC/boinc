<?php
require_once("docutil.php");
page_head("BOINC tools for Macintosh system administrators");
echo "
Several tools are available for Macintosh System Administrators, mostly in the 
form of command-line shell scripts to be run from the Terminal application. 
Please read the comments in each script for descriptions and directions.


<h2> Running BOINC as a daemon or system service</h2>
<p>
<a href=mac_build/Make_BOINC_Service.sh>Make_BOINC_Service.sh</a> 
is a command-line shell script to set up the BOINC Client to run as a daemon 
at system startup.  It can be used with either full GUI installations (BOINC 
Manager) or the stand-alone BOINC Client. 
<p>When run as a daemon:
<ul>
<li> The BOINC Client always runs even when no user is logged in.  However, it still 
observes the Activity settings as set by the Manager or the <b>boinc_cmd</b> application 
(Run always, Run based on preferences, Suspend, Snooze; Network activity always 
available, Network activity based on preferences, Network activity suspended.)
<li> Quitting the BOINC Manager will not cause the Client to exit.
<li> Application graphics (including screen saver graphics) are not available 
when the Client runs as a daemon.
<li> The following apply to the full GUI installation (BOINC Manager):
    <ul>
    <li> You may need version 5.5.9 or later to work properly as a daemon.
    <li> Normally, BOINC Manager starts up automatically when each user logs 
        in.  You can change this as explained below.
    <li> If you wish to block some users from using BOINC Manager, <i>move</i> 
        it out of the <b>/Applications</b> directory into a directory with 
        restricted permissions.  Due to the Manager's internal permissions, 
        you can <i>move</i> it but cannot <i>copy</i> it.  See 
        <a href=sandbox_user.php>The Secure BOINC Client</a> 
        for more information.
    </ul>
</ul>

<h2> Disabling auto-launch of BOINC Manager</h2>
<p>
By default, BOINC Manager starts up automatically when each user logs in.  You 
can override this behavior by removing the BOINC Manager Login Item for selected 
users, either via the Accounts System Preferences panel or by creating a 
<b>nologinitems.txt</b> file in the <b>BOINC Data</b> folder.  This should be a 
plain text file containing a list of users to be excluded from auto-lauch, one 
user name per line.
<p>
An easy way to create this file is to type the following in terminal, then edit 
the file to remove unwanted entries:
    <pre>
    ls /Users &gt; \"/Library/Application Support/BOINC Data/nologinitems.txt\"
    </pre>
After creating this file, run the installer.  The installer will delete the Login 
Item for each user listed in the file.  Entries which are not names of actual users 
are ignored (e.g., Shared, Deleted Users.)

<h2> Using BOINC's security features with the stand-alone BOINC Client</h2>
<p>
Beginning with version 5.5.4, the Macintosh BOINC Manager Installer implements 
additional security to protect your computer data from potential theft or accidental 
or malicious damage by limiting BOINC projects' access to your system and data, as 
described in <a href=sandbox_user.php>The Secure BOINC Client</a>.  
We recommend that stand-alone BOINC Client installations also take advantage of this protection.  
You can do this by running the 
<a href=mac_build/Mac_SA_Secure.sh>Mac_SA_Secure.sh</a> 
command-line shell script after installing the stand-alone Client, and again any time you 
upgrade the Client.
<p>
Although we don't recommend it, you can remove these protections by running the 
<a href=mac_build/Mac_SA_Insecure.sh>Mac_SA_Insecure.sh script.
";
page_tail();
?>
