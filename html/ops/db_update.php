#! /usr/local/bin/php
<?php

// code for one-time database updates goes here.
// Don't run this unless you know what you're doing!

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

function update_4_18_2004() {
    mysql_query("alter table user add cross_project_id varchar(254) not null");
    $result = mysql_query("select * from user");
    while ($user = mysql_fetch_object($result)) {
        $x = random_string();
        mysql_query("update user set cross_project_id='$x' where id=$user->id");
    }
}

function update_5_12_2004() {
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

function update_5_27_2004() {
    mysql_query(
        "alter table host add nresults_today integer not null"
    );
}

function update_6_9_2004() {
    mysql_query(
        "alter table profile change verification verification integer not null"
    );
}
function update_6_15_2004() {
    mysql_query(
        "alter table user add index user_name(name)"
    );
}

function update_7_02_2004() {
    mysql_query(
        "alter table workunit drop column result_template"
    );
    mysql_query(
        "alter table workunit add column result_template_file varchar(63) not null"
    );
    mysql_query(
        "update workunit set result_template_file='templates/foo.xml'"
    );
}

function update_7_08_2004() {
    mysql_query(
        "alter table result drop index ind_res_st"
    );
    mysql_query(
        "alter table add index ind_res_st(server_state)"
    );
}

function update_9_04_2004() {
    mysql_query(
        "insert into forum_preferences (userid, signature, posts) select user.id, user.signature, user.posts from user where user.posts > 0 or user.signature<>''");
}

function update_9_05_2004() {
    mysql_query(
        "ALTER TABLE forum_preferences ADD special_user INT NOT NULL"
    );
}

function update_9_26_2004() {
    mysql_query(
        "alter table app add homogeneous_redundancy smallint not null"
    );
}

function update_10_09_2004() {
    mysql_query(
        "alter table forum_preferences add jump_to_unread tinyint(1) unsigned not null default 1"
    );
    mysql_query(
        "alter table forum_preferences add hide_signatures tinyint(1) unsigned not null default 0"
    );
    mysql_query(
        "alter table post add signature tinyint(1) unsigned not null default 0"
    );
}
//update_10_09_2004();

?>
