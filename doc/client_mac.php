<?php
require_once("docutil.php");
page_head("Installing BOINC on Mac OS/X");
echo "
<a name=mac>
<h3>Installing BOINC on Mac OS/X</h3>
<p>
The Mac OS X client will unpack correctly with gunzip on Mac OS X
10.2 (jaguar) or 10.3 (panther) as long as you type the command
within Terminal. Stuffit 7.x or newer will work under the Finder
in either OS X or OS 9, but I'd recommend using 'gunzip' or 'gzip -d'
within Terminal instead.

<p>
However, the two main browsers on OS X (IE 5.2.x and Safari 1.x) will
automatically unpack downloads by default, so your work may already
be done.

<p>
If you use IE, the boinc client will download and automatically unpack
leaving two files:
<ol>
<li>
   boinc_2.12_powerpc-apple-darwin
     [this will have the stuffit icon in the finder]

<li>
   boinc_2.12_powerpc-apple-darwin7.0.0
     [this will not have any icon in the finder]

</ol>
<p>
 #2 is the unpacked program ready-to-run. You can just start Terminal
 and run boinc.

<p>
If you use Safari, the boinc client will download and automatically
unpack, leaving a single file:
<ul>
<li>
   boinc_2.12_powerpc-apple-darwin7.0.0
     [this will not have any icon in the finder]
</ul>
 This is the unpacked program, but it's not yet ready-to-run (this is
 a bug with how Safari handles gzipped downloads; we'll fix this soon).

 <p>
 Here's what you have to do to fix the Safari download (apologies if
 you already know how to do this):

 <ul>
   <li> Create a folder in your home directory and put the boinc
     file in it
   <li> Start Terminal
   <li> 'cd' to the folder you just created
   <li> Type 'chmod +x boinc_2.12_powerpc-apple-darwin7.0.0'
     (without the quotes)
</ul>
 Now you can run BOINC.
 ";
page_tail();
?>
