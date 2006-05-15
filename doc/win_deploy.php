<?php
require_once("docutil.php");
page_head("Deploying BOINC on a Windows network");
echo "
<h2>Customizing the installer</h2>
<p>
The BOINC installer is an MSI package.

<h3>Active Directory Deployment</h3>

Suppose you want to modify it so that you can
deploy BOINC across a Windows network using Active Directory,
and have all the PCs attached to a particular set of accounts.
Here's how to do this:


<ul>
<li> Download the BOINC client package and execute it with the /a parameter.
<li> Move the files to a directory that can be accessed
by all the computers on the network.
<li> Using <a href=http://support.microsoft.com/kb/255905/EN-US/>Microsoft ORCA</a>,
or some other MSI packaging tool, create an MSI Transform that contains the 
account_*.xml files of the projects you have already attached to
on another machine
plus the following parameters:
    <ul>
    <li>Single-user install:<br>
        <table>
            <tr BGCOLOR=#d8e8ff>
                <td>Parameter</td>
                <td>Description</td>
            </tr>
            <tr valign=top>
                <td>INSTALLDIR</td>
                <td>
                    The location to install BOINC to.<br>
                    Example: 'C:\\BOINC'
                </td>
            </tr>
            <tr valign=top>
                <td>SETUPTYPE</td>
                <td>
                    The type of installation to perform.<br>
                    Valid Values: 'Single'.
                </td>
            </tr>
            <tr valign=top>
                <td>ALLUSERS</td>
                <td>
                    Whether the shortcuts appear for just one user or all users.<br>
                    Valid Values: '0' for Single.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLESCREENSAVER</td>
                <td>
                    Whether to automatically enable the screensaver.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLELAUNCHATLOGON</td>
                <td>
                    Whether to automatically start BOINC when the installing user or all 
                    users sign on to the computer.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>LAUNCHPROGRAM</td>
                <td>
                    Whether to automatically launch BOINC Manager after setup completes.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
        </table>
    <li>Shared install:<br>
        <table>
            <tr BGCOLOR=#d8e8ff>
                <td>Parameter</td>
                <td>Description</td>
            </tr>
            <tr valign=top>
                <td>INSTALLDIR</td>
                <td>
                    The location to install BOINC too.<br>
                    Example: 'C:\\BOINC'
                </td>
            </tr>
            <tr valign=top>
                <td>SETUPTYPE</td>
                <td>
                    The type of installation to perform.<br>
                    Valid Values: 'Shared'.
                </td>
            </tr>
            <tr valign=top>
                <td>ALLUSERS</td>
                <td>
                    Whether the shortcuts appear for just one user or all users.<br>
                    Valid Values: '1' for shared.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLESCREENSAVER</td>
                <td>
                    Whether to automatically enable the screensaver.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLELAUNCHATLOGON</td>
                <td>
                    Whether to automatically start BOINC when the installing user or all 
                    users sign on to the computer.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>LAUNCHPROGRAM</td>
                <td>
                    Whether to automatically launch BOINC Manager after setup completes.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
        </table>
    <li>Service Install:<br>
        <table>
            <tr BGCOLOR=#d8e8ff>
                <td>Parameter</td>
                <td>Description</td>
            </tr>
            <tr valign=top>
                <td>INSTALLDIR</td>
                <td>
                    The location to install BOINC too.<br>
                    Example: 'C:\\BOINC'
                </td>
            </tr>
            <tr valign=top>
                <td>SETUPTYPE</td>
                <td>
                    The type of installation to perform.<br>
                    Valid Values: 'Service'.
                </td>
            </tr>
            <tr valign=top>
                <td>ALLUSERS</td>
                <td>
                    Whether the shortcuts appear for just one user or all users.<br>
                    Valid Values: '1' for service.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLESCREENSAVER</td>
                <td>
                    Whether to automatically enable the screensaver.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>ENABLELAUNCHATLOGON</td>
                <td>
                    Whether to automatically start BOINC when the installing user or all 
                    users sign on to the computer.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>LAUNCHPROGRAM</td>
                <td>
                    Whether to automatically launch BOINC Manager after setup completes.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
            <tr valign=top>
                <td>SERVICE_DOMAINUSERNAME</td>
                <td>
                    Which user account should the service use.<br>
                    Valid Values: '%ComputerName%\\%UserName%'<br>
                    &nbsp;&nbsp;&nbsp;&nbsp;%ComputerName% can be either the local computername or a domain name.<br>
                    &nbsp;&nbsp;&nbsp;&nbsp;%UserName% should be the username of the user to use.
                </td>
            </tr>
            <tr valign=top>
                <td>SERVICE_PASSWORD</td>
                <td>
                    The password for the account described in the SERVICE_DOMAINUSERNAME property.<br>
                    Valid Values: '%Password%' <br>
                    &nbsp;&nbsp;&nbsp;&nbsp;%Password% the password for the SERVICE_DOMAINUSERNAME user account.
                </td>
            </tr>
            <tr valign=top>
                <td>SERVICE_GRANTEXECUTIONRIGHT</td>
                <td>
                    Grant the above user account the 'Logon as a Service' user right.<br>
                    Valid Values: '0' for disabled, '1' for enabled.
                </td>
            </tr>
        </table>
    </ul>
</ul>

<h3>Command line deployment</h3>

<p>An example for the single-user install would be:<br>
msiexec /i boinc.msi /qn /l c:\boincsetup.log SETUPTYPE='Single' ALLUSERS=0 ENABLESCREENSAVER=0 ENABLELAUNCHATLOGON=0 LAUNCHPROGRAM=0</p>

<p>An example for the shared install would be:<br>
msiexec /i boinc.msi /qn /l c:\boincsetup.log SETUPTYPE='Shared' ALLUSERS=1 ENABLESCREENSAVER=0 ENABLELAUNCHATLOGON=0 LAUNCHPROGRAM=0</p>

<p>An example for the service install would be:<br>
msiexec /i boinc.msi /qn /l c:\boincsetup.log SETUPTYPE='Service' ALLUSERS=0 ENABLESCREENSAVER=0 ENABLELAUNCHATLOGON=0 LAUNCHPROGRAM=0 SERVICE_DOMAINUSERNAME='%ComputerName%\\%UserName%' SERVICE_PASSWORD='%Password%' SERVICE_GRANTEXECUTIONRIGHT=1</p>

";
page_tail();
?>
