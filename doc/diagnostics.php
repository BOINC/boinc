<?php
require_once("docutil.php");
page_head("The BOINC diagnostics API");
echo"
BOINC applications can call
<pre>
int boinc_init_diagnostics(int flags)
</pre>
to initialize various diagnostic functions.
<b>This call should be made early in the program - before boinc_init() -
so that error info is routed appopriately.</b>
<code>flags</code> is formed by or'ing together
a subset of the following flags:
<pre>
#define BOINC_DIAG_DUMPCALLSTACKENABLED     0x00000001L
#define BOINC_DIAG_HEAPCHECKENABLED         0x00000002L
#define BOINC_DIAG_MEMORYLEAKCHECKENABLED   0x00000004L
#define BOINC_DIAG_ARCHIVESTDERR            0x00000008L
#define BOINC_DIAG_ARCHIVESTDOUT            0x00000010L
#define BOINC_DIAG_REDIRECTSTDERR           0x00000020L
#define BOINC_DIAG_REDIRECTSTDOUT           0x00000040L
#define BOINC_DIAG_REDIRECTSTDERROVERWRITE  0x00000080L
#define BOINC_DIAG_REDIRECTSTDOUTOVERWRITE  0x00000100L
#define BOINC_DIAG_TRACETOSTDERR            0x00000200L
#define BOINC_DIAG_TRACETOSTDOUT            0x00000400L
</pre>
The flags are as follows.
Applications are advised to use at least
BOINC_DIAG_DUMPCALLSTACKENABLED,
BOINC_DIAG_REDIRECTSTDERR,
and BOINC_DIAG_TRACETOSTDERR.
";
list_start();
list_item(
    "BOINC_DIAG_DUMPCALLSTACKENABLED",
    "If the application crashes, write a symbolic call stack to stderr.
    If you use this in a Windows app,
    you must include lib/stackwalker_win.cpp in the compile,
    and you must include the .pdb (symbol) file in your app version bundle."
);
list_item(
    "BOINC_DIAG_HEAPCHECKENABLED",
    "Check the integrity of the malloc heap every N allocations.
    (N is line 120 in diagnostics.C; default 1024)."
);
list_item(
    "BOINC_DIAG_MEMORYLEAKCHECKENABLED",
    "When process exits, write descriptions of any outstanding
    memory allocations to stderr."
);
list_item(
    "BOINC_DIAG_ARCHIVESTDERR",
    "Rename stderr.txt to stderr.old on startup."
);
list_item(
    "BOINC_DIAG_ARCHIVESTDOUT",
    "Rename stdout.txt to stdout.old on startup."
);
list_item(
    "BOINC_DIAG_REDIRECTSTDERR",
    "Redirect stderr to stderr.txt."
);
list_item(
    "BOINC_DIAG_REDIRECTSTDOUT",
    "Redirect stdout to stdout.txt."
);
list_item(
    "BOINC_DIAG_REDIRECTSTDERROVERWRITE",
    "Overwrite stderr.txt (default is to append)."
);
list_item(
    "BOINC_DIAG_REDIRECTSTDOUTOVERWRITE",
    "Overwrite stdout.txt (default is to append)."
);
list_item(
    "BOINC_DIAG_TRACETOSTDERR",
    "Write TRACE macros to stderr (Windows specific)."
);
list_item(
    "BOINC_DIAG_TRACETOSTDOUT",
    "Write TRACE macros to stdout (Windows specific)."
);
list_end();
page_tail();
?>
