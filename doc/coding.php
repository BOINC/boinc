<?php
require_once("docutil.php");
page_head("BOINC coding style");
echo "
<h2> All languages</h2>
<h3> Code factoring</h3>
<ul>
    <li> if code is repeated, factor it out and make it into a function
    <li> if a function becomes longer than 100 lines or so, split it up
    <li> C++ .h files often contain both interface and implementation.
        Clearly divide these.
</ul>

<h3> code documentation</h3>
<ul>
    <li> every .C file has a comment at the top saying what's
        in the file (and perhaps what isn't).
        If a file is becoming 'landfill', split it up.
    <li> every function is preceded by a comment saying what it does
    <li> every struct/class is preceded by a comment saying what it is
</ul>

<h3> Naming</h3>
<ul>
    <li> Names should be descriptive without being verbose
        (local variables names may be short)
    <li> Class names, #defined symbols, and type names are all upper case,
        with underscores to separate words.
    <li> Variable and function names are all lower case,
        with underscores to separate words.
    <li> no mixed case names
</ul>

<h3> Indentation</h3>
<ul>
    <li> Each level of indentation is 4 spaces (not a tab).
    <li> multi-line function call:
        <pre>
        func(
            blah, blah, blah, blah, blah,
            blah, blah, blah, blah, blah
        );
        </pre>
    <li> switch statements: case labels are at same indent level as switch
        <pre>
        switch (foo) {
        case 1:
            ...
            break;
        case 2:
            ...
            break;
        }
        </pre>
</ul>

<h3> Constants</h3>
<ul>
    There should be few numeric constants in code.
    Generally they should be #defines
</ul>

<h3> Braces</h3>
<ul>
    <li> Opening curly brace goes at end of line (not next line):
    <pre>
        if (foobar) {
            ...
        } else if (blah) {
            ...
        } else {
            ...
        }
    </pre>
    <li> always use curly braces on multi-line if statements
    <pre>
        if (foo)
            return blah;        // WRONG
    </pre>
    <li> 1-line if() statments are OK:
    <pre>
        if (foo) return blah;
    </pre>
</ul>

<h3> comments and #ifdefs</h3>
<ul>
    <li> use // for all comments
    <li> comment out blocks of code as follows:
    <pre>
        #if 0
            ...
        #endif
    </pre>
</ul>

<h2> C++ specific</h2>
<h3> Includes</h3>
<ul>
    <li> A .C file should have the minimum set of #includes to get
        that particular file to compile
        (e.g. the includes needed by foo.C should be in foo.C, not foo.h)
    <li> Includes should go from general (&lt;stdio.h>) to specific (thisfile.h)
</ul>
<h3> extern declarations</h3>
<ul>
    foo.h should have 'extern' declarations for all public
        functions and variables in foo.C
    There should be no 'extern' statements in .C files.
</ul>

<h3> Use of static</h3>
<ul>
    <li> if a function or variable is used only in 1 file, declare it static.
</ul>

<h3> Things to avoid unless there's a truly compelling reason:</h3>
<ul>
    <li> inline functions
    <li> operator or function overloading
    <li> templates
</ul>
<h3> Things to avoid</h3>
<ul>
    <li> use typedef (not #define) to define types
    <li> don't use memset() or memcpy() to initialize or copy classes
        that are non-C compatible.
        Write a default constructor and a copy constructor.
</ul>
<h3> error codes</h3>
<ul>
    <li> (almost) all functions should return an integer error code.
        Nonzero means error.
    <li> All calls to functions that return an error code should
        check the code.  Generally they should return on error, e.g.:
    <pre>
            retval = blah();
            if (retval) return retval;
    </pre>
</ul>

<h3> structure definitions</h3>
<ul>
    Define structures as follows:
    <pre>

    struct FOO {
        ...
    };
    </pre>

    You can then declare variables as:

    <pre>
    FOO x;
    </pre>
</ul>


<h2> PHP specific</h2>

<h3>Getting POST and GET data</h3>
Remember that hackers can pass arbitrary values in POST and GET,
and they can use this to do SQL injections and other exploits.
<ul>
<li> Do not access \$_POST or \$_GET directly.
<li> Use get_int(), get_str(), post_int() and post_str()
    (from util.inc) to get POST and GET data.
<li> If a POST or GET value will be used in a SQL query,
    use process_user_text() to escape it.
</ul>
";

page_tail();

?>
