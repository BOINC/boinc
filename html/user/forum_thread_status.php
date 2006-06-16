<?php
/**
 * This file serves to change the status of a thread
 * A thread can be either unsolved or solved (used in the Q&A part)
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

$threadid = get_int('id');
$thread = new Thread($threadid);
$logged_in_user = re_get_logged_in_user();

if ($logged_in_user->getID()==$thread->getOwner()->getID()){
    if (!$thread->setStatus(THREAD_SOLVED)){
	error_page("Could not update the status of the thread: ".$thread->getID());
    }
} else {
    error_page("You must be the owner of the thread to do this");
}

// --------------

page_head("Status of the thread");
echo "<p>The status has been updated. Thank you!</p>";
echo "<p><a href=\"forum_thread.php?nowrap=true&id=".$thread->getID()."\">Return to the thread</a></p>";
page_tail();
?>
