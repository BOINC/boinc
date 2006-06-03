<?php
require_once("docutil.php");
page_head("Using BOINC with modem, ISDN and VPN connections");
echo "

<p>
If you run BOINC on a computer with an Internet
connection that's not always on
(such as a modem, ISDN or VPN connection),
you can have BOINC connect in either of two ways:

<ul>
<li><b>Automatic</b>:
BOINC establishes an Internet connection as needed,
with no action on your part.
It optionally closes the connection when done.
This mode is useful
with dedicated modem lines, ISDN and VPN connections.
It is also be useful in combination with
<a href=prefs.php>general preferences</a> that set a time range when
BOINC is allowed to communicate (such as at night wnen
the phone line won't be in use or phone rates are cheaper).
<li><b>Manual</b>:
BOINC notifies you when it needs to connect to the Internet.
This mode is useful when a phone line is used for both
modem and regular phone calls.
It eliminates the situation where BOINC starts dialing
in the middle of a conversation.
</ul>

<h2>Automatic connection</h2>
<p>
<b>Note: this mode is currently available only on Windows</b>.
To use automatic connection:
<ul>
   <li>Select a default connection using the BOINC Manager
    <ul>
       <li>Click the 'Advanced' menu and click 'Options'.
       <li>Select the 'Connections' tab
       <li>Select the connection to be used by BOINC
       <li>Click the 'Set default' checkbox
       <li>Click the 'OK' button
     </ul>
   <li>Set network preferences on project web site
     <ul>
       <li>Set 'Confirm before connecting to Internet' to 'No'.
       <li>Set 'Disconnect when done' to 'Yes'.
       <li>Set 'Use network only between the hours of' as required
       <li>Click 'Update preferences' button at the bottom of the page
       <li>In BOINC Manager, 'update' the project to download the new
           preferences.
     </ul>
</ul>

<h2>Manual connection</h2>
<p>
In this mode, BOINC gets your permission before connection.
This mode is in effect if you haven't selected a default connection,
or if your 'confirm before connection' preference is set to Yes.
<p>
When BOINC Manager is minimized to the system tray (no open window),
and BOINC needs to communicate,
a little yellow balloon will pop up
with a message about BOINC needing a connection to the Internet.
You must open BOINC Manager to continue; to do so, double-click the BOINC icon.

<h3>Default connection</h3>
<p>
<b>This mode is currently available only on Windows</b>.
If you've selected a default connection (see above)
and your 'confirm before connection' preference is set to Yes,
then when BOINC needs to communicate you'll see a Confirm Yes/No dialog.
Click Yes to connect.
BOINC will use your default connection.
You may need to type in a user name and password depending
on how this connection is configured.

<h3>No default connection</h3>
<p>
If you don't have a default connection and BOINC needs to communicate,
the BOINC Manager will open a dialog asking you
to open an Internet connection.
You can then open a connection by whatever method you normally use,
and click OK in the dialog when you're done.
<p>

";
page_tail();
?>
