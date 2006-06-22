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
        <ul>
            <li><a href=\"#SymIntroduction\">Introduction</a>
            <li><a href=\"#SymRequirements\">Requirements</a>
            <li><a href=\"#SymProject\">Project Symbol Store</a>
            <li><a href=\"#SymAdd\">Adding symbols to the symbol store</a>
            <li><a href=\"#SymUpload\">Uploading symbols to the symbol store</a>
        </ul>
    <li><a href=\"#Common Issues\">Common Issues</a>
        <ul>
            <li><a href=\"#CommonIntroduction\">Introduction</a>
            <li><a href=\"#Common0x80000003\">Breakpoint Encountered (0x80000003)</a>
            <li><a href=\"#Common0xc000000d\">Invalid Parameter (0xc000000d)</a>
            <li><a href=\"#Common0xc0000096\">Privileged Instruction (0xc0000096)</a>
            <li><a href=\"#Common0xc00000fd\">Stack Overflow (0xc00000fd)</a>
            <li><a href=\"#Common0xe06d7363\">Out of Memory Exception (0xe06d7363)</a>
        </ul>
</ul>

<h3><a name=\"Anatomy of a Windows stack trace\">Anatomy of a Windows stack trace</a></h3>
<h4><a name=\"Introduction\">Introduction</a></h4>
<p>
This section is going to describe what the output of a crash dump looks like and what it contains.
<p>
<h4><a name=\"Debugger version\">Debugger version</a></h4>
"; block_start(); echo "
BOINC Windows Runtime Debugger Version 5.5.0

Dump Timestamp    : 04/16/06 23:41:39
Debugger Engine   : 4.0.5.0
Symbol Search Path: C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release;
C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release;
srv*c:\\windows\\symbols*http://msdl.microsoft.com/download/symbols;
srv*C:\\DOCUME~1\\romw\\LOCALS~1\\Temp\\symbols*http://boinc.berkeley.edu/symstore
"; block_end(); echo "
<p>
This area provides some basic information about the version of the BOINC debugger 
being used, when the crash occured, and what the internal version of the Windows
debugger technology is being used.
<p>
Symbol search paths are used to inform the debugger where it might be able to
find the symbol files related to the modules loaded in memory.
 Entries prefixed with 'srv*' are used to denote a web based symbol store.
DbgHelp will use them if symsrv can be loaded at the time of the crash.
<p>
If you see a load library failure for either dbghelp.dll or symsrv.dll then there
is a pretty good chance that most of the data in the dump will be useless.
<p>

<h4><a name=\"Module List\">Module List</a></h4>
"; block_start(); echo "
ModLoad: 00400000 00060000 C:\\BOINCSRC\\Main\\boinc_samples\\win_build\\Release\\uppercase_5.10_windows_intelx86.exe (PDB Symbols Loaded)
ModLoad: 7c800000 000c0000 C:\\WINDOWS\\system32\\ntdll.dll (5.2.3790.1830) (PDB Symbols Loaded)
    File Version   : 5.2.3790.1830 (srv03_sp1_rtm.050324-1447)
    Company Name   : Microsoft Corporation
    Product Name   : Microsoft® Windows® Operating System
    Product Version: 5.2.3790.1830
"; block_end(); echo "
<p>
Information about which modules were loaded into the processes memory
space can be found here.
The first hexadecimal value is the address in memory in which the module was loaded; 
the second hexadecimal is the size of the module.
<p>
If a version record was found inside the module,
it'll be dumped out as part of the module list dump.
<p>
If the correct symbol file can be found,
it'll display what symbol file type is in use. 
The three most common symbol types in modern software are 'PDB', 'exports', and 
'-no symbols-'.
Symbol files are pretty large and so most projects do not like to include 
them as part of the APP VERSION package.
Luckily Microsoft has created a technology called 
Symbol Stores which enable an application to be able to
grab its symbol file from a web 
server at the time of a crash in a compressed format.
 We will describe setting up a symbol store later in this document.
