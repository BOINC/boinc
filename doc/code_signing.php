<?php

require_once("docutil.php");

page_head("Code signing");

echo "
<p>
BOINC uses digital signatures to allow the core client
to authenticate executable files.
<p>
<b>
It is important that you use a proper
code-signing procedure for publicly-accessible projects.
If you don't, and your server is broken into,
hackers will be able to use your BOINC project to distribute
whatever malicious code they want.
This could result in the end of your project,
and possibly the end of all BOINC projects.
</b>
<ul>
<li> Choose a computer
(an old, slow one is fine) to act as your 'code signing machine'.
After being set up,
this computer <b>must remain physically secure
and disconnected from the network</b>
(i.e. keep it in a locked room
and put duct tape over its Ethernet port).
You'll need a mechanism for moving files to and from
the code-signing machine.
A USB-connected disk or CD-RW will work,
or if your files are small you can use a floppy disk.
<li>
Install <a href=key_setup.php>crypt_prog</a> on the code signing machine
(it's easiest if the machine runs Unix/Linux;
Windows can be used but requires Visual Studio 2003).

<li>
Run 'crypt_prog -genkey' to create a code-signing key pair.
Copy the public key to your server.
Keep the private key on the code-signing machine,
make a permanent, secure copy of the key pair
(e.g. on a CD-ROM that you keep locked up),
and delete all other copies of the private key.

<li>
To sign an executable file, move it to the code-signing machine,
run 'crypt_prog -sign' to produce the signature file,
then move the signature file to your server.

<li>
Use <a href=tool_update_versions.php>update_versions</a>
to install your application,
including its signature files,
in the download directory and database.
</ul>

<p>
There are less-secure variants;
e.g. you could keep the private key on a CD-ROM
that is only mounted during signature generation,
on a machine that is disconnected during signature generation.
But we do not recommend this;
a hacked computer could be running a hidden program that
steals the private key and transmits it when
the computer is connected again.

";
page_tail();
?>
