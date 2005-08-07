#! /usr/local/bin/php
<?php

// code for one-time database updates goes here.
// Don't run this unless you know what you're doing!

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/ops.inc");

cli_only();
db_init_cli();

set_time_limit(0);

function do_query($query) {
    echo "Doing query:\n$query\n";
    $result = mysql_query($query);
    if (!$result) {
        die("Failed!\n".mysql_error());
    } else {
        echo "Success.\n";
    }
}

function update_4_18_2004() {
    do_query("alter table user add cross_project_id varchar(254) not null");
    $result = do_query("select * from user");
    while ($user = mysql_fetch_object($result)) {
        $x = random_string();
        do_query("update user set cross_project_id='$x' where id=$user->id");
    }
}

function update_5_12_2004() {
    do_query(
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
    do_query(
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
    do_query(
        "alter table trickle_up add index trickle_handled (appid, handled)"
    );
    do_query(
        "alter table trickle_down add index trickle_host(hostid, handled)"
    );
}

function update_5_27_2004() {
    do_query(
        "alter table host add nresults_today integer not null"
    );
}

function update_6_9_2004() {
    do_query(
        "alter table profile change verification verification integer not null"
    );
}
function update_6_15_2004() {
    do_query(
        "alter table user add index user_name(name)"
    );
}

function update_7_02_2004() {
    do_query(
        "alter table workunit drop column result_template"
    );
    do_query(
        "alter table workunit add column result_template_file varchar(63) not null"
    );
    do_query(
        "update workunit set result_template_file='templates/foo.xml'"
    );
}

function update_7_08_2004() {
    do_query(
        "alter table result drop index ind_res_st"
    );
    do_query(
        "alter table add index ind_res_st(server_state)"
    );
}

function update_9_04_2004() {
    do_query(
        "insert into forum_preferences (userid, signature, posts) select user.id, user.signature, user.posts from user where user.posts > 0 or user.signature<>''");
}

function update_9_05_2004() {
    do_query(
        "ALTER TABLE forum_preferences ADD special_user INT NOT NULL"
    );
}

function update_9_26_2004() {
    do_query(
        "alter table app add homogeneous_redundancy smallint not null"
    );
}

function update_10_09_2004() {
    do_query(
        "alter table forum_preferences add jump_to_unread tinyint(1) unsigned not null default 1"
    );
    do_query(
        "alter table forum_preferences add hide_signatures tinyint(1) unsigned not null default 0"
    );
    do_query(
        "alter table post add signature tinyint(1) unsigned not null default 0"
    );
}

function update_10_25_2004() {
    do_query(
        "alter table forum_preferences add rated_posts varchar(254) not null"
    );
    do_query(
        "alter table forum_preferences add low_rating_threshold integer not null"
    );
    do_query(
        "alter table forum_preferences add high_rating_threshold integer not null"
    );
}

function update_10_26_2004() {
    do_query("alter table forum_preferences modify jump_to_unread tinyint(1) unsigned not null default 0");
}

function update_11_24_2004() {
    do_query(
        "alter table workunit change workseq_next hr_class integer not null"
    );
    do_query(
        "alter table workunit add priority integer not null"
    );
    do_query(
        "alter table workunit add mod_time timestamp"
    );
    do_query(
        "alter table result add priority integer not null"
    );
    do_query(
        "alter table result add mod_time timestamp"
    );
    do_query(
        "alter table host drop column projects"
    );
    do_query(
        "alter table host add avg_turnaround double not null"
    );
    do_query(
        "alter table result drop index ind_res_st"
    );
    do_query(
        "alter table result add index ind_res_st(server_state, priority)"
    );
    do_query(
        "alter table result drop index app_received_time"
    );
    do_query(
        "alter table result add index app_mod_time(appid, mod_time desc)"
    );
}

// or alternatively: (can run in parallel)

function update_11_24_2004_result() {
    do_query(
        "alter table result add priority integer not null, "
        ."add mod_time timestamp, "
        ."drop index ind_res_st, "
        ."add index ind_res_st(server_state, priority), "
        ."drop index app_received_time, "
        ."add index app_mod_time(appid, mod_time desc)"
    );
}
function update_11_24_2004_workunit() {
    do_query(
        "alter table workunit "
        ." change workseq_next hr_class integer not null, "
        ." add priority integer not null, "
        ." add mod_time timestamp"
    );
}
function update_11_24_2004_host() {
    do_query(
        "alter table host drop column projects, "
        ." add avg_turnaround double not null"
    );
}

function update_12_27_2004() {
    do_query("alter table workunit drop index wu_filedel");
    do_query("alter table workunit add index wu_filedel (file_delete_state, mod_time)");
}

function update_1_3_2005() {
    do_query("alter table workunit drop index wu_filedel");
    do_query("alter table workunit add index wu_filedel (file_delete_state)");
    do_query("alter table result drop index app_mod_time");
}

function update_1_7_2005() {
    do_query("alter table forum_preferences add ignorelist varchar(254) not null");
}

function update_1_13_2005() {
    do_query("alter table thread add hidden integer not null");
    do_query("alter table post add hidden integer not null");
}

function update_1_18_2005() {
    do_query("ALTER TABLE forum_preferences CHANGE special_user special_user CHAR( 8 ) DEFAULT '0' NOT NULL");
}

function update_1_19_2005() {
    do_query("create table tentative_user (
        nonce               varchar(254) not null,
        email_addr          varchar(254) not null,
        confirmed           integer not null,
        primary key(nonce)
        );"
    );
}

function update_1_20_2005() {
    do_query("alter table host add host_cpid varchar(254)");
}

function update_1_20a_2005() {
    do_query("alter table host add external_ip_addr varchar(254)");
}

function update_2_25_2005() {
    do_query("alter table host add max_results_day integer not null");
}

function update_4_20_2005(){
    do_query("ALTER TABLE `thread` ADD `sticky` TINYINT UNSIGNED DEFAULT '0' NOT NULL");
    do_query("ALTER TABLE `forum` ADD `post_min_total_credit` INT NOT NULL AFTER `posts`");
    do_query("ALTER TABLE `forum` ADD `post_min_expavg_credit` INT NOT NULL AFTER `posts`");
    do_query("ALTER TABLE `forum` ADD `post_min_interval` INT NOT NULL AFTER `posts`");
    do_query("ALTER TABLE `forum` ADD `rate_min_total_credit` INT NOT NULL AFTER `posts`");
    do_query("ALTER TABLE `forum` ADD `rate_min_expavg_credit` INT NOT NULL AFTER `posts`");
    do_query("ALTER TABLE `forum_preferences` ADD `last_post` INT( 14 ) UNSIGNED NOT NULL AFTER `posts`");
}

function update_4_30_2005(){
    do_query("ALTER TABLE `forum_preferences` ADD `ignore_sticky_posts` TINYINT( 1 ) UNSIGNED NOT NULL");
}    

function update_6_22_2005() {
    do_query("alter table host add cpu_efficiency double not null after active_frac, add duration_correction_factor double not null after cpu_efficiency");
}

function update_8_05_2005() {
    do_query("alter table user add passwd_hash varchar(254) not null");
    do_query("alter table user add email_validated smallint not null");
    do_query("update user set passwd_hash=MD5(concat(authenticator, email_addr))");
    do_query("update user set email_validated=1");
}

//update_10_25_2004();

?>
