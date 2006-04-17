<?php
require_once("docutil.php");
page_head("Application debugging");
echo "
Some suggestions for debugging applications:

<h3>Standalone mode</h3>

When you have built your application
and linked it with the BOINC libraries,
you can run it in 'standalone mode' (without a BOINC core client present).
To do this, put instances of all input files in the same directory.
(with the proper logical, not physical, names).
The application should run and produce output files
(also with their logical names).
You can run the program under a debugger.

<p>
When you run an application in standalone mode,
the BOINC API will recognize this and take it into account.
A couple of things to note:
<ul>
<li> If your application does graphics, it will open a graphics window.
Closing this window will exit your application.
<li> boinc_time_to_checkpoint() will always return false,
so your application will never checkpoint.
</ul>


<h3>Using the anonymous platform mechanism</h3>

Once your application works in standalone mode
you'll want to run it from the BOINC core client.
This will exercise the various forms of interaction
with the core client.

<p>
To get this process started, create a test project,
add an application version and some work,
then run the core client.
It will download everything and run your application,
which will possibly crash.

<p>
At this point you'll want to start experimenting with your application.
It would be very tedious to create a new
application version for each change.
It's far easier to use BOINC's
<a href=anonymous_platform.php>anonymous platform</a> mechanism.
To do this:
<ul>
<li> Following the <a href=anonymous_platform.php>directions</a>,
create a file 'app_info.xml' in the client's project_* directory,
with the appropriate name and version number of your application.
<li> Each time your build a new version of your application,
copy the executable into the project_* directory,
making sure it has the appropriate name.
Then restart the core client.
</ul>

<p>
On Unix, it's possible to attach a debugger to a running process.
Use 'ps' to find the process ID of your application, then
something like
<pre>
gdb exec_filename PID
</pre>
to attach a debugger to it.

<h3>Getting and deciphering stack traces</h3>
<p>
Once your application is working on your own computers,
you're ready to test it with outside computers
(alpha testers initially).
It may crash on some computers, e.g. because their
software or hardware is different from yours.
You'll get some information back in the stderr_txt field
of the results.
If your application called boinc_init_diagnostics()
with the BOINC_DIAG_DUMPCALLSTACKENABLED flag set,
and you included symbols,
hopefully you'll get symbolic stack traces.
<p>

<p>
Here is an example of a stack trace on Windows:
<p>
<pre>
BOINC Windows Runtime Debugger Version 5.5.0

Dump Timestamp    : 04/16/06 23:41:39
Debugger Engine   : 4.0.5.0
Symbol Search Path: C:\BOINCSRC\Main\boinc_samples\win_build\Release;
C:\BOINCSRC\Main\boinc_samples\win_build\Release;
srv*c:\windows\symbols*http://msdl.microsoft.com/download/symbols;
srv*C:\DOCUME~1\romw\LOCALS~1\Temp\symbols*http://boinc.berkeley.edu/symstore


ModLoad: 00400000 00060000 C:\BOINCSRC\Main\boinc_samples\win_build\Release\uppercase_5.10_windows_intelx86.exe (PDB Symbols Loaded)
ModLoad: 7c800000 000c0000 C:\WINDOWS\system32\ntdll.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 77e40000 00102000 C:\WINDOWS\system32\kernel32.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 5e8d0000 000ce000 C:\WINDOWS\system32\OPENGL32.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 77ba0000 0005a000 C:\WINDOWS\system32\msvcrt.dll (7.0.3790.1830) (PDB Symbols Loaded)
    File Version   : 7.0.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 7.0.3790.1830

ModLoad: 77f50000 0009c000 C:\WINDOWS\system32\ADVAPI32.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 77c50000 0009f000 C:\WINDOWS\system32\RPCRT4.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 77c00000 00048000 C:\WINDOWS\system32\GDI32.dll (5.2.3790.2606) (PDB Symbols Loaded)
    File Version   : 5.2.3790.2606 (srv03_sp1_gdr.051230-1233)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.2606

