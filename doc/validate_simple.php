<?php

require_once("docutil.php");
page_head("Simple validator framework");
echo "
To create a validator using the simple framework,
you must supply three functions:
"; block_start(); echo "
extern int init_result(RESULT& result, void*& data);
"; block_end(); echo "
This takes a result, reads its output file(s),
parses them into a memory structure,
and returns (via the 'data' argument) a pointer to this structure.
It returns:
<ul>
<li> Zero on success,
<li> ERR_OPENDIR if there was a transient error,
e.g. the output file is on a network volume that is not available.
The validator will try this result again later.
<li> Any other return value indicates a permanent error.
Example: an output file is missing, or has a syntax error.
The result will be marked as invalid.
</ul>
"; block_start(); echo "
extern int compare_results(
    RESULT& r1, void* data`, RESULT& r2, void* data2, bool& match
);
"; block_end(); echo "
This takes two results and their associated memory structures.
It returns (via the 'match' argument) true if the two results
are equivalent (within the tolerances of the application).
"; block_start(); echo "
extern int cleanup_result(RESULT& r, void* data);
"; block_end(); echo "
This frees the structure pointed to by data, if it's non-NULL.
<p>
You must link these functions with the files
validator.C, validate_util.C, and validate_util2.C.
The result is your custom validator.

<h3>Example</h3>
Here's an example in which the output file
contains an integer and a double.
Two results are considered equivalent if the integer is equal
and the doubles differ by no more than 0.01.
<p>
This example uses utility functions
get_output_file_path() and try_fopen(),
which are documented <a href=backend_util.php>here</a>.
"; block_start(); echo "
struct DATA {
    int i;
    double x;
};

int init_result(RESULT& result, void*& data) {
    FILE* f;
    string path;
    int i, n, retval;
    double x;

    retval = get_output_file_path(result, path);
    if (retval) return retval;
    retval = try_fopen(path.c_str(), f, \"r\");
    if (retval) return retval;
    n = fscanf(f, \"%d %f\", &i, &x);
    if (n != 2) return ERR_XML_PARSE;
    DATA* dp = new DATA;
    dp->i = i;
    dp->x = x;
    data = (void*) dp;
    return 0;
}

int compare_results2(
    RESULT& r1, void* _data1, RESULT& r2, void* _data2, bool& match
) {
    DATA* data1 = (DATA*)_data1;
    DATA* data2 = (DATA*)_data2;
    match = true;
    if (data1->i != data2->i) match = false;
    if (fabs(data1->x - data2->x) > 0.01) match = false;
    return 0;
}

int cleanup_result(RESULT& r, void* data) {
    if (data) free(data);
    return 0;
}

"; block_end(); echo "
";
page_tail();
?>
