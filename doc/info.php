<?php
require_once("docutil.php");

page_head('Rules and Policies');
echo "

The following rules and policy apply to BOINC.
BOINC-based projects may have additional rules and policies.

<h3>Run BOINC only on authorized computers</h3>

<p>
Run BOINC only on computers that you own,
or for which you have obtained the owner's permission.
Some companies and schools have policies that prohibit using their computers
for BOINC-based projects.

<h3>How BOINC will use your computer</h3>

<p>
When you run BOINC on your computer,
it will use part of the computer's CPU power, disk space, and network bandwidth.
You can control how much of your resources are used by BOINC,
and when it uses them.

<h3>Privacy policy</h3>

<p>
To avoid computing while you're typing,
BOINC monitors mouse and keyboard and activity
(it does not record keystrokes).
This may trigger some anti-virus software.

<p>
Your account on a BOINC-based project is identified by a name that you choose.
This name may be shown on the project's web site,
along with a summary of the work your computer has done for the project
and other BOINC projects.
If you want to be anonymous, choose a name that doesn't reveal your identity.

<p>
If you participate in a BOINC-based project,
information about your computer
(such as its processor type, amount of memory, etc.)
will be recorded by the project and used to decide
what type of work to assign to your computer.
This information will also be shown on the project's web site.
Nothing that reveals your computer's location
(e.g. its domain name or network address) will be shown.

<p>
To participate in a BOINC-based project,
you must give an address where you receive email.
This address will not be shown on the project's web site
or shared with organizations.
The project may send you periodic newsletters;
however, you can choose not to be sent these at any time.

<h3>Is it safe to run BOINC?</h3>

<p>
Any time you download a program through the Internet you are taking a chance:
the program might have dangerous errors,
or the download server might have been hacked.

<h3>Liability</h3>

<p>
The BOINC project and the University of California
assume no liability for damage to your computer,
loss of data, or any other event or condition that may occur
as a result of participating in BOINC-based projects.

";
page_tail();
?>