ModLoad: 77380000 00092000 C:\WINDOWS\system32\USER32.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 68720000 00020000 C:\WINDOWS\system32\GLU32.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 73860000 0004c000 C:\WINDOWS\system32\DDRAW.dll (5.3.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.3.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft(R) Windows(R) Operating System
    Product Version: 5.3.3790.1830

ModLoad: 73b30000 00006000 C:\WINDOWS\system32\DCIMAN32.dll (5.2.3790.0) (PDB Symbols Loaded)
    File Version   : 5.2.3790.0 (srv03_rtm.030324-2048)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.0

ModLoad: 76aa0000 0002d000 C:\WINDOWS\system32\WINMM.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 71b70000 00036000 C:\WINDOWS\system32\uxtheme.dll (6.0.3790.1830) (PDB Symbols Loaded)
    File Version   : 6.00.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 6.00.3790.1830

ModLoad: 4b8d0000 00051000 C:\WINDOWS\system32\MSCTF.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 69500000 004f3000 C:\WINDOWS\system32\nvoglnt.dll (6.14.10.7801) (-exported- Symbols Loaded)
    File Version   : 6.14.10.7801
    Company Name   : NVIDIA Corporation
    Product Name   : NVIDIA Compatible OpenGL ICD
    Product Version: 6.14.10.7801

ModLoad: 02e50000 00006000 C:\WINDOWS\system32\ctagent.dll (1.0.0.11) (-exported- Symbols Loaded)
    File Version   : 1, 0, 0, 11
    Company Name   : Creative Technology Ltd
    Product Name   :   ctagent
    Product Version: 1, 0, 0, 11

ModLoad: 60970000 0000a000 C:\WINDOWS\system32\mslbui.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830

ModLoad: 03030000 00118000 C:\BOINCSRC\Main\boinc_samples\win_build\Release\dbghelp.dll (6.5.3.7) (PDB Symbols Loaded)
    File Version   : 6.5.0003.7 (vbl_core_fbrel(jshay).050527-1915)
    Company Name   : Microsoft Corporation
    Product Name   : Debugging Tools for Windows(R)
    Product Version: 6.5.0003.7

ModLoad: 03250000 00046000 C:\BOINCSRC\Main\boinc_samples\win_build\Release\symsrv.dll (6.5.3.8) (PDB Symbols Loaded)
    File Version   : 6.5.0003.8 (vbl_core_fbrel(jshay).050527-1915)
    Company Name   : Microsoft Corporation
    Product Name   : Debugging Tools for Windows(R)
    Product Version: 6.5.0003.8

ModLoad: 032a0000 00031000 C:\BOINCSRC\Main\boinc_samples\win_build\Release\srcsrv.dll (6.5.3.7) (PDB Symbols Loaded)
    File Version   : 6.5.0003.7 (vbl_core_fbrel(jshay).050527-1915)
    Company Name   : Microsoft Corporation
    Product Name   : Debugging Tools for Windows(R)
    Product Version: 6.5.0003.7

ModLoad: 77b90000 00008000 C:\WINDOWS\system32\version.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830



*** UNHANDLED EXCEPTION ****
Reason: Breakpoint Encountered (0x80000003) at address 0x7C822583

*** Dump of the Worker(offending) thread: ***
eax=00000000 ebx=00000000 ecx=77e4245b edx=7c82ed54 esi=77e424a8 edi=00454f20
eip=7c822583 esp=00a1fd64 ebp=00a1ffb4
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000246

ChildEBP RetAddr  Args to Child
00a1fd60 0040203b 00000000 00000000 00000000 00000001 ntdll!_DbgBreakPoint@0+0x0 FPO: [0,0,0] 
00a1ffb4 004239ce 77e66063 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!worker+0x0 (c:\boincsrc\main\boinc_samples\uppercase\upper_case.c:174) 
00a1ffb8 77e66063 00000000 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!foobar+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) FPO: [1,0,0] 
00a1ffec 00000000 004239c0 00000000 00000000 00000000 kernel32!_BaseThreadStart@8+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) 

