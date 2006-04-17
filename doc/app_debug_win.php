<?php
require_once("docutil.php");
page_head("Application debugging on Windows");
echo "
<h3>Anatomy of a Windows stack trace</h3>

<h4>Debugger version</h4>

<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
BOINC Windows Runtime Debugger Version 5.5.0

Dump Timestamp    : 04/16/06 23:41:39
Debugger Engine   : 4.0.5.0
Symbol Search Path: C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release;
C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release;
srv*c:\\windows\\symbols*http://msdl.microsoft.com/download/symbols;
srv*C:\\DOCUME~1\\romw\\LOCALS~1\\Temp\\symbols*http://boinc.berkeley.edu/symstore
      </pre>
    </td>
  </tr>
</table>
<p>
This area provides some basic information about the version of the BOINC debugger 
being used, when the crash occured, and what the internal version of the Windows
debugger technology is being used.
<p>
Symbol search paths are used to inform the debugger where it might be able to
find the symbol files related to the modules loaded in memory.  Entries prefixed with
'srv*' are used to denote a web based symbol store.  DbgHelp will use them if
symsrv can be loaded at the time of the crash.
<p>
<h4>Module List</h4>

<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
ModLoad: 00400000 00060000 C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release\\uppercase_5.10_windows_intelx86.exe (PDB Symbols Loaded)
ModLoad: 7c800000 000c0000 C:\\WINDOWS\\system32\\ntdll.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830
      </pre>
    </td>
  </tr>
</table>
<p>
Information about which modules were loaded into the processes memory space can be 
found here. The first hexdecimal value is the address in memory in which the module
was loaded, the second hexdecimal is the size of the module.
<p>
If a version record was found inside the module, it'll be dumped out as part of the
module list dump.
<p>
<h4>Exception Record</h4>

<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
*** UNHANDLED EXCEPTION ****
Reason: Breakpoint Encountered (0x80000003) at address 0x7C822583
      </pre>
    </td>
  </tr>
</table>
<p>


<h4>Stack traces</h4>

<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
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
      </pre>
    </td>
  </tr>
</table>
";

page_tail();
?>