<p>
PDB files are generated at compilation time and usually have
to be turned on for release builds.
This file will contain all the needed information to generate a pretty good 
callstack which you can use to diagnose problems.
<p>
Export symbols usually only appear on DLLs since DLLs
can export function pointers via the export table.
When you see this in the module list you'll only see functions which 
are listed in the export table in the callstack.
<p>
No symbols means that the runtime debugger could not determine a way to give you any 
symbolic information.
You'll only receive function pointers in the callstack.
<p>
<h4><a name=\"Process Information\">Process Information</a></h4>
"; block_start(); echo "
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
"; block_end(); echo "
<p>
This is some overall useful information about the process.
Most of the time the 'Virtual Memory', 'Pagefile', and 'Working Set'
are the most useful indications 
of whether or not the process was under low available memory pressure from the OS.
<p>
<h4><a name=\"Thread Information\">Thread Information</a></h4>
"; block_start(); echo "
*** Dump of the Worker thread (a4): ***
"; block_end(); echo "
<p>
This identifies the thread for which additional information is going to be 
displayed.
Both the thread name and thread ID are displayed.
To set the thread
name for any thread you have created in your program just call
diagnostics_set_thread_name() as defined in diagnostics.h to set the thread name
for the currently executing thread.
<p>
<h5><a name=\"General Information\">General Information</a></h5>
"; block_start(); echo "
- Information -
Status: Waiting, Wait Reason: UserRequest, Kernel Time: 0.000000, User Time: 0.000000, Wait Time: 38241696.000000
"; block_end(); echo "
<p>
Status shows what state the thread was in when the snapshot was taken.
If the thread is waiting, wait reason will describe why the thread is waiting.
If the thread is running both the base thread priority
and current thread priority will be displayed.
<p>
Kernel time, user time, and wait time describe how much time, in nanoseconds, the
thread has spent in each of those states.
<p>
<h5><a name=\"Unhandled Exception Record\">Unhandled Exception Record</a></h5>
"; block_start(); echo "
- Unhandled Exception Record -
Reason: Breakpoint Encountered (0x80000003) at address 0x7C822583
"; block_end(); echo "
<p>
This section if included in the thread describes what event ocurred that caused
the runtime debugger to engage.
Structured Exceptions in Windows are not the same thing as C++ exceptions.
Unless you are using a compiler that knows about both types
it is unlikely that a C++ catch is going to actually catch this type of exception.
<p>
Further information about Structured Exception Handling can be found 
<a href=\"http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/about_structured_exception_handling.asp\">
here.
</a>
<p>
It is important to note that both hardware and software exceptions can
bubble up from the operating system through this mechinism.
<p>
The example above shows that 
EXCEPTION_BREAKPOINT(PlatformSDK\\Include\\winbase.h) was raised at 0x7C822583. 
EXCEPTION_BREAKPOINT is defined as STATUS_BREAKPOINT(PlatformSDK\\Include\\ntstatus.h) 
which is defined as ((NTSTATUS)0x80000003L).
<p>
<h5><a name=\"Registers\">Registers</a></h5>
"; block_start(); echo "
- Registers -
eax=00000000 ebx=00000000 ecx=77e4245b edx=7c82ed54 esi=77e88bfe edi=00459f40
eip=7c822583 esp=00aafd64 ebp=00aaffb4
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000202
"; block_end(); echo "
<p>
This is a basic dump of the processor registers at the time the
exception was raised and will look different for each process type.
<p>
In this example these are the registers and flags for the Intel based x86 processor.
<p>
<h5><a name=\"Callstack\">Callstack</a></h5>
"; block_start(); echo "
- Callstack -
ChildEBP RetAddr  Args to Child
00aafd60 00402221 00000000 00000000 00000000 00000001 ntdll!_DbgBreakPoint@0+0x0 FPO: [0,0,0] 
00aaffb4 0042684e 77e66063 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!worker+0x0 (c:\boincsrc\main\boinc_samples\uppercase\upper_case.c:181) 
00aaffb8 77e66063 00000000 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!foobar+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) FPO: [1,0,0] 
00aaffec 00000000 00426840 00000000 00000000 00000000 kernel32!_BaseThreadStart@8+0x0 (c:\boincsrc\main\boinc\api\graphics_impl.c:75) 
"; block_end(); echo "
<p>
This describes the state in which the thread was in at the time of the exception.
ChildEBP and RetAddr are not really useful unless you can reproduce
the issue using the same OS version.
<p>
Args to Child are the first four parameters passed to the function.
<p>
The next piece of information has the following format:<br>
&lt;Module Name&gt;!&lt;Function Name&gt;@&lt;Function Ordinal&gt;+&lt;Symbol Offset&gt; &lt;File/Line Information&gt;
<p>
<dl>
  <dt>Module Name</dt>
  <dd>The friendly name for the DLL or EXE.</dd>
  <dt>Function Name</dt>
  <dd>The name of the function.</dd>
  <dt>Function Ordinal</dt>
  <dd>Function ordinals only apply to functions that are publically exported from a DLL.</dd>
  <dt>Symbol Offset</dt>
  <dd>If the symbol file being used has private symbols stripped, then the symbol displayed may 
      not be the correct symbol.
	  You can use the symbol offset to lookup the correct symbol
      from the object file or some other tool that can map symbols to source.</dd>
  <dt>File/Line Information</dt>
  <dd>Source file and line number information is not available in all symbol file formats, but
      if it is there it'll be displayed.
	  PDB files are generally the best symbol file format to use.</dd>