*** Dump of the Timer thread: ***
eax=0002625a ebx=00000000 ecx=00000000 edx=00b1feb0 esi=00000001 edi=00000000
eip=7c82ed54 esp=00b1ff0c ebp=00b1ffb8
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000246

ChildEBP RetAddr  Args to Child
00b1ff08 7c822114 76aba0d3 00000002 00b1ff70 00000001 ntdll!_KiFastSystemCallRet@0+0x0 FPO: [0,0,0] 
00b1ff0c 76aba0d3 00000002 00b1ff70 00000001 00000001 ntdll!_NtWaitForMultipleObjects@20+0x0 FPO: [5,0,0] 
00b1ffb8 77e66063 00000000 00000000 00000000 00000000 WINMM!_timeThread@4+0x0 
00b1ffec 00000000 76aba099 00000000 00000000 49474542 kernel32!_BaseThreadStart@8+0x0 

*** Dump of the Graphics thread: ***
eax=00000000 ebx=7738e3f7 ecx=00000000 edx=00000000 esi=0012fc00 edi=7739ca9d
eip=7c82ed54 esp=0012fbb4 ebp=0012fbd8
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000246

ChildEBP RetAddr  Args to Child
0012fbb0 7739c78d 77392f3a 0012fc00 00000000 00000000 ntdll!_KiFastSystemCallRet@0+0x0 FPO: [0,0,0] 
0012fbd8 00424c3f 0012fc00 00000000 00000000 00000000 USER32!_NtUserGetMessage@16+0x0 
0012fcb0 00423ca3 00000001 00000001 00000001 00000001 uppercase_5.10_windows_intelx86!win_graphics_event_loop+0x14 (c:\boincsrc\main\boinc\api\windows_opengl.c:571) FPO: [0,46,0] 
0012fcd0 004220eb 00401078 0045c3b0 0040233c 00401078 uppercase_5.10_windows_intelx86!boinc_init_graphics_impl+0x30 (c:\boincsrc\main\boinc\api\graphics_impl.c:84) FPO: [2,7,0] 
0012fcdc 0040233c 00401078 00454f00 004483a4 00000002 uppercase_5.10_windows_intelx86!boinc_init_graphics+0x4b (c:\boincsrc\main\boinc\api\graphics_api.c:45) FPO: [1,0,0] 
0012fcf4 004023b1 00000002 0012fd0c 00142550 0012fd0c uppercase_5.10_windows_intelx86!main+0xa (c:\boincsrc\main\boinc_samples\uppercase\upper_case.c:233) FPO: [2,0,0] 
0012fe98 004035b4 00400000 00000000 001425a7 00000001 uppercase_5.10_windows_intelx86!WinMain+0x0 (c:\boincsrc\main\boinc_samples\uppercase\upper_case.c:110) FPO: [4,100,0] 
0012ffc0 77e523cd 00000000 00000000 7ffd9000 8707adb0 uppercase_5.10_windows_intelx86!WinMainCRTStartup+0x1d (f:\vs70builds\3077\vc\crtbld\crt\src\crt0.c:251) 
0012fff0 00000000 00403430 00000000 78746341 00000020 kernel32!_BaseProcessStart@4+0x0 (f:\vs70builds\3077\vc\crtbld\crt\src\crt0.c:251) 

Exiting...

</pre>
<p>
Otherwise, you should at least get numeric (hex) stack traces.
You can decipher these by running a symbolic debugger
with an unstripped version and typing in the hex addresses.
See http://developer.apple.com/technotes/tn2004/tn2123.html#SECNOSYMBOLS
";

page_tail();
?>
