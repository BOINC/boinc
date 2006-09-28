#!/usr/bin/env php

<?php

require_once("../inc/db.inc");
require_once("../inc/uotd.inc");

db_init();

build_uotd_page();
?>
