<?php {

    // show summary of results that have been received or timed out recently

    require_once("../inc/util_ops.inc");

    db_init();
    page_head("Result summary by Platform");
    
    show_failure_result_summary_by_platform();

    page_tail();
} ?>
