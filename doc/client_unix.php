<?
require_once("docutil.php");
page_head("Installing and running the BOINC command-line client");
echo "
<h3>The BOINC command-line client</h3>
<p>
Install the BOINC client by using gunzip to decompress the application.
Use 'chmod' to make it executable.
Put it in a directory by itself.
Run it manually, from your login script,
or from system startup files.
<p>
The command line client has several options:
";
list_start();
list_item("-attach_project",
    "Attach this computer to a new project.
    You must have an account with that project.
    You will be asked for the project URL and the account ID."
);
list_item("-show_projects",
    "Print a list of projects to which this computer is attached."
);

list_item("-detach_project URL",
    "Detach this computer from a project."
);

list_item("-reset_project URL",
    "Clear pending work for a project.
    Use this if there is a problem that is preventing
    your computer from working."
);

list_item("-update_prefs URL",
    "Contact a project's server to obtain new preferences."
);

list_item("-run_cpu_benchmarks",
    "Run CPU benchmarks.
    Do this if you have modified your computer's hardware."
);
list_item("-help",
    "Show client options."
);

list_item("-version",
    "Show client version."
);
list_end();
echo"
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