</dl>
<h4><a name=\"Debug Message Dump\">Debug Message Dump</a></h4>
"; block_start(); echo "
*** Debug Message Dump ****
"; block_end(); echo "
<p>
This feature is disabled by default.
<p>
What is allows for is capturing the debugger viewport data at runtime
just as though you were running the application within a debugger.
Since all applications use the same block of memory is can
slow down any and all applications that want to write to the debugger viewport,
even on release builds which is why it is disabled by default.
Video capture, edit, and playback
software tends to dump data to the viewport even when running a release build.
<p>
The following regedit script demonstrates how to enable the debug message dump:
"; block_start(); echo "
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Space Sciences Laboratory, U.C. Berkeley\BOINC Diagnostics]
\"CaptureMessages\"=dword:00000001
"; block_end(); echo "
To disable able the message capture use this regedit script:
"; block_start(); echo "
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Space Sciences Laboratory, U.C. Berkeley\BOINC Diagnostics]
\"CaptureMessages\"=dword:00000000
"; block_end(); echo "
<p>
<h4><a name=\"Foreground Window Data\">Foreground Window Data</a></h4>
"; block_start(); echo "
*** Foreground Window Data ***
    Window Name      : 
    Window Class     : 
    Window Process ID: 16f8
    Window Thread ID : ae8
"; block_end(); echo "
<p>
This shows which window has the user input focus.
The feature was originally meant to
detect potiential problems with 3rd party application injecting code into BOINC
applications and displaying UI to the user.
<p>
This feature turns out to be problematic since the foreground window might be hung 
which would mean that trying to get the window name and class would cause the runtime 
debugger to hang as well.
This feature will probably be removed in the future.
<p>
<h3><a name=\"Symbol Stores\">Symbol Stores</a></h3>
<h4><a name=\"SymIntroduction\">Introduction</a></h4>
<p>
In order to obtain useful diagnostic information in the event of an application crash, 
it is necessary to dump a callstack and any other relevant information about what was 
going on at the time of the crash.  Symbols are only needed during a crash event, 
therefore they are stripped from most applications to cut down on the binary size and 
bandwidth requirements to deploy a new release.
<p>
Without symbols, callstacks tend to be nothing more than a list of function pointers 
in memory.  A developer has to load the un-stripped executable in memory using the 
same operating system and similar processor to jump to that memory address in order 
to determine the function name and parameters.  This is very labor intensive and 
generally not a very fun job.
<p>
Microsoft created a technology called a 'Symbol Store' to use with their debugger 
technology which allows Windows debuggers to locate and download compressed symbol 
files to diagnose problems and convert function pointers into human readable text. 
This greatly speeds up the process of diagnosing and fixing bugs.
<p>
With the BOINC Runtime Debugger for Windows framework a project can publish their 
symbol files and only have to distribute the application to each of the BOINC 
clients.  When a crash event occurs the runtime framework will download the symbol 
file from the symbol store and then proceed to dump as much diagnostic information 
as possible to help projects diagnose the failure.
<p>
<h4><a name=\"SymRequirements\">Requirements</a></h4>
<p>
You'll need the latest stable release of the 
<a href='http://www.microsoft.com/whdc/devtools/debugging/default.mspx'>
  Debugging Tools for Windows.
