<?php

/**
 * This file can define optional callback functions that will be used
 * to add project-specific content the website.
 *
 * - To enable a particular callback you should uncomment the function in
 *   this file.
 * - Callbacks should prefferably not write directly to the output stream,
 *   they should instead return the text/data if possible.
 **/
 

/*function project_workunit_callback($wu){
    return "This is displayed on the workunit page";
}/**/

/*function project_user_summary_callback($user){
    return "Text here will go to the user summary page";
}/**/

/*function project_user_page_private_callback($user){
    return "Whatever is here is added to the private account page";
}/**/

function project_user_credit_callback($user){
    if ($user->seti_nresults) {
        row2(
            "SETI@home classic workunits
            <br><font size=-2>as of 14 May 2004</font>",
            number_format($user->seti_nresults)
        );
    }
    if ($user->seti_total_cpu) {
        $x = number_format($user->seti_total_cpu/3600)." hours";
        row2(
            "SETI@home classic CPU time
            <br><font size=-2>as of 14 May 2004</font>",
            $x
        );
    }
    return "";
}

?>