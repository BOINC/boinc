<?php
require_once("gallery.inc");
require_once("profile.inc");
require_once("util.inc");
require_once("db.inc");

db_init();

page_head("Generate Profile Gallery");

build_picture_pages(GALLERY_WIDTH, GALLERY_HEIGHT);

page_tail();

?>