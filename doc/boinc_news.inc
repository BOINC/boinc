<?

$project_news = array(

array("August 29, 2005",
    "<a href=http://www.acmqueue.com/modules.php?name=Content&pa=showpage&pid=313>A conversation with David Anderson</a> in ACM Queue Magazine."
),
array("August 14, 2005",
    "We are <a href=new_setup.php>redesigning BOINC</a> to use
    a user-supplied identifier and password
    instead of a system-supplied random key
    for identifying an account.
    Comments on the design are welcome."
),
array("August 9, 2005",
    "BOINC is transitioning to use <a href=http://curl.haxx.se/>libcurl</a>
    for HTTP operations.
    This will allow BOINC to use HTTPS (secure HTTP)
    for scheduler requests and file transfers."
),
array("August 3, 2005",
    "Non-open-source code (RSAEuro and GLUT) has been removed from BOINC.
    OpenSSL's crypto library is used for encryption."
),

array("July 25, 2005", 
    "A group of students from Cornell have written
    <a href=distributing_science_final.pdf>Distributing Science</a>,
    a very nice article about BOINC
    and some of the projects using it."
),
array("July 24, 2005",
    "<a href=http://www.fatbat.dk/thesis/>A paper by Jakob Pedersen and Christian S&#248;ttrup</a>
    discusses the interoperation of the Grid and BOINC."
),

array("July 11-12, 2005",
    "The <a href=cern_workshop.php>First Pangalactic BOINC Workshop</a>
    was held at CERN,
    bringing together people from ClimatePrediction.net,
    Einstein@home, CERN, and BOINC.
    "
),
array("June 25, 2005",
    "BOINC <a href=stats_sites.php>statistics</a> sites continue to evolve.
    Check out <a href=http://home.btconnect.com/Gabys_Bazar/hwupgrade.html>Every Earthly Hour</a>, developed by Hydnum Repandum"
),
array("June 24, 2005",
    "We have added
    <a href=http://boinc.berkeley.edu/api.php#credit>new API calls</a>
    that allow applications to do their own CPU benchmarking,
    or directly report their floating-point operation count.
    This will give more accurate credit numbers for
    applications that are optimized for specific architectures
    or that use non-CPU computing resources such as GPUs."
),
array("June 24, 2005",
    "We've added two mechanisms that give better estimates
    of result completion time
    (and thus better scheduling and fewer missed deadlines):
    1) we track the 'CPU efficiency': the average fraction
    of CPU time that BOINC apps receive while they're running
    (this can be less than one on systems that run
    compute-intensive tasks other than BOINC);
    2) we dynamically maintain a per-project upper bound on the
    ratio of actual CPU time to predicted CPU time."
),
array("June 24, 2005",
    "We've improved and simplified the CPU scheduling and work-fetch policies,
    using a detailed simulation of BOINC's weighted round-robin scheduling.
    When the simulator predicts that a deadline will be missed,
    the client switches to earliest-deadline-first scheduling.
    When the simulator predicts that fetching additional work
    will cause a deadline to be missed, it suspends work fetch."
),
array("June 24, 2005",
    "<a href=acct_mgt.php>Account management system</a>
    are web sites that let participants sign up for
    multiple projects with just a few clicks (and no account keys).
    The BOINC manager (development version) now lets you
    select, change, or get updated account info from an account manager."
),
array("June 24, 2005",
    "BOINC now works in Windows 'service' mode on computers
    with dialup connections and the 'confirm before connect' preference set.
    The connection confirmation and setup process has been
    moved to the BOINC manager, which (unlike a service)
    is able to do GUI activities."
),

