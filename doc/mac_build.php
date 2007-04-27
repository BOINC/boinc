<?php
require_once("docutil.php");
page_head("Building BOINC Clients and Applications on Macintosh OS X");

echo "
This document applies to BOINC version 5.9.2 and later.
It has instructions for building BOINC for Macintosh OS X,
plus information for building science project applications to run under
BOINC on Macintosh OS X.  


<p>
Note: the information in this document changes from time to time for different
versions of BOINC.
For any version of BOINC source files,
the corresponding version of this document
can be found in the source tree at:
<pre>
boinc/mac_build/HowToBuildBOINC_XCode.rtf
</pre>


Contents of this document:

<ul>
<li> Important requirements for building BOINC software for the Mac.

<li> Building BOINC libraries to link with project applications.

<li> Building BOINC Manager, BOINC Client and BOINC libraries.

<li> Building BOINC Manager Installer.

<li> Debugging and BOINC security.

<li> Building project applications.

<li> Upgrading applications for Macs with Intel processors.

<li> Adding a Finder icon to your application
</ul>


<h2>Important requirements for building BOINC software for the Mac</h2>


All BOINC software for Power PC Macs must be built using
GCC 3.3 and MacOS10.3.9 SDK to assure backward compatibility with OS 10.3.
All BOINC software for Intel Macs must be built using GCC 4.0
and MacOS10.4.u SDK to allow cross-compiling.
This includes not only BOINC itself, but also the WxWidgets,
JPEG and cURL libraries, as well as all project applications.
<p>

Beware of using the wrong compiler!  Apple's release notes for GCC 4.0 say:

<blockquote>
If your application must support versions of Mac OS X prior to 10.3.9,
you must not use the GCC 4.0 compiler.
Instead, build your project using the GCC 3.3 compiler.
</blockquote>


Elsewhere on Apple's web site is the warning:

<blockquote>
Do not link C++ modules compiled with one of these compilers
against modules compiled with the other.
Even if the modules appear to link correctly,
C++ ABI differences may still cause problems that will not manifest
themselves until run time.
</blockquote>


Be sure to follow the directions in this document to ensure that these requirements are met.
<p>
Building BOINC now requires XCode Tools version 2.4.1 or later.  (Version 2.3 may work; this has not been tested.)
<p>
Source files are now archived using Subversion.  You can download svnX, a free GUI application for running Subversion from either
";
show_link("http://www.apple.com/downloads/macosx/development_tools/svnx.html");
echo "
or
";
show_link("http://www.lachoseinteractive.net/en/community/subversion/svnx/");
echo "
<p>
You also need to install Subversion itself.  One place to get it is:
";
show_link("http://www.codingmonkeys.de/mbo/");

echo "
<h2>Building BOINC libraries to link with project applications</h2>


If you are building a project application to be run by BOINC, you only need to build the boinc libraries libboinc_api.a, ibboinc.a, and (if you want graphics) libboinc_graphics_api.a.  There are two ways to do this:


<ol>
<li> Use the BOINC autoconf / automake scripts to build these libraries and the jpeg and curl libraries on which they depend.  You must do all of this twice: once on a PowerPC Mac running OS 10.3.x (do NOT use OS 10.4), and once on an Intel Mac running OS 10.4.x.
<p>

(If you wish, you can combine separate Intel and PowerPC builds in a single Universal Binary mach-O file using the command-line utility lipo.  For details on lipo, type 'man lipo' in Terminal; it is available on all Macs running OS10.4.x.)


<li> Use scripts setupForBOINC.sh and BuildMacBOINC.sh.  You do this once on any Macintosh (PowerPC or Intel) running OS 10.4.x and with XCode 2.4.1 (or later) installed.  This will produce Universal Binaries of all the libraries.  These can then be linked with both PowerPC applications and Mac Intel applications.

</ol>

This document gives instructions only for the second method.

<p>

After building the libraries as Universal Binaries using the second method, you probably still want to build your actual application separately on the two architectures: on a PowerPC Mac running OS 10.3.x (do NOT use OS 10.4), and also on an Intel Mac running OS 10.4.x.  Or you can look at the scripts buildcurl.sh and buildjpeg.sh for examples of environment settings which can cross-compile on one Mac running OS 10.4.x.



<h2>Building BOINC Manager with embedded Core Client plus libraries libboinc.a and libboinc_graphics_api.a</h2>


