<?php
require_once("docutil.php");
page_head("Joining a project");
echo "

<p>
To join a BOINC project:
<ol>
<li> Visit the project's web site and create an <b>account</b>.
This involves filling out a form with
<ul>
<li> An email address.
<li> A public 'screen name' (real name or nickname).
<li> other optional information
</ul>

<li> You will receive an email containing an <b>account ID</b>
(a long random string).

<li> Download, install and run the BOINC client program.
It will ask for a project URL and an account ID.
Enter the project's URL,
cut and paste the account ID from the email.

</ol>
<p>
That's it!
You can go to the project's web site to set your
<b>user preferences</b>.

<h3>Multiple computers under one account</h3>
<p>
You can run BOINC on many computers, all under one account.
Once you have created an account as above,
you can add new computers in either of two ways:

<ol>
<li> Download and install the BOINC client program on each computer,
and cut and paste the project URL and account ID as above.

<li> If the new computer is the same type as an existing computer,
create a BOINC directory on the new computer,
and copy the core client executable and all 'account_*' files
from the existing computer to the new computer.
Do not copy 'client_state.xml'.
NOTE: this does not work on Windows,
because it does not copy registry entries and other information.

</ol>

The BOINC client resides in a <b>BOINC home directory</b>.
If hosts share a network file system,
each host must have its own BOINC home directory. 
";
page_tail();
?>
