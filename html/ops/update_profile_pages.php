#!/usr/local/bin/php

<?php

require_once("../inc/db.inc");
require_once("../inc/gallery.inc");
require_once("../inc/profile.inc");

db_init();

build_country_pages();
build_alpha_pages();
build_picture_pages(GALLERY_WIDTH, GALLERY_HEIGHT);

build_uotd_page();
?>
