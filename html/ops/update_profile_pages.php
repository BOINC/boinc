#!/usr/local/bin/php

<?php

require_once("../html_user/db.inc");
require_once("gallery.inc");
require_once("../html_user/profile.inc");

db_init();

build_country_pages();
build_alpha_pages();
build_picture_pages(GALLERY_WIDTH, GALLERY_HEIGHT);

?>
