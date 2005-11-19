<?php
require_once("docutil.php");
page_head("Joining a project");
echo "

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