array("June 16, 2005",
    "Climateprediction.net is featured in
    <a href=http://www.iee.org/oncomms/sector/PN_Article.cfm?objectID=3D100A5C-BC75-D279-D8D15A49EC4131B4>an article</a>
    in the journal of the Institute of Electrical Engineers."
),
array("June 3, 2005",
    "Two Wiki-based BOINC informational sites have been launched recently:
    <a href=http://boinc-doc.net/boinc-wiki/>The Unofficial BOINC Wiki</a>
    (in English) and
    <a href=http://faq.boinc.de/>Deutsche BOINC FAQ</a> (in German).
    Because these sites are Wikis,
    participants can add and modify content."
),
array("May 31, 2005",
    "BOINC is featured in <a href=http://sciencereview.berkeley.edu:7066/pdf/5.1/briefs_pt3.pdf>The Berkeley Science Review</a>."
),
array("May 14, 2005",
    "Eric Myers and Charlie Fenton have written
    excellent documents on
    <a href=compile.php>how to compile BOINC and BOINC applications
    on Windows, Linux, and Mac OS X</a>."
),
array("May 11, 2005",
    "BOINC and BOINC-based projects are discussed in
    the article 'Grassroots Computing'
    in the May 6 2005 issue of
    <a href=http://www.sciencemag.org/sciext/computers/>Science magazine</a>."
),
array("May 10, 2005",
    "LHC@home is a BOINC-based project being used to
    simulate the orbits of particles in CERN's Large Hadron Collider (LHC).
    Read about it in the
    <a href=http://bulletin.cern.ch/eng/articles.php?bullno=13/2005&base=art#Article1>CERN Bulletin</a>.
    "
),
array("May 10, 2005",
    "As the number of BOINC-based projects has increased,
    a problem has emerged - if a computer is attached to lots of projects,
    it may have trouble finishing work in time.
    We are revising the BOINC client's <a href=sched.php>CPU scheduling
    and work-fetch policies</a> to address this issue."
),
array("May 10, 2005",
    "Many BOINC applications have interactive graphics
    (e.g. you can zoom in/out using the mouse).
    For apps that use the latest BOINC API, you can interact
    even when in screensaver mode - just hold down the control key."
),
array("May 10, 2005",
    "We finally have an installer and GUI for Mac OS X!
    It is available as a development version,
    and will become the standard version soon."
),
array("April 11, 2005",
    "Volunteers have ported SETI@home and BOINC to
    <a href=http://boinc.berkeley.edu/download_other.php>a variety of platforms</a>,
    including Solaris/x86-64, Linux/x86-64, Linux/PPC, HP-UX, and FreeBSD."
),

array(
    "March 8, 2005",
    "We released the BOINC Menubar, a user-friendly client software
    version for Macintosh OS X.
    It can be installed and run through a GUI interface -
    no Unix commands.
    We also released the Windows installer announced in
    the January 1 2005 news item.
    Both are available on the <a href=download.php>Download</a> page."
),
array(
    "February 19, 2005",
    "The <a href=http://einstein.phys.uwm.edu/>Einstein@home</a>
    project was officially launched this morning.
    BOINC users are encouraged to participate in this project,
    which searches for spinning neutron stars (also called pulsars)
    using data from the LIGO and GEO gravitational wave detectors.
    "
),
array(
    "February 17, 2005",
    "We've added a <a href=web_translation.php>framework for web-site translations</a>
    (both BOINC-supplied and project-specific parts).
    We are adding support for
    <a href=acct_mgt.php>account management systems</a>
    that simplify participation in multiple projects.
    Security enhancement: all application files are digitally signed.
    "
),
array(
    "January 20, 2005",
    "BOINC source code is now distributed under the
    Lesser GNU Public License (LPGL)."
),
array(
    "January 19, 2005",
    "BOINC-based projects projects now have
    80,721 participants in 188 countries.
    Based on <a href=flops.php>current credit statistics</a>,
    together they supply 106 TeraFLOPS of computing power -
    far more than any conventional supercomputer."
),
array(
    "January 18, 2005",
    "BOINC's message board system supports moderation."
),
array(
    "January 10, 2005",
    "With help from the Einstein@home group, we have refined BOINC's
    <a href=sched_locality.php>locality scheduling</a> mechanism,
    It can now interact with project-specific programs
    that generate work on demand,
    and that maintain a 'working set' of data files
    for which work is distributed."
),
array(
    "January 1, 2005",
    "The Windows version of BOINC can now be configured
    to run when a particular user is logged in,
    to run when any user is logged in,
    or run all the time (as a service)
    even when no users are logged in."
),

