<?php

require_once("docutil.php");

page_head("Running BOINC on Solaris");

echo "
The BOINC client software should work on:

<ul>
<li> Solaris versions 7, 8, 9, or 10
<li> Any SPARC processor
</ul>

<hr>
If BOINC produces messages of the form
<pre>
ld.so.1: boinc: fatal: libgcc_s.so.1: open failed: No such file or directory
</pre>
then you may need to add /usr/local/lib to your LD_LIBRARY_PATH
before running BOINC.

<hr>
If BOINC produces messages of the form
<pre>
ld.so.1: boinc: fatal: libstdc++.so.3: open failed: No such file or
directory
</pre>
then you probably have a newer version of libstdc++.so,
and you may need to symbolically link the newer version to libstdc++.so.3

<hr>
If BOINC produces messages of the form
<pre>
Can't create shared mem: -144 
</pre>
then you need to change your shared-memory configuration.
For example, make the following change to /etc/system and reboot:
<pre>
set strctlsz=4096
set shmsys:shminfo_shmmax=130000000
set shmsys:shminfo_shmseg=600
set shmsys:shminfo_shmmni=512
set shmsys:shminfo_shmmin=1
set semsys:seminfo_semmns=4096
set semsys:seminfo_semmni=4096
set semsys:seminfo_semmnu=4096
set semsys:seminfo_semume=64
set semsys:seminfo_semmap=512
set semsys:seminfo_semmsl=128 
</pre>
These settings should work on machines with up to 20 CPUs.
";

page_tail();
?>
