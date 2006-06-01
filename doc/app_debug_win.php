<?php
require_once("docutil.php");
page_head("Application debugging on Windows");
echo "
<h3>Contents</h3>
<ul>
    <li><a href=\"#Anatomy of a Windows stack trace\">Anatomy of a Windows stack trace</a>
        <ul>
            <li><a href=\"#Introduction\">Introduction</a>
            <li><a href=\"#Debugger version\">Debugger version</a>
            <li><a href=\"#Module List\">Module List</a>
            <li><a href=\"#Process Information\">Process Information</a>
            <li><a href=\"#Thread Information\">Thread Information</a>
                <ul>
                    <li><a href=\"#General Information\">General Information</a>
                    <li><a href=\"#Unhandled Exception Record\">Unhandled Exception Record</a>
                    <li><a href=\"#Registers\">Registers</a>
                    <li><a href=\"#Callstack\">Callstack</a>
                </ul>
            <li><a href=\"#Debug Message Dump\">Debug Message Dump</a>
            <li><a href=\"#Foreground Window Data\">Foreground Window Data</a>
        </ul>
    <li><a href=\"#Symbol Stores\">Symbol Stores</a>
    <li><a href=\"#Common Issues\">Common Issues</a>
</ul>

<h3><a name=\"Anatomy of a Windows stack trace\">Anatomy of a Windows stack trace</a></h3>
<h4><a name=\"Introduction\">Introduction</a></h4>
<p>
This section is going to describe what the output of a crash dump looks like and what it contains.
<p>
<h4><a name=\"Debugger version\">Debugger version</a></h4>
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
If you see a load library failure for either dbghelp.dll or symsrv.dll then there
is a pretty good chance that most of the data in the dump will be useless.
<p>

<h4><a name=\"Module List\">Module List</a></h4>
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
Information about which modules were loaded into the processes memory space can be found 
here. The first hexadecimal value is the address in memory in which the module was loaded; 
the second hexadecimal is the size of the module.
<p>
If a version record was found inside the module, it'll be dumped out as part of the module 
list dump.
<p>
If the correct symbol file can be found, it'll display what symbol file type is in use. 
The three most common symbol types in modern software are 'PDB', 'exports', and 
'-no symbols-'. Symbol files are pretty large and so most projects do not like to include 
them as part of the APP VERSION package.  Luckily Microsoft has created a technology called 
Symbol Stores which enable an application to be able to grab its symbol file from a web 
server at the time of a crash in a compressed format.  We will describe setting up a 
symbol store later in this document.
<p>
PDB files are generated at compilation time and usually have to be turned on for release 
builds. This file will contain all the needed information to generate a pretty good 
callstack which you can use to diagnose problems.
<p>
Export symbols usually only appear on DLLs since DLLs can export function pointers via 
the export table. When you see this in the module list you’ll only see functions which 
are listed in the export table in the callstack.
<p>
No symbols means that the runtime debugger could not determine a way to give you any 
symbolic information. You’ll only receive function pointers in the callstack.
<p>
<h4><a name=\"Process Information\">Process Information</a></h4>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
*** Dump of the Process Statistics: ***

- I/O Operations Counters -
Read: 24, Write: 0, Other 206

- I/O Transfers Counters -
Read: 0, Write: 358, Other 0

- Paged Pool Usage -
QuotaPagedPoolUsage: 29116, QuotaPeakPagedPoolUsage: 29228
QuotaNonPagedPoolUsage: 6624, QuotaPeakNonPagedPoolUsage: 6640

- Virtual Memory Usage -
VirtualSize: 64102400, PeakVirtualSize: 71045120

- Pagefile Usage -
PagefileUsage: 26218496, PeakPagefileUsage: 33697792

- Working Set Size -
WorkingSetSize: 19210240, PeakWorkingSetSize: 26361856, PageFaultCount: 6729
      </pre>
    </td>
  </tr>
</table>
<p>
This is some overall useful information about the process.  Most of the time the 
'Virtual Memory', 'Pagefile', and 'Working Set' are the most useful indicications 
of whether or not the process was under low available memory pressure from the OS.
<p>
<h4><a name=\"Thread Information\">Thread Information</a></h4>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
*** Dump of the Worker thread (a4): ***
      </pre>
    </td>
  </tr>
