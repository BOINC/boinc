<?php

require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../project/project_news.inc");

page_head("News archive");

show_old_news($project_news, 0);

page_tail();

?>