</a>
<p>
Verify that your executable is setup to generate PDB debugging symbols for a release
build.  
<p>
Verify that the advance linker option to generate a checksum is enabled for a release
build.
<p>
You'll need to explictly name both your EXE and PDB before compilation since
the debugger bases the name of the PDB file off of information that is stored in the
executable header.  
<p>
<h4><a name=\"SymProject\">Project Symbol Store</a></h4>
<p>
Specifying a project wide symbol store is as easy as adding the symstore element
to your config.xml file for the project.
<p>
Below is an XML shred with an example symstore element.
<p>
". html_text("
<boinc>
    <config>
        <symstore>http://sample.example.com/symstore</symstore>
    </config>
</boinc>
")."
<p>
<h4><a name=\"SymAdd\">Adding symbols to the symbol store</a></h4>
<p>
<a href='http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/using_symstore.asp'>Symstore</a>
is a utility to manage symbol stores.  You'll want to create a local symbol store on
your Windows build machine in which you'll initially add new symbol files with each
revision of your application.
<p>
Symstore will compress the symbol file and then copy it into your local symbol store.
<p>
Below is an example command which you can run from the Windows command line or
cygwin command line.
<p>
"; block_start(); echo "
symstore.exe add /l /f c:\SampleSrc\*.pdb /s c:\symstore /compress /t \"Sample\" /v \"5.02\" /o /c \"Application Release\"
"; block_end(); echo "
<p>
<h4><a name=\"SymUpload\">Uploading symbols to the symbol store</a></h4>
<p>
Most projects tend to use scp to copy files between Windows machines and their project
server.
<p>
The example below copies the entire symstore to the target location.  After the copy
operation you can delete all the subdirectories except '000Admin' to save time uploading
for future application symbols.
<p>
"; block_start(); echo "
pscp.exe -r -C -batch c:\symstore sample@project.example.com:projects/sample/html/user/symstore
"; block_end(); echo "
<p>
<h3><a name=\"Common Issues\">Common Issues</a></h3>
<h4><a name=\"CommonIntroduction\">Introduction</a></h4>
<p>
In this section we'll list a few things to look for when reading the dumps.  Please keep in mind
that every application is different, but there should be enough similiarity that you should be
able to figure something out.
<p>
<h4><a name=\"Common0x80000003\">Breakpoint Encountered (0x80000003)</a></h4>
"; block_start(); echo "
*** Dump of the Worker thread (15c4): ***

- Information -
Status: Waiting, Wait Reason: UserRequest, Kernel Time: 156250.000000, User Time: 0.000000, Wait Time: 38126444.000000

- Unhandled Exception Record -
Reason: Breakpoint Encountered (0x80000003) at address 0x7C822583

- Registers -
eax=00000000 ebx=00000000 ecx=77e4245b edx=7c82ed54 esi=00b2fb78 edi=00b2fed8
eip=7c822583 esp=00b2fb74 ebp=00b2fee4
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000206

