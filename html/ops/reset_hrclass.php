<?php

// change all WUs committed to a given HR class back to uncommitted
//
// TODO: document - when/why would you want to do this?
// TODO: use new DB interface

include_once( "../inc/db.inc" );
include_once( "../inc/util.inc" );
include_once( "../inc/db_ops.inc" );
include_once( "../inc/util_ops.inc" );
include_once( "../inc/prefs.inc" );

db_init();

if (get_int('hr_class')) {
    $hr_class = get_int('hr_class');
} else {
    $hr_class = 0;
}

$timestr = time_str(time(0));
$title = "hr_class ".$hr_class." reset at ".$timestr;

admin_page_head($title);

if ($hr_class != 0) {
    $result = mysql_query("UPDATE workunit SET hr_class=0 WHERE hr_class=".$hr_class);
}

echo $title;

admin_page_tail();

?>
