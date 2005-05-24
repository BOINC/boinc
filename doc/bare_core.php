<?php
require_once("docutil.php");
page_head("Installing the core client");
echo "
This type of installation
requires that you be familiar with the
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
</ul>
<p>
Instructions on running the core client (command-line options, etc.) are
<a href=client_unix.php>here</a>.
<p>
Mac OS X - specific instructions are
<a href=http://boinc-doc.net/site-boinc/oman-app/inst-mac.php>here</a>.
<p>
You'll probably want to arrange
to run the executable each time your machine boots or you log on.
Some examples follow.

<h2>Automatic startup on Unix</h2>
<p>
Users have contributed scripts for automatic
startup of BOINC on various versions of Unix:

<ul>
<li>
<a href=http://noether.vassar.edu/~myers/help/boinc/unix.html>Red Hat Linux</a>.
(includes general instructions for installing on Unix systems).
<li>
<a href=autostart_unix.txt>Linux with KDE</a>.
<li>
<a href=autostart_dennett.txt>Red Hat 9, should also work on Solaris</a>
<li>
<a href=gentoo.txt>Gentoo Linux</a> (from Gabor Nagy)
</ul>

<h2>Automatic startup on Mac OS X</h2>
<p>
[ The following is from Berki Yenigun, deadsmile at minitel.net ]
<p>
Here's a step by step guide to create a command file for Mac OS X 
Panther that will automatically launch Boinc command line client each 
time you log into your account:
<ol>
<li> Open TextEdit, paste and adapt the following two lines (adapt the 
path after the cd command to the folder in which you saved Boinc, and 
the name of Boinc file in the second line to the version you downloaded):

<pre>
cd myapplications/boinc
./boinc_4.19_powerpc-apple-darwin
</pre>

<li> Convert this into plain text format by clicking on Format and then on
Make Plain Text (rich text doesn't fit our needs), and save it 
with .command extension (replace the .txt) : Let's call our file 
boincscript, the file name & extension should be boincscript.command.
Once your file is saved, you can quit TextEdit.

<li> Now open Terminal, using cd command go to the folder where you saved 
the script file, then type
chmod +x boincscript.command
to make it executable. (use help if necessary : type man or man man.)
After that, you can quit Terminal. 

</ol>

It's almost done : all you need to do now is to add boincscript.command 

file to the login items in the 'Accounts' pane in 'System Preferences'. 

<p>
If you want to start Boinc manually again, you simply have to remove 
boincscript.command from the login items.

<p>
PS: You can use the
<a href=client_unix.php>core client command-line options</a> in your script.
An example:
<pre>
./boinc_4.19_powerpc-apple-darwin -return_results_immediately
</pre>


";
page_tail();
?>
