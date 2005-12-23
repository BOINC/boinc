<?php

require_once("docutil.php");

page_head("Project security");

echo "
Before creating a BOINC project, read about
<a href=security.php>security issues in volunteer computing</a>.
BOINC provides mechanisms that address the major issues,
making volunteer computing safe both for you
and for participants.
<p>
<b>If you don't use these mechanisms correctly,
your project will be vulnerable to a variety of attacks.
In the worst case, your project could be used as a vector
to distribute malicious software to large numbers of computers.
This would be fatal to your project,
and would cause serious damage to volunteer computing in general.
</b>
<p>
We recommend that you do the following:

<ul>
<li> Secure each of your server computers as much as possible.
Read and implement the
<a href=http://www.cert.org/tech_tips/usc20_full.html>UNIX Security Checklist 2.0</a>
from AusCERT and CERT/CC.
<li> Put all server computers behind a firewall
that lets through minimal traffic (e.g., HTTP and SSH where needed).
<li> Read about <a href=http://dev.mysql.com/doc/refman/5.0/en/security-guidelines.html>MySQL general security guidelines</a>,
and make your MySQL server as secure as possible.
<li> Make sure your application doesn't become infected.
Secure your source-code repository, and examine all checkins.
If your application uses third-party libraries, make sure they're safe.
Read about <a href=http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO/index.html>Secure Programming for Linux and Unix</a>,
especially if your application does network communication.
<li> Use BOINC's <a href=code_signing.php>code-signing mechanism</a>,
and use a disconnected and physically secure code-signing computer.
</ul>
";
page_tail();
?>