- Callstack -
ChildEBP RetAddr  Args to Child
00b2fb70 00444232 00b2ffb8 00b2feec 00000000 cccccccc ntdll!_DbgBreakPoint@0+0x0 FPO: [0,0,0] 
00b2fee4 0049a187 00000000 00000000 00000000 cccccccc uppercase_5.10_windows_intelx86!worker+0x8 (boinc_samples\uppercase\upper_case.c:176) 
00b2ffb8 77e66063 00000000 00000000 00000000 00000000 uppercase_5.10_windows_intelx86!foobar+0x8 (boinc\api\graphics_impl.c:74) 
00b2ffec 00000000 0043f4ba 00000000 00000000 00000000 kernel32!_BaseThreadStart@8+0x0 (boinc\api\graphics_impl.c:74) 
"; block_end(); echo "
<p>
This kind of error is an intentional error.  Somewhere in the code base it encountered a breakpoint.
<p>
The callstack will point to the function that started the call to the breakpoint function.
<p>
To add manual breakpoints to your code for diagnostics purposes you can call the Windows API:
"; block_start(); echo "
void DebugBreak( void );
"; block_end(); echo "
<p>
<h4><a name=\"Common0xc000000d\">Invalid Parameter (0xc000000d)</a></h4>
<p>
Starting with Visual Studio 2005, Microsoft re-vamped the whole C Runtime Library.  Part
of the re-vamp process was to do parameter checking on each function.  Places that would
normally return a NULL value now cause a structured exception to be thrown.
<p>
The nature of this structed exception is different than most as they specifically coded
it so that it will not engage the BOINC Runtime Debugger and it'll display a dialog box
asking the user if they wish to debug the error.  If the user cancels the error code
0xc000000d is returned without any more information.
<p>
To get more information with this error you'll need to create a function like this:
"; block_start(); echo "
#ifdef _WIN32
void AppInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line,	uintptr_t pReserved ) {
	fprintf(
		stderr,
		\"Invalid parameter detected in function %s. File: %s Line: %d\\n\",
		function,
		file,
		line
	);
	fprintf(
		stderr,
		\"Expression: %s\\n\",
		expression
	);
	// Cause a Debug Breakpoint.
	DebugBreak();
}
#endif
"; block_end(); echo "
<p>
The following code block should be added after the call to boinc_diagnostics_init():
"; block_start(); echo "
#ifdef _WIN32
	// Every once and awhile something looks at a std::vector or some other
	// CRT/STL construct that throws an exception when an invalid parameter
	// is detected.  In this case we should dump whatever information we 
	// can and then bail.  When we bail we should dump as much data as 
	// possible.
	_set_invalid_parameter_handler(AppInvalidParameterHandler);
#endif
"; block_end(); echo "
<p>
When this issues happens in the future it'll describe which CRT function call was passed
an invalid parameter and it should dump out the callstack for all threads.
<p>
The function blocks above overwrite the default behavior of the CRT when an invalid 
parameter is detected.
<p>
<h4><a name=\"Common0xc0000096\">Privileged Instruction (0xc0000096)</a></h4>
"; block_start(); echo "
- Unhandled Exception Record -
Reason: Privileged Instruction (0xc0000096) at address 0x008E9808

- Registers -
eax=00000400 ebx=00000000 ecx=00002922 edx=00b0c650 esi=01e1f7ec edi=027e2abc
eip=008e9808 esp=01e1f778 ebp=ffffffff
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00010202

