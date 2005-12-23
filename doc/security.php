<?php
require_once("docutil.php");
page_head("Security issues in volunteer computing");
echo "
<p>
Many types of attacks are possible in volunteer computing.
<ul>
<li> <b>Result falsification</b>.
Attackers return incorrect results.
<li> <b>Credit falsification</b>.
Attackers return results claiming more CPU time than was actually used.
<li>
<b>Malicious executable distribution</b>.
Attackers break into a BOINC server and,
by modifying the database and files, attempt to
distribute their own executable (e.g. a virus program) disguised as a
BOINC application.
<li>
<b>Overrun of data server</b>.
Attackers repeatedly send large files to BOINC data servers,
filling up their disks and rendering them unusable.
<li>
<b>Theft of participant account information by server attack</b>.
Attackers break into a BOINC server and steal email
addresses and other account information.
<li>
<b>Theft of participant account information by network attack</b>.
Attackers exploit the BOINC network protocols to steal account information.
<li>
<b>Theft of project files</b>.
Attackers steal input and/or output files.
<li>
<b>Intentional abuse of participant hosts by projects</b>.
A project intentionally releases an application that abuses participant
hosts, e.g. by stealing sensitive information stored in files.
<li>
<b>Accidental abuse of participant hosts by projects</b>.
A project releases an application that unintentionally abuses participant
hosts, e.g. deleting files or causing crashes.
</ul>
BOINC provides mechanisms to reduce the likelihood of some of these attacks.
<p>
<b>Result falsification</b>
<p>
This can be probabilistically detected using redundant computing and
result verification: if a majority of results agree (according to an
application-specific comparison) then they are classified as correct.
<p>
<b>Credit falsification</b>
<p>
This can be probabilistically detected using redundant computing and
credit verification: each participant is given the minimum credit from
among the correct results (or some other algorithm, such as the mean or
median of claimed credits).
<p>
<b>Malicious executable distribution</b>
<p>
BOINC uses code signing to prevent this. Each project has a key pair
for code signing.
The private key should be kept on a network-isolated
machine used for generating digital signatures for executables.
The public key is distributed to, and stored on, clients.
All files
associated with application versions are sent with digital signatures
using this key pair.
<p>
Even if attackers break into a project's BOINC servers, they will
not be able to cause clients to accept a false code file.
<p>
BOINC provides a mechanism by which projects can periodically change
their code-signing key pair.
The project generates a new key pair, then
(using the code-signing machine) generates a signature for the new
public key, signed with the old private key.
The core client will accept
a new key only if it's signed with the old key.
This mechanism is
designed to prevent attackers from breaking into a BOINC server and
distributing a false key.
<p>
<b>Denial of server attacks on data servers</b>
<p>
Each result file has an associated maximum size.
Each project has a
<b>upload authentication key pair</b>.
The public key is stored on the
project's data servers. Result file descriptions are sent to clients
with a digital signature, which is forwarded to the data server when the
file is uploaded. The data server verifies the file description, and
ensures that the amount of data uploaded does not exceed the maximum size.
<p>
<b>Theft of participant account information by server attack</b>
<p>
Each project must address theft of private account information
(e.g. email addresses) using conventional security practices.
All server machines should be protected by a firewall, and
should have all unused network services disabled.
Access to these machines should be done only with encrypted protocols like SSH.
The machines should be subjected to regular security audits.
<p>
Projects should be undertaken only the organizations that have
sufficient expertise and resources to secure their servers.
A successful
attack could discredit all BOINC-based projects, and
public-participation computing in general.
<p>
<b>Theft of participant account information by network attack</b>
<p>
Attackers sniffing network traffic could get user's account keys,
and use them to get the user's email address,
or change the user's preferences.
BOINC does nothing to prevent this.

<p>
<b>Theft of project files</b>
<p>
The input and output files used by BOINC applications are not encrypted.
Applications can do this themselves, but it has little effect
since data resides in cleartext in memory, where it is easy to access
with a debugger.
<p>
<b>Intentional abuse of participant hosts by projects</b>
</p>
<p>
BOINC does nothing to prevent this (e.g. there is no 'sandboxing' of
applications).
Participants must understand that when they join a BOINC project,
they are entrusting the security of their systems to that project.
<p>
<b>Accidental abuse of participant hosts by projects</b>
<p>
BOINC prevents some problems: for example, it detects when applications
use too much disk space, memory, or CPU time, and aborts them.
But applications are not 'sandboxed', so many
types of accidental abuse are possible.
Projects can minimize the likelihood by pre-released application testing.
Projects should test
their applications thoroughly on all platforms and with all input data
scenarios before promoting them to production status.
";
page_tail();
?>
