#! /usr/local/bin/php
<?php

// code for one-time database updates goes here.
// Don't run this unless you know what you're doing!

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function update_4_18_2004() {
    db_init();
    mysql_query("alter table user add cross_project_id varchar(254) not null");
    $result = mysql_query("select * from user");
    while ($user = mysql_fetch_object($result)) {
        $x = random_string();
        mysql_query("update user set cross_project_id='$x' where id=$user->id");
    }
}

function update_5_12_2004() {
    db_init();
    mysql_query(
        "create table trickle_up (
        id                  integer     not null auto_increment,
        create_time         integer     not null,
        send_time           integer     not null,
        resultid            integer     not null,
        appid               integer     not null,
        hostid              integer     not null,
        handled             smallint    not null,
        xml                 text,
        primary key (id)
        )"
    );
    mysql_query(
        "create table trickle_down (
        id                  integer     not null auto_increment,
        create_time         integer     not null,
        resultid            integer     not null,
        hostid              integer     not null,
        handled             smallint    not null,
        xml                 text,
        primary key (id)
        )"
    );
    mysql_query(
        "alter table trickle_up add index trickle_handled (appid, handled)"
    );
    mysql_query(
        "alter table trickle_down add index trickle_host(hostid, handled)"
    );
}

update_5_12_2004();

?>
