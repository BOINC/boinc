<?php

// show summary of results that have been received or timed out recently

require_once("../inc/util_ops.inc");

db_init();
page_head("Result summary");

show_result_summary();

page_tail();
?>
