<?php
require_once("docutil.php");
page_head("Debugging the BOINC Windows client");
echo "
<h2>Symbol Files</h2>
<p>Symbol files are used by various Windows debugging engines to match up
memory addresses to symbol names.
Symbol names are names of variables, 
functions, and subroutines that a developer has named a piece of code
before the compilation process.

<p>Symbol files for BOINC can be found here:<br>
<a href=http://boinc.berkeley.edu/dl/>http://boinc.berkeley.edu/dl/</a>

<p>The files generally start out like boinc_xxx_pdb.zip where the xxx is 
the version of BOINC you are running.

<p>The uncompressed files should be stored in the same directory as BOINC.

<h2>Crashes</h2>
<p>If either BOINC or the BOINC Manager crash, 
(they unexpectedly stop running or disappear),
there should be some diagnostic information recorded in stderrdae.txt for 
BOINC and stderrgui.txt for the BOINC Manager.

<p>The diagnostic information we are looking for is called a callstack which 
is described below.

<h2>Callstacks</h2>
<p>Callstacks help us identify where the problem is by telling us which file
and what line number in the source code the error occurred in.

<p>Here is an example of a good callstack:
<pre>
***UNHANDLED EXCEPTION****
Reason: Access Violation (0xc0000005) at address 0x00411A87 write attempt to address 0x00000000

1: 06/08/05 15:07:01
1: c:\\boincsrc\\main\\boinc\\client\\cs_benchmark.c(310) +8 bytes (CLIENT_STATE::cpu_benchmarks_poll)
1: c:\\boincsrc\\main\\boinc\\client\\client_state.c(431) +0 bytes (CLIENT_STATE::do_something)
</pre>

<p>Here is an example of a bad callstack:
<pre>
***UNHANDLED EXCEPTION****
Reason: Access Violation (0xc0000005) at address 0x7C34FEDC read attempt to address 0x05595D90

1: 06/08/05 00:36:54
1: SymGetLineFromAddr(): GetLastError = 126
1: SymGetLineFromAddr(): GetLastError = 126
1: SymGetLineFromAddr(): GetLastError = 126
1: SymGetLineFromAddr(): GetLastError = 126
</pre>

<h2>Extracting callstacks</h2>
<p>If we need a callstack to help solve a problem that doesn't manifest itself as a crash you
can still extract a callstack from a running program using some freeware tools Microsoft
publishes called Debugging Tools for Windows.

<p>The Debugging Tools for Windows can be found here:
<a href=http://www.microsoft.com/whdc/DevTools/Debugging/default.mspx>Debugging Tools for Windows</a>

<p>
Once these tools are installed on your machine and the symbol files are extracted to the
BOINC installation directory you can run 'WinDBG.exe' and attach to the 'BOINC.exe' process
through the file menu to begin your debugging session.
Once 'WinDBG.exe' has attached itself 
to 'BOINC.exe' you can view the callstack by typing 'kb' in the command window
and then hitting enter.

<p>
Here is an example of the output 'WinDBG.exe' displays when it dumps a callstack:
<pre>
ChildEBP RetAddr  Args to Child              
0012fafc 7c821364 77e42439 00000000 0012fb40 ntdll!KiFastSystemCallRet
0012fb00 77e42439 00000000 0012fb40 ffffff4a ntdll!NtDelayExecution+0xc
0012fb68 77e424b7 00000064 00000000 0012fee0 kernel32!SleepEx+0x68
*** WARNING: Unable to verify checksum for C:\\Program Files\\BOINC\\boinc.exe
0012fb78 00430206 00000064 00424b3c 9999999a kernel32!Sleep+0xf
0012fb80 00424b3c 9999999a 3fb99999 00000002 boinc!boinc_sleep+0x16 [c:\\boincsrc\\main\\boinc_public\\lib\\util.c @ 251]
0012fee0 00424f87 0012fefc 9999999a 3fb99999 boinc!NET_XFER_SET::do_select+0x26c [c:\\boincsrc\\main\\boinc_public\\client\\net_xfer.c @ 319]
0012fef4 00407a2d 00000000 00000000 7c34f6f4 boinc!NET_XFER_SET::net_sleep+0x17 [c:\\boincsrc\\main\\boinc_public\\client\\net_xfer.c @ 245]
0012ff18 004225dd 9999999a 3fb99999 00000000 boinc!CLIENT_STATE::net_sleep+0x7d [c:\\boincsrc\\main\\boinc_public\\client\\client_state.c @ 369]
0012ff38 0042276f 7ffdf000 00000000 00000000 boinc!boinc_main_loop+0x7d [c:\\boincsrc\\main\\boinc_public\\client\\main.c @ 331]
0012ff60 00431c0d 00000002 017acfc0 016def68 boinc!main+0x11f [c:\\boincsrc\\main\\boinc_public\\client\\main.c @ 412]
0012ffc0 77e523cd 00000000 00000000 7ffdf000 boinc!mainCRTStartup+0x143 [f:\\vs70builds\\3077\\vc\\crtbld\\crt\\src\\crtexe.c @ 398]
0012fff0 00000000 00431aca 00000000 78746341 kernel32!BaseProcessStart+0x23
</pre>

<p>
Warning: You may need to add the BOINC directory to 'WinDBG.exe' symbol search path from the
File menu in order to get a valid callstack.
";
page_tail();
?>
