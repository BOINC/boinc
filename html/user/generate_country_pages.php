<?php
require_once("gallery.inc");
require_once("util.inc");
require_once("db.inc");

db_init();

page_head("Generate Profile Gallery");

build_country_pages();

page_tail();

?>