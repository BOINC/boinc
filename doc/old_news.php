<?php

require_once("docutil.php");
require_once("../html/inc/news.inc");
require_once("boinc_news.php");

page_head("News archive");

show_old_news($project_news, 6);

page_tail();

?>
