#! /usr/local/bin/php
<?php

// code for one-time database updates goes here.
// Don't run this unless you know what you're doing!

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

set_time_limit(0);

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

function update_10_25_2004() {
    mysql_query(
        "alter table forum_preferences add rated_posts varchar(254) not null"
    );
    mysql_query(
        "alter table forum_preferences add low_rating_threshold integer not null"
    );
    mysql_query(
        "alter table forum_preferences add high_rating_threshold integer not null"
    );
}

function update_10_26_2004() {
    mysql_query("alter table forum_preferences modify jump_to_unread tinyint(1) unsigned not null default 0");
}

function update_11_24_2004() {
    mysql_query(
        "alter table workunit change workseq_next hr_class integer not null"
    );
    mysql_query(
        "alter table workunit add priority integer not null"
    );
    mysql_query(
        "alter table workunit add mod_time timestamp"
    );
    mysql_query(
        "alter table result add priority integer not null"
    );
    mysql_query(
        "alter table result add mod_time timestamp"
    );
    mysql_query(
        "alter table host drop column projects"
    );
    mysql_query(
        "alter table host add avg_turnaround double not null"
    );
    mysql_query(
        "alter table result drop index ind_res_st"
    );
    mysql_query(
        "alter table result add index ind_res_st(server_state, priority)"
    );
    mysql_query(
        "alter table result drop index app_received_time"
    );
    mysql_query(
        "alter table result add index app_mod_time(appid, mod_time desc)"
    );
}

// or alternatively: (can run in parallel)

function update_11_24_2004_result() {
    mysql_query(
        "alter table result add priority integer not null, "
        ."add mod_time timestamp, "
        ."drop index ind_res_st, "
        ."add index ind_res_st(server_state, priority), "
        ."drop index app_received_time, "
        ."add index app_mod_time(appid, mod_time desc)"
    );
}
function update_11_24_2004_workunit() {
    mysql_query(
        "alter table workunit "
        ." change workseq_next hr_class integer not null, "
        ." add priority integer not null, "
        ." add mod_time timestamp"
    );
}
function update_11_24_2004_host() {
    mysql_query(
        "alter table host drop column projects, "
        ." add avg_turnaround double not null"
    );
}

function update_12_27_2004() {
    mysql_query("alter table workunit drop index wu_filedel");
    mysql_query("alter table workunit add index wu_filedel (file_delete_state, mod_time)");
}

function update_1_3_2005() {
    mysql_query("alter table workunit drop index wu_filedel");
    mysql_query("alter table workunit add index wu_filedel (file_delete_state)");
    mysql_query("alter table result drop index app_mod_time");
}

function update_1_7_2005() {
    mysql_query("alter table forum_preferences add ignorelist varchar(254) not null");
}

function update_1_13_2005() {
    mysql_query("alter table thread add hidden integer not null");
    mysql_query("alter table post add hidden integer not null");
}

function update_1_18_2005() {
    mysql_query("ALTER TABLE `forum_preferences` CHANGE `special_user` `special_user` CHAR( 8 ) DEFAULT '0' NOT NULL");
}

function update_1_19_2005() {
    mysql_query("create table tentative_user (
        nonce               varchar(254) not null,
        email_addr          varchar(254) not null,
        confirmed           integer not null,
        primary key(nonce)
        );"
    );
}

//update_10_25_2004();

?>