- Callstack -
ChildEBP RetAddr  Args to Child
01e1f7b4 008ea16b 3f4bcaf9 3f827d51 01e1f7ec 01e1f7fc rosetta_beta_5.19_windows_intel!spherical+0x1 (rosetta++\structure.cc:1436) 
01e1f83c 008ec11f 00b38180 00000003 00000009 01e1f974 rosetta_beta_5.19_windows_intel!HSpair_score+0x0 (rosetta++\structure.cc:367) 
01e1f854 008b6d18 00b38058 00b3805c 00b38180 00b381b0 rosetta_beta_5.19_windows_intel!evaluate_ss+0x6 (rosetta++\structure.cc:102) 
01e1f974 00937bf6 a8af5c9d 0001c3f9 00001473 00000100 rosetta_beta_5.19_windows_intel!scorefxn+0x25 (rosetta++\score.cc:190) 
01e1f9b0 005e435f 00000009 008b7960 0001c3f9 a8af5cd5 rosetta_beta_5.19_windows_intel!main_frag_trial+0x4 (rosetta++\torsion_bbmove_trials.cc:446) 
01e1fb74 006f1c01 a8af5e9d 3030302e 3c303030 00000000 rosetta_beta_5.19_windows_intel!fold_abinitio+0xc (rosetta++\fold_abinitio.cc:270) 
01e1ffb0 006363c0 7c80b50b 00000000 3030302e 3c303030 rosetta_beta_5.19_windows_intel!main_rosetta+0x5 (rosetta++\main.cc:343) 
01e1ffb4 7c80b50b 00000000 3030302e 3c303030 00000000 rosetta_beta_5.19_windows_intel!foobar+0x0 (boinc\api\graphics_impl.c:75) 
01e1ffec 00000000 006363b0 00000000 00000000 00000000 kernel32!_BaseThreadStart@8+0x0 (boinc\api\graphics_impl.c:75) 
"; block_end(); echo "
<p>
In this example it appears the processor took exception to the fact that a user mode
process attempted to push a kernel mode address onto the stack without first switching
to kernel mode.
<p>
Look at the EBP register, 'ffffffff' when converted into a signed int is equal to '-1' 
and when converted to an unsigned int it is equal to 4GB.  On Windows anything above 2GB 
is considered a kernel mode address.  If the Windows machine supports PAE and the /3GB
boot option is specified in BOOT.INI then kernel addresses will start at 3GB instead.
<p>
What has probably happened here is that a function is about to be called and a 'push EBP'
instruction was called to push a new address onto the stack, the CPU threw the exception 
since the address was outside user mode land. EBP should have had a similar progression 
as all the other stack frames ChildEBP values.
<p>
If EBP had some random kernel mode address it would be pretty easy to dismiss this as
a CPU overheating.  'ffffffff' begs the question is the stack being overwritten by an
error result from another function?
<p>
Investigation of this issue is still ongoing.
<p>
<h4><a name=\"Common0xc00000fd\">Stack Overflow (0xc00000fd)</a></h4>
<p>
An application will throw this exception when one of it's threads exceed the 1MB stack
size allotment.
<p>
<h4><a name=\"Common0xe06d7363\">Out of Memory Exception (0xe06d7363)</a></h4>
"; block_start(); echo "
*** Dump of the  thread (d08): ***

- Information -
Status: Waiting, Wait Reason: UserRequest, Kernel Time: 156250.000000, User Time: 625000.000000, Wait Time: 4725904.000000

- Registers -
eax=00000000 ebx=00000004 ecx=0012e56c edx=7c82ed54 esi=00000798 edi=00000000
eip=7c82ed54 esp=0012e4e4 ebp=0012e554
cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000             efl=00000246