array("December 10, 2004",
    "With considerable help from the Einstein@home group,
    we have application graphics working on all platforms, including Linux.
    To handle hosts without graphics libraries,
    we have developed a <a href=graphics.php>new architecture</a> in which
    graphics code resides in a dynamically loaded library."
),
array("December 6, 2004",
    "Added prioritization to work distribution."
),
array("December 3, 2004",
    "We have created a new email list,
    <a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_cvs>boinc_cvs@ssl.berkeley.edu</a>.
    A summary of each CVS checkin to the BOINC source tree
    will be posted to this list."
),

array(
    "December 2, 2004",
    "A technical paper about BOINC is
    <a href=grid_paper_04.pdf>here</a>.
    This paper appeared in the
    5th IEEE/ACM International Workshop on Grid Computing,
    November 8, 2004, Pittsburgh, USA."
),
array(
    "November 21, 2004",
    "We have developed a GUI for the core client.
    It's similar to the old Windows GUI but is implemented
    using <a href=http://www.wxwidgets.org/>wxWidgets</a>
    so it runs on all platforms.
    It offers new features such as the ability to
    suspend/resume projects and results,
    and project-specific web links.
    It uses a socket-based GUI RPC mechanism,
    which makes it possible to develop other GUIs,
    including ones that control many hosts.
    "
),
array(
    "November 21, 2004",
    "A new utility, db_purge, allows projects to
    flush old entries from the result and workunit tables,
    so that these tables have a bounded size."
),
array(
    "November 21, 2004",
    "We have developed a new Windows installer
    that lets you run BOINC as a service,
    and that is compatible with the Microsoft MSI framework."
),
array(
    "November 21, 2004",
    "The application graphics framework now works on
    all platforms, including Mac OS X."
),
array(
    "November 21, 2004",
    "The message-board system now supports self-moderation
    (i.e., 'ignore lists')."
),
array(
    "November 21, 2004",
    "BOINC servers can now use read-only database replicas
    for increased throughput."
),
array(
    "September 21, 2004",
    "XML Schemas for the statistics data exported by BOINC projects
    is <a href=BOINCSchemas/>here</a>,
    and graphical representations are
    <a href=BOINCSchemas/documentation/>here</a>."
),
array(
    "September 14, 2004",
    "We have created an email list, boinc_opt@ssl.berkeley.edu,
    for people interested in porting and optimizing BOINC applications.
    Sign up <a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_opt>here</a>."
),
array(
    "September 11, 2004",
    "We are working on a new type of work distribution, called
    <a href=sched_locality.php>Locality scheduling</a>,
    that sends work to hosts that already have the needed files."
),
array(
    "September 6, 2004",
    "The <a href=http://lattice.umiacs.umd.edu/>Lattice</a> project
    from the University of Maryland
    is developing a Grid system that integrates Globus,
    BOINC, and several other software components."
),
array(
    "September 1, 2004",
    "The new project LHC@home announces its first beta test,
    initially restricted to 1000 users.
    Please sign up at <a href=http://lhcathome.cern.ch>lhcathome.cern.ch</a>."
),
array(
    "August 26, 2004",
    "The <a href=http://climateprediction.net/>Climateprediction.net</a>
    project is now open to the public."
),
array(
    "August 6, 2004",
    "Users can choose whether applications should
    quit or suspend when they're not executing."
),
array(
    "August 5, 2004",
    "The core client now checks whether disk usage exceeds
    user-specified limits, and if so it deletes projects files
    in a way that respects project resource shares."
),
array(
    "August 5, 2004",
    "Large BOINC projects (like SETI@home) were getting
    on the order of a million files in upload/download directories.
    This caused OS performance problems.
    We fixed this by providing a two-level directory
    hierarchy for upload/download directories."
),
array(
    "August 3, 2004",
    "The Climateprediction.net project recently held an
    <a href=http://www.climateprediction.net/project/openday.php>Open Day</a>,
    describing its status and its plans to use BOINC.
    "
),
array(
    "July 9, 2004",
    "We made the core/app interface more flexible and general,
    providing better support for applications that consist
    of separate controller, worker, and graphics programs.
    We also added mechanisms that prevent
    multiple applications from running in the same slot,
    and that cause applications to exit if the core client dies.
    "
),
array(
    "July 9, 2004",
    "We changed the SQL queries used by the performance-critical
    server components, such as the transitioner and feeder,
    to boost maximum server throughput."
),
array(
    "July 9, 2004",
    "The core client now does preemptive scheduling of results.
    It time-slices between results,
    maintaining the approximate project resource shares.
    The work-fetch policy has also been modified;
    a single 'connection frequency' parameter replaces the
    min/max buffer parameter,
    and the policy matches the requirements of the CPU-scheduling policy."
),
array(
    "July 9, 2004",
    "We have added a mechanism that allows projects to
    upload and download 'persistent files'.
    This is the first step to using participant hosts for storage
    as well as computation, possibly creating
    the world's largest storage server."
),
array(
    "June 20, 2004",
    "The <a href=db_dump.php>statistics export mechanism</a> has been redesigned
    to give projects more control over what they export;
    e.g. they can shift the burden of sorting to statistics web sites."
),
array(
    "June 16, 2004",
    "New features: support for multi-program applications.
    Daily result quota per host.
    Applications can supply a static graphic
    (for screensaver) without writing code.
    Improved support for FORTRAN.
    Support for GUI RPCs in Windows client.
    Option for allowing GUI RPCs from non-local hosts.
    Check for keyboard/mouse input in Unix.
    Support for name/password authentication for HTTP proxies."
),

