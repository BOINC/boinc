<?php
/**
 * The responsibility of this file is to include the classes used on every page
 * since PHP4 does not allow autoincluding. So far this is done by hardcoding
 * each include.
 **/

include("../inc/forum_mainfactory.inc");
include("../inc/forum_category.inc");
include("../inc/forum_forum.inc");
include("../inc/forum_thread.inc");
include("../inc/forum_post.inc");
include("../inc/forum_user.inc");
require_once("../inc/util.inc");

?>
