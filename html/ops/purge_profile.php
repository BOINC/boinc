<?php

// Script to help you purge spam profiles.
//
// To use: do the following query from mysql:
//
// select name, id  from user, profile where user.id=profile.userid and match(response1, response2) against ('Viagra');
// (replace "Viagra" with other keywords)
//
// Then copy the ids into the array below and run this script

require_once("../inc/db.inc");
db_init();

$ids = array(
        9031517,
        9031518,
);

foreach ($ids as $id) {
    mysql_query("delete from user where id=$id");
    mysql_query("delete from profile where userid=$id");
}

?>