array("May 31, 2004",
    "BOINC news is available as an RSS feed;
    see Community and Resources."
),

array(
    "May 19, 2004",
    "CPU benchmarking (and credit) are now based on
    the Dhrystone and Whetstone benchmarks.
    Memory bandwidth is no longer measured."
),

array("May 18, 2004",
"Source code is now available
<a href=community.php>directly via CVS</a>.
Improved Unix signal handling.
Support app versions, workunits and results with lots of files
(changed XML fields from 4KB to 64KB).
Added
<a href=homogeneous_redundancy.php>homogeneous redundancy</a> mechanism.
"),

array("May 2, 2004",
"General preferences are now propagated from client to server,
but only to accounts with the same email address
as where the preferences originated.
"),

array("April 20, 2004",
"Added <a href=cpid.php>cross-project identification</a> system.
Added support for caching of PHP web pages,
and restored leaderboard pages.
"),

array("April 12, 2004",
"Recent fixes and features:
<a href=http://boinc.berkeley.edu/trickle.php>trickle messages</a>
for long-running work units;
benchmarking for hyperthreaded CPUs;
don't propagate preferences from hosts to projects;
check for file existence before running apps;
SOCKS proxy support;
separate suspend/resume for network activity;
fix race conditions in server;
add file compression code.
See checkin_notes for details.
"),

array("March 16, 2004",
"An unauthorized
<a href=calvin-hobbes-boinc.jpg>BOINC graphic</a>.
"),

array("February 11, 2004",
"BOINC is being used at the Scripps Research Institute
to run a distributed version of CHARMM
(a program for macromolecular simulations).
"),

array("February 1, 2004",
"BOINC now has an
<a href=anonymous_platform.php>anonymous platform mechanism</a>
which 1) allows computers of any type to participate
in a BOINC project, and 2) allows participants to
run only software they have compiled themselves,
should they so desire.

"),

array("January 22, 2004",
"The BOINC core client now provides
<a href=gui_rpc.php>a set of RPCs allowing separate GUIs to be developed</a>.
"),

array("November 26, 2003",
"The CVS repository of BOINC code (including web pages) is now visible through
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>a web-based interface</a>.
This may be useful for people maintaining web-site translations.
"),

array("November 26, 2003",
"A <a href=http://boinc.berkeley.edu/talks/madrid_03/madrid.html>paper
about the goals of BOINC</a> was presented at a recent conference.
"),

array("November 25, 2003",
"The beta test project is accepting new users.
CPU time measurement accuracy improved.
Scheduler respects disk usage limits.
Web site can issue cookies.
"),

array("November 5, 2003",
"The graphics of running application are updated whenever
preferences are changed.
The scheduler now issues only as many results as will fit in
the allotted amount of disk space.

"),

array("October 30, 2003",
"New minor versions of the core client and beta-test apps have been released.
XML statistics data for the beta test
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats.php>is available</a>;
it's intended for graphical or tabular representation.
"),

array("October 18, 2003",
"A SETI@home application has been added to the beta test.
The scheduling server detect results that can't be sent
to any hosts, and flags them.
HTTP 404 errors on downloads are treated as unrecoverable.
Graphics can be limited by frame rate or percent of CPU.
Don't send two results from the same workunit to a single user.
Added JPEG support to application graphics.
Added \"confirm before accept executable\" mechanism.

"),

array("September 11, 2003",
"BOINC is discussed in
<a href=http://www.nytimes.com/2003/09/11/technology/circuits/11dist.html>
an article by Joan Oleck in the New York TImes</a>.
"),


array("September 4, 2003",
"We've released BOINC version 2.0.
All BOINC components must be upgraded to major version 2.
We've upgraded the <a
href=http://maggie.ssl.Berkeley.edu/ap/>Astropulse</a> server to 2.01 and
released <a href=http://setiboinc.ssl.Berkeley.edu/ap/download.php>BOINC core
client 2.01</a>.
This was due to an architecture and protocol change in
specifying resource estimates and limits.
"),

array("September 1, 2003",
"BOINC server tools are now Python-based.  A <a href=python.php>database
back-end API</a> allows quick development of data-manipulation utilities.
"),

array("August 15, 2003",
"We rewrote parts of the scheduling server architecture to make operations
more flexible and efficient.
The <code>timeout_check</code>
daemon has been replaced by a more general <code>transitioner</code> daemon.
"),

array("July 25, 2003",
"We've added two new web-site features for BOINC projects:
user profiles and message boards.
These are visible on the <a href=http://maggie.ssl.berkeley.edu/ap/>beta-test web site</a>.
"),

array("July 23, 2003",
"The <a href=source/>BOINC source code</a> is available again,
under <a href=legal.html>a new public license</a>.
Version 1.05 incorporates bug fixes and new features,
including the ability to add your own graphics to the screensaver.
The beta test has been resumed, and the scheduling server and
database have been moved to a new machine.
"),

array("June 10, 2003",
"<a href=http://boinc.berkeley.edu/db_dump.php>XML based statistics</a> for
the Astropulse beta test are
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats/>available</a>.
"),

array("May 30, 2003",
"No big news to report,
but thanks to the beta test we've fixed a number of bugs,
involving runaway applications, too-large output files,
screensaver behavior, and mechanisms for
quitting and resetting projects.
"),

array("April 29, 2003",
"We have created a second project, allowing beta testers to
experiment with dividing their resources between multiple projects.
"),

array("April 23, 2003",
"We have resumed the BOINC beta test.
"),

array("April 9, 2003",
"Due to a legal issue,
we are suspending the BOINC beta test,
and we have requested that BOINC source code
no longer be available at Sourceforge.net.
We hope to resolve this issue soon.
"),

array("March 31, 2003",
"We are preparing a BOINC-based version of SETI@home.
See a <a href=setiathome.jpg>preview of the graphics</a>.
"),

array("March 25, 2003",
"<a href=http://www.boinc.dk/index.php?page=download_languages>Non-English
language.ini files</a> are available.
Preferences include time-of-day restrictions.
Core client and applications communicate via shared memory and signals
rather than files, reducing disk traffic.
"),

array("March 19, 2003",
"New account parameters and preferences: URL, limit number of processors,
frequency of writes to disk, whether to show your computers on the web.
"),

array("March 4, 2003",
"Participants can have separate preferences
(such as network and disk usage limits)
for computers at home, work, and school.
"),

array("February 25, 2003",
"Participants can limit their upstream and downstream network bandwidth.
"),

array("February 22, 2003",
"Participants can control the parameters (color, transparency,
timing) of the Astropulse graphics.
"),

array("February 19, 2003",
"New feature: secure, verified email address update.
"),

array("January 29, 2003",
"A <a href=http://setiathome.berkeley.edu/~eheien/ap_ss.jpg>screenshot</a> of the
BOINC client running AstroPulse, our first test application.
"),

array("December 10, 2002",
"We have started a beta test of BOINC using
the Astropulse application.
Many bugs have been found and fixed.
"),

array("August 24, 2002",
"BOINC is under development.
The basic features are working on UNIX platforms.
We plan to release the first public application of BOINC later this year.
")
);

?>