</table>
<p>
This identifies the thread for which additional information is going to be 
displayed.  Both the thread name and thread ID are displayed.  To set the thread
name for any thread you have created in your program just call
diagnostics_set_thread_name() as defined in diagnostics.h to set the thread name
for the currently executing thread.
<p>
<h5><a name=\"General Information\">General Information</a></h5>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
- Information -
Status: Waiting, Wait Reason: UserRequest, Kernel Time: 0.000000, User Time: 0.000000, Wait Time: 38241696.000000
      </pre>
    </td>
  </tr>
</table>
<p>
Status shows what state the thread was in when the snapshot was taken.  If the 
thread is waiting, wait reason will describe why the thread is waiting.  If the
thread is running both the base thread priority and current thread priority will
be displayed.
<p>
Kernel time, user time, and wait time describe how much time, in nanoseconds, the
thread has spent in each of those states.
<p>
<h5><a name=\"Unhandled Exception Record\">Unhandled Exception Record</a></h5>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
- Unhandled Exception Record -
Reason: Breakpoint Encountered (0x80000003) at address 0x7C822583
      </pre>
    </td>
  </tr>
</table>
<p>
This section if included in the thread describes what event ocurred that caused
the runtime debugger to engage.  Structured Exceptions in Windows are not the same
thing as C++ exceptions.  Unless you are using a compiler that knows about both types
it is unlikely that a C++ catch is going to actually catch this type of exception.
<p>
Further information about Structured Exception Handling can be found 
<a href=\"http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/about_structured_exception_handling.asp\">
here.
</a>
<p>
It is important to note that both hardware and software exceptions can bubble up from
the operating system through this mechinism.
<p>
The example above shows that 
EXCEPTION_BREAKPOINT(PlatformSDK\\Include\\winbase.h) was raised at 0x7C822583. 
EXCEPTION_BREAKPOINT is defined as STATUS_BREAKPOINT(PlatformSDK\\Include\\ntstatus.h) 
which is defined as ((NTSTATUS)0x80000003L).
<p>
<h5><a name=\"Registers\">Registers</a></h5>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
- Registers -
eax=00000000 ebx=00000000 ecx=77e4245b edx=7c82ed54 esi=77e88bfe edi=00459f40
eip=7c822583 esp=00aafd64 ebp=00aaffb4
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000202
      </pre>
    </td>
  </tr>
</table>
<p>
This is a basic dump of the processor registers at the time the exception was raised and 
will look different for each process type.
<p>
In this example these are the registers and flags for the Intel based x86 processor.
<p>
<h5><a name=\"Callstack\">Callstack</a></h5>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
- Callstack -
ChildEBP RetAddr  Args to Child
00aafd60 00402221 00000000 00000000 00000000 00000001 ntdll!_DbgBreakPoint@0+0x0 FPO: [0,0,0] 
00aaffb4 0042684e 77e66063 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!worker+0x0 (c:\boincsrc\main\boinc_samples\uppercase\upper_case.c:181) 
00aaffb8 77e66063 00000000 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!foobar+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) FPO: [1,0,0] 
00aaffec 00000000 00426840 00000000 00000000 00000000 kernel32!_BaseThreadStart@8+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) 
      </pre>
    </td>
  </tr>
</table>
<p>
<p>
<h4><a name=\"Debug Message Dump\">Debug Message Dump</a></h4>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
*** Debug Message Dump ****
      </pre>
    </td>
  </tr>
</table>
<p>
<p>
<h4><a name=\"Foreground Window Data\">Foreground Window Data</a></h4>
<table width=100%>
  <tr>
    <td bgcolor=ddddff width=100%>
      <pre>
*** Foreground Window Data ***
    Window Name      : 
    Window Class     : 
    Window Process ID: 16f8
    Window Thread ID : ae8
      </pre>
    </td>
  </tr>
</table>
<p>
<p>
<h3><a name=\"Symbol Stores\">Symbol Stores</a></h3>
<p>
<p>
<h3><a name=\"Common Issues\">Common Issues</a></h3>

";

page_tail();
?>
