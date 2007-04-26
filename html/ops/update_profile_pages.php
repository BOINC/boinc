#!/usr/bin/env php
<?php

require_once("../inc/db.inc");
require_once("../inc/gallery.inc");
require_once("../inc/profile.inc");

echo date(DATE_RFC822), ": Starting\n";

set_time_limit(0);

db_init();

build_country_pages();
build_alpha_pages();
build_picture_pages(GALLERY_WIDTH, GALLERY_HEIGHT);

echo date(DATE_RFC822), ": Finished\n";
?>