BOINC depends on three third-party libraries: wxMac-2.8.3, curl-7.16.1, and jpeg-6b.  You can obtain these from the following URLs:


<p>
wxMac-2.8.3 (needed  only if you are building the BOINC Manager):

";
show_link("http://www.wxwidgets.org");
show_link("http://prdownloads.sourceforge.net/wxwindows/wxMac-2.8.3.tar.gz");
echo "
<p>
curl-7.16.1:
";
show_link("http://curl.haxx.se");
show_link("http://curl.haxx.se/download/curl-7.16.1.tar.gz");
echo "
<p>
jpeg-6b (needed  only if you are building the BOINC libboinc_graphics_api.a library):

";
show_link("http://www.ijg.org");
show_link("ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v6b.tar.gz");
echo "
<p>

XCode 2.4.1 installs autoconf 2.59 and automake 1.63.  To determine the version number, type 'autoconf --version' or 'automake --version' .  Building wxMac-2.8.3 and curl-7.16.1 require autoconf 2.59 and automake 1.93 or later.  

<p>
Upgrades for autoconf and automake are available from www.gnu.org:  

";
show_link("http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.gz");
show_link("http://ftp.gnu.org/gnu/automake/automake-1.9.3.tar.gz");
echo "
<p>
XCode installed these utilities in the /usr/bin/ directory, but the upgrades by default will install in /usr/local/bin/.  If you install there, you must also set your PATH environment variable to include that location.  The scripts referenced below do this automatically.


<p>
As stated above, all BOINC software for Power PC Macs must be built using GCC 3.3 and MacOS10.3.9 SDK to assure backward compatibility with OS 10.3.  All BOINC software for Intel Macs must be built using GCC 4.0 and MacOS10.4.u SDK to allow cross-compiling.  


<p>
These are not done by either the XCode projects which come with wxMac-2.8.3, nor  the AutoMake scripts supplied with wxMac-2.8.3, curl-7.16.1, or jpeg-6b.  So be sure to use our special scripts to build these packages.


<p>
Building BOINC and the library packages on which it depends requires OS 10.4.4 and XCode 2.4.1 (or greater).  It may be possible to use XCode 2.3 and/or versions of OS X earlier than 10.4.4, but this has not been tested by the authors.


<ol>
<li> Create a parent directory within which to work.  In this description , we will call it BOINC_dev, but you can name it anything you wish.