- Callstack -
ChildEBP RetAddr  Args to Child
0012e4e0 7c822124 77e6baa8 00000798 00000000 00000000 ntdll!_KiFastSystemCallRet@0+0x0 FPO: [0,0,0] 
0012e4e4 77e6baa8 00000798 00000000 00000000 0012e838 ntdll!_NtWaitForSingleObject@12+0x0 FPO: [3,0,0] 
0012e554 77e6ba12 00000798 ffffffff 00000000 0012e7d4 kernel32!_WaitForSingleObjectEx@12+0x0 
0012e568 0050e307 00000798 ffffffff 0012e838 00000000 kernel32!_WaitForSingleObject@8+0x0 
0012e57c 77e995f7 0012e838 e06d7363 02001ea0 00b01ba8 rosetta++.vc.boinc.release!boinc_catch_signal+0xa (boinc\lib\diagnostics_win.c:1798) 
0012e7d4 00988c7b 0012e838 00000000 00000000 00000000 kernel32!_UnhandledExceptionFilter@4+0x0 (boinc\lib\diagnostics_win.c:1798) 
0012e7f4 0097ab90 e06d7363 0012e838 009876a0 00af991c rosetta++.vc.boinc.release!_XcptFilter+0x9 (crt\src\winxfltr.c:237) 
0012e800 009876a0 00af991c 0012ffc0 00af9900 00000001 rosetta++.vc.boinc.release!__tmainCRTStartup+0x11 (crt\src\crt0.c:327) FPO: [0,0,0] 
0012e814 00974ace 00000000 00000000 0012e860 00000000 rosetta++.vc.boinc.release!@_EH4_CallFilterFunc@8+0x0 (crt\src\crt0.c:327) FPO: [0,0,4] 
0012e83c 7c82eeb2 0012ebf8 0012ffb0 0012e918 0012e8fc rosetta++.vc.boinc.release!_except_handler4+0x0 (crt\src\crt0.c:327) 
0012e860 7c82ee84 0012ebf8 0012ffb0 0012e918 0012e8fc ntdll!ExecuteHandler2@20+0x0 (crt\src\crt0.c:327) 
0012e908 7c82eda4 0012e000 0012e918 00010007 02000000 ntdll!ExecuteHandler@20+0x0 (crt\src\crt0.c:327) 
0012ebe8 77e55dea 0012ebf8 00b31740 e06d7363 00000001 ntdll!_RtlRaiseException@4+0x0 (src\crt0.c:327) 
0012ec48 0097039a e06d7363 00000001 00000003 0012ec74 kernel32!_RaiseException@16+0x0 (crt\src\crt0.c:327) 
0012ec80 0096fe55 0012ec90 00aa0df4 00a458b4 00a4579c rosetta++.vc.boinc.release!_CxxThrowException+0x0 (crt\prebuild\eh\\throw.cpp:166) 
0012ec9c 0044365a ffffffff 0012edf0 00447308 ffffffff rosetta++.vc.boinc.release!operator new+0x1e (crt\src\\new.cpp:63) 
0012eca8 00447308 ffffffff 45ffa9ac 0012edf0 0012ed84 rosetta++.vc.boinc.release!ObjexxFCL::FArrayB<double>::FArrayB<double>+0x2a (rosetta++\objexxfcl\farrayb.hh:160) 
0012ecd4 0049b234 0012ecf0 45ffa9f0 00000ece 00000001 rosetta++.vc.boinc.release!ObjexxFCL::FArray1D<double>::FArray1D<double>+0x38 (rosetta++\objexxfcl\farray1d.hh:145) 
0012ed18 0049b44b 0012edf0 0012ed84 0012edc4 45ffa824 rosetta++.vc.boinc.release!ObjexxFCL::cross+0x0 (rosetta++\objexxfcl\farray1d.hh:772) 
0012ee60 0049bf4a 45ffaa18 00000ece 00000002 00000001 rosetta++.vc.boinc.release!cross_benchmark+0x1a (rosetta++\boinc\boinc_rosetta_util.cc:257) 
0012ef0c 006f0f85 00000002 006f0fd5 00000018 0012ef2c rosetta++.vc.boinc.release!start_boinc_init+0x5 (rosetta++\boinc\boinc_rosetta_util.cc:96) 
0012ef14 006f0fd5 00000018 0012ef2c 001520a0 0012ef2c rosetta++.vc.boinc.release!main+0x0 (rosetta++\main.cc:109) 
0012ff28 0097ab69 00400000 00000000 001520bf 0000000a rosetta++.vc.boinc.release!WinMain+0x0 (rosetta++\main.cc:433) 
0012ffc0 77e523cd 00000000 00000000 7ffd8000 0000000f rosetta++.vc.boinc.release!__tmainCRTStartup+0x1c (crt\src\crt0.c:315) 
0012fff0 00000000 0097abd2 00000000 78746341 00000020 kernel32!_BaseProcessStart@4+0x0 (crt\src\crt0.c:315) 
"; block_end(); echo "
<p>
In the above example the new opterator was being requested to allocate 4GB of memory.  An out of 
memory exception was thrown.
<p>
";

page_tail();
?>