<li> Put the following 3 directories inside the BOINC_dev folder (omit any you don't need):

<pre>
curl-7.16.1
jpeg-6b
wxMac-2.8.3
</pre>


Important: do not change the names of any of these 3 directories.


<li> Get the BOINC source tree from SVN, and put it in the same BOINC_dev folder.  To do this, type the following in Terminal:


<pre>
cd {path}/BOINC_dev/
svn co http://boinc.berkeley.edu/svn/trunk/boinc
</pre>

(You may change the name of the boinc directory to anything you wish.)

    <p>

The command above retrieves the source code from the HEAD or development branch of the SVN repository.
For more information on getting the BOINC source code, see:

";
show_link("http://boinc.berkeley.edu/source_code.php");
echo "


<li> Run the script to build the curl, jpeg and wxMac libraries as follows:

<pre>
cd {path}/BOINC_dev/boinc/mac_build/
source setupForBoinc.sh -clean
</pre>


If you don't wish to force a full rebuild of everything, omit the -clean argument.


<p>
Note: this script builds curl first, followed by jpeg and finally wxMac.  If you haven't downloaded wxMac because you aren't building the BOINC Manager, the script will build curl and jpeg.  Likewise, if you only downloaded curl because you need neither graphics nor the BOINC Manager, the script will build curl before quitting.


<li> Build BOINC as follows:


<pre>
cd {path}/BOINC_dev/boinc/mac_build/
source BuildMacBOINC.sh
</pre>


The complete syntax for this script is

<pre>
source BuildMacBOINC.sh [-dev] [-noclean] [-all] [-lib] [-client]
</pre>


The options for BuildMacBOINC.sh are:

<dl>
<dt>
-dev
<dd>
build the development (debug) version (native architecture only). 
default is deployment (release) version (universal binaries: ppc and i386).



<dt>-noclean
<dd> don't do a 'clean' of each target before building.

default is to clean all first.
</dl>


  The following arguments determine which targets to build

<dl>
<dt>-all
<dd>
build all targets (i.e. target 'Build_All' -- this is the default)


<dt> -lib
<dd> build the three libraries: libboinc_api.a, libboinc_graphics_api.a, libboinc.a


<dt> -client
<dd> build two targets: boinc client and command-line utility boinc_cmd

(also builds libboinc.a, since boinc_cmd requires it.)

</dl>

 Both -lib and -client may be specified to build five targets (no BOINC Manager.)

</ol>

Note: You may find three XCode projects in the BOINC_dev/boinc/mac_build/ directory: 

<ul>
  <li> boinc.pbproj is obsolete and should no longer be used.

  <li> wxMac-BOINC.xcodeproj was needed for building older versions of the wxMac library in conjunction with the older versions of the setupForBoinc.sh or buildWxMac.sh scripts.  It is not used for BOINC 5.9.2 or later. 

  <li> boinc.xcodeproj builds BOINC.  It can be used either with the BuildMacBOINC.sh script or as a stand-alone project.  It has two extra build configurations, i386-Deployment and ppc-Deployment, which can be used for testing only to build for just one architecture.  The Development build configuration builds only the native architecture and is used for debugging.  The Deployment build configuration builds a universal binary and is suitable for release builds.

</ul>

<h2>Building BOINC Manager Installer</h2>


To build the Installer for the BOINC Manager, if the BOINC version number is x.y.z, you must be logged in as an administrator.  Type the following in Terminal, then enter your administrator password when prompted by the script:


<pre>
cd {path}/BOINC_dev/boinc/
source {path}/BOINC_dev/boinc/mac_installer/release_boinc.sh x y z
</pre>


Substitute the 3 parts of the BOINC version number for x y and z in the above.  For example, to build the installer for BOINC version 5.5.4, the command would be

<pre>
source {path}/BOINC_dev/boinc/mac_installer/release_boinc.sh 5 5 4
</pre>

This will create a directory 'BOINC_Installer/New_Release_5_5_4' in the BOINC_dev directory.


<p>
To build version 5.5.4 of the Grid Republic flavor of BOINC, you would type: 

<pre>
cd {path}/BOINC_dev/boinc/
source {path}/BOINC_dev/boinc/mac_installer/release_GridRepublic.sh 5 5 4
</pre>

This will create a directory 'BOINC_Installer/New_Release_GR_5_5_4' in the BOINC_dev directory.



<h2>Debugging and BOINC security</h2>


Version 5.5.4 of BOINC Manager for the Macintosh introduced new, stricter security measures.  For details, please see the file boinc/mac_installer/Readme.rtf and
";
show_link("http://boinc.berkeley.edu/sandbox.php  ");
echo "


<p>
The GDB debugger can't attach to applications which are running as a diferent user or group so it ignores the S_ISUID and S_ISGID permisison bits when launching an application.  To work around this, BOINC does not use the special boinc_master or boinc_project users or groups when run from XCode.  


<p>
The Development build only of the BOINC Manager allows you to change the ownership and permission settings of the BOINC Data and executables by entering an administrator user name and password.  This also streamlines the development cycle by avoiding the need to run the installer for every change.


<p>
To restore the standard ownerships and permissions, run the installer.



<h2>Building project applications</h2>


<h3>Upgrading applications for Macs with Intel processors</h3>



Apple began shipping Macs with Intel processors on January 10, and Apple expects to convert all its lines of computers to Intel by the end of 2006.  


<p>
All future releases of BOINC will include 'universal binary' builds for the Macintosh of BOINC Manager, command-line BOINC client and the boinc_cmd command-line tool.  (Universal binaries contain both PowerPC and Intel executables in one file; the Macintosh OS automatically selects the appropriate one for that computer.)


<p>
The advantage of 'universal binaries' is that you only need to have one copy of the application, and it will run on either PowerPC or Intel Macs, so users don't need to choose between two options.  Since BOINC participants manually download BOINC from the web site, we will be providing BOINC in  'universal binary' form.


<p>
However, participants do not manually download project applications; this is done automatically by BOINC.  So there would be no advantage to combining the Intel and PowerPC versions in a single 'universal binary' file, but doing so would double the size of the download.


<p>
So BOINC treats Intel Macs as a new, separate platform.  BOINC previously directly supported four platforms: PowerPC Macs (powerpc-apple-darwin), Intel Linux (i686-pc-linux-gnu), Windows (windows-intelx86) and Solaris (sparc-sun-solaris2.7).   


<p>
We have now added a fifth platform for Intel Macs (i686-apple-darwin).

<p>

As a temporary measure, projects can set their servers to deliver a copy of their current PowerPC application (renamed for the new platform) under the new i686-apple-darwin platform.  The OS will run it in compatibility mode, emulating a PowerPC.  (Apple calls this compatibility mode Rosetta, which of course has nothing to do with the Rosetta BOINC project.)  


<p>
If you do this, be sure to give your native Intel application a higher version number when you do release it, so that clients will download it.

<p>

However, running a PowerPC application  in compatibility mode has two significant drawbacks:

<ul>

<li> Screensaver graphics do not work.


<li> Since it is running under emulation, your application will run at reduced efficiency.  But the benchmarks are based on running native Intel applications.  This may cause scheduler problems, such as uncompleted deadlines and inadequate credit for participants.  

</ul>

So it is important to make a native Intel application available as soon as possible.

<p>

It is very easy to add a new platform to your server with the xadd utility.  For directions on how to do this, see these web pages:

";
show_link("http://boinc.berkeley.edu/platform.php");
show_link("http://boinc.berkeley.edu/tool_xadd.php");
echo "
<p>

BOINC supports all PowerPC Macs running OS 10.3.0 or later, and all Intel Macs.  (The Intel Macs themselves require OS 10.4.4 or later.)

<p>
The easiest way to build your application for these two platforms is to build each one on its native platform.  In other words, do your powerpc-apple-darwin build on a PowerPC Mac running OS 10.3.9, and your i686-apple-darwin build on an Intel Mac.


<p>
But Apple provides the tools to allow you to cross-compile your application on any Mac (PowerPC or Intel) running OS 10.4 or later.  Here is how:


<p>
All BOINC software for Power PC Macs must be built using GCC 3.3 and MacOS10.3.9 SDK to assure backward compatibility with OS 10.3.  If building a PowerPC application on an Intel Mac, you must also specify '-arch ppc' in the compiler and linker flags.


<p>
All BOINC software for Intel Macs must be built using GCC 4.0 and MacOS10.4.u SDK to allow cross-compiling.  If building an Intel application on a PowerPC Mac, you must also specify '-arch i386' in the compiler and linker flags.


<p>
You can find examples of how to do this for two different kinds of configure / make scripts in the HEAD branch of the BOINC SVN tree at boinc/mac_build/buildcurl.sh and boinc/mac_build/buildjpeg.sh.  


<p>
The lipo utility is used at the end of each of these scripts to combine the two binaries into a single 'Universal Binary' file.  You won't need to do that with you project applications, since you will be distributing them separately under the two platforms.  But if you prefer, you can create a Universal Binary and distribute the same file for both i686-apple-darwin and powerpc-apple-darwin platforms.


<p>
Note that the BOINC libraries (and any third-party libraries) which you link with your applications must be built with the same configuration as the application itself.  Follow the instructions earlier in this document to build the needed libraries.


<p>
Additional information on building Unix applications universal can  be found here:

";
show_link("http://developer.apple.com/documentation/Porting/Conceptual/PortingUnix/compiling/chapter_4_section_3.html");
echo "
<p>
and here:
";
show_link("http://developer.apple.com/documentation/MacOSX/Conceptual/universal_binary/universal_binary_compiling/chapter_2_section_7.html");
echo "
<p>
For information on making your code work with GCC 4:

";
show_link("http://developer.apple.com/releasenotes/DeveloperTools/GCC40PortingReleaseNotes/index.html");
echo "

<h3>Adding a Finder icon to your application</h3>


There is an optional api setMacIcon() in the libboinc_api.a library.  This allows science applications to display an application icon in the Dock) and in the Finder.   (The icon does not appear in the Dock until the application displays graphics.)  To implement this, do the following:


<ul>
<li> Use '/Developer/Applications/utilities/Icon Composer.app' to create a xxx.icns file.  (Use any name you wish instead of xxx.)


<li>  Convert the xxx.icns file to an app_icon.h file as follows: in Terminal, run: 

<pre>
{path}/MakeAppIcon_h {source_file_path}/xxx.icns {dest_file_path}/app_icon.h
</pre>

(The MakeAppIcon_h command-line utility is built by the Mac boinc XCode project in the 'boinc/mac_build/build/' directory.)  Add the app_icon.h file to your science application's project.


<li> In the science application's main(), add 

<pre>
#include \"app_icon.h\" 
</pre>

and call:

<pre>
setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
</pre>


<li> The science application must link with Carbon.framework to use setMacIcon().
</ul>


";

page_tail();
?>
