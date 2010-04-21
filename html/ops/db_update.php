#! /usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// code for one-time database updates goes here.
// Don't run this unless you know what you're doing!

$cli_only = true;
require_once("../inc/util_ops.inc");

$db = BoincDb::get_aux(false);
if (!$db) {
    echo "Can't open database\n";
    exit;
}

set_time_limit(0);

function do_query($query) {
    echo "Doing query:\n$query\n";
    $result = mysql_query($query);
    if (!$result) {
        echo "Failed:\n".mysql_error()."\n";
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
    do_query("ALTER TABLE forum_preferences CHANGE special_user special_user CHAR(12) DEFAULT '0' NOT NULL");
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

function update_8_25_2005() {
    do_query("alter table user add donated smallint not null");
}

function update_8_26_2005() {
    do_query("drop table tentative_user");
}

function update_9_22_2005() {
    do_query("update user set country='Macedonia' where country='Macedonia, The Former Yugoslav Republic of'");
}

function update_11_24_2005(){
    do_query("ALTER TABLE `forum_preferences` ADD `minimum_wrap_postcount` INT DEFAULT '100' NOT NULL AFTER `high_rating_threshold` ,
ADD `display_wrap_postcount` INT DEFAULT '75' NOT NULL AFTER `minimum_wrap_postcount`");
}

function update_6_16_2006() {
    do_query("ALTER TABLE `thread` ADD `score` DOUBLE NOT NULL AFTER `sufferers` , ADD `votes` INT NOT NULL AFTER `score`");
    do_query("ALTER TABLE `forum_preferences` ADD `forum_sorting` INT NOT NULL AFTER `sorting`");
    do_query("ALTER TABLE `forum_preferences` ADD `thread_sorting` INT NOT NULL AFTER `forum_sorting`");

    do_query("CREATE TABLE `post_ratings` (
    `post` INT UNSIGNED NOT NULL ,
    `user` INT UNSIGNED NOT NULL ,
    `rating` TINYINT NOT NULL ,
    PRIMARY KEY ( `post` , `user`))");

    do_query("ALTER TABLE `forum_preferences` DROP `avatar_type`");
    do_query("ALTER TABLE `forum_preferences` CHANGE `low_rating_threshold` `low_rating_threshold` INT( 11 ) DEFAULT '-25' NOT NULL");
    do_query("ALTER TABLE `forum_preferences` CHANGE `high_rating_threshold` `high_rating_threshold` INT( 11 ) DEFAULT '5' NOT NULL");
    do_query("ALTER TABLE `forum_preferences` CHANGE `jump_to_unread` `jump_to_unread` TINYINT( 1 ) UNSIGNED DEFAULT '1' NOT NULL");

    do_query("ALTER TABLE `forum_preferences` DROP `sorting`");

    do_query("ALTER TABLE `forum_preferences` CHANGE `no_signature_by_default` `no_signature_by_default` TINYINT( 1 ) UNSIGNED DEFAULT '1' NOT NULL ");
    do_query("ALTER TABLE `thread` ADD `status` SMALLINT UNSIGNED NOT NULL AFTER `owner`");

    do_query("ALTER TABLE `subscriptions` ADD `notified` TINYINT( 1 ) UNSIGNED DEFAULT '0' NOT NULL");
    do_query("ALTER TABLE `subscriptions` CHANGE `notified` `notified_time` INT( 14 ) UNSIGNED DEFAULT '0' NOT NULL");
}

function update_7_11_2006() {
    do_query("alter table app add weight double not null");
}

function update_8_8_2006() {
    do_query("alter table forum_preferences add banished_until integer not null default 0");
}

function update_10_21_2006() {
    do_query("alter table app add beta smallint not null default 0");
}

function update_10_26_2006() {
    do_query("ALTER TABLE `team` ADD `ping_user` INT UNSIGNED NOT NULL DEFAULT '0',
        ADD `ping_time` INT UNSIGNED NOT NULL DEFAULT '0'");
    do_query("ALTER TABLE team ADD INDEX team_userid (userid)");
}

function update_11_10_2006() {
    do_query("ALTER TABLE thread ADD locked TINYINT NOT NULL DEFAULT 0");
}

function update_12_22_2006() {
    do_query("ALTER TABLE forum ADD is_dev_blog TINYINT NOT NULL DEFAULT 0");
}

function update_4_07_2007() {
    do_query('create table sent_email (
            userid              integer     not null,
            time_sent           integer     not null,
            email_type          smallint    not null,
            primary key(userid)
        ) TYPE=MyISAM;'
    );
}

function update_4_24_2007() {
    do_query('alter table host add error_rate double not null default 0');
    
}

function update_4_29_2007() {
    do_query("CREATE TABLE `private_messages` (
            `id`        int(10)     unsigned    NOT NULL auto_increment,
            `userid`    int(10)     unsigned    NOT NULL,
            `senderid`  int(10)     unsigned    NOT NULL,
            `date`      int(10)     unsigned    NOT NULL,
            `opened`    tinyint(1)  unsigned    NOT NULL default '0',
            `subject`   varchar(255)            NOT NULL,
            `content`   text                    NOT NULL,
            PRIMARY KEY  (`id`),
            KEY `userid` (`userid`)
        ) TYPE=MyISAM;"
    );
}

function update_4_30_2007() {
    do_query("create table credited_job (
    userid              integer     not null,
    workunitid          bigint      not null
    ) TYPE=MyISAM;");
    do_query("alter table credited_job add index credited_job_user (userid),
    add index credited_job_wu (workunitid),
    add unique credited_job_user_wu (userid, workunitid);"
    );
}

function update_5_27_2007() {
    do_query("create table donation_items (
    id                  integer     unsigned    not null auto_increment,
    item_name           varchar(32)             not null,
    title               varchar(255)            not null,
    description         varchar(255)            not null,
    required            double      unsigned    not null default '0',
    PRIMARY KEY(id)
) TYPE=MyISAM;");
    do_query("create table donation_paypal (
    id                  integer                 not null auto_increment,
    order_time          integer     unsigned    not null,
    userid              integer                 not null,
    email_addr          varchar(255)            not null,
    order_amount        double(6,2)             not null,
    processed           tinyint(1)              not null default '0',
    payment_time        integer     unsigned    not null,
    item_name           varchar(255)            not null,
    item_number         varchar(255)            not null,
    payment_status      varchar(255)            not null,
    payment_amount      double(6,2)             not null,
    payment_fee         double(5,2)             default null,
    payment_currency    varchar(255)            not null,
    txn_id              varchar(255)            not null,
    receiver_email      varchar(255)            not null,
    payer_email         varchar(255)            not null,
    payer_name          varchar(255)            not null,
    PRIMARY KEY(id)
) TYPE=MyISAM;");
}

function update_6_5_2007() {
    do_query("ALTER TABLE `forum_preferences` ADD `pm_notification` TINYINT( 1 ) UNSIGNED NOT NULL DEFAULT '1';");
}

function update_7_26_2007() {
    do_query("create table team_delta (
    userid              integer                 not null,
    teamid              integer                 not null,
    timestamp           integer                 not null,
    joining             tinyint(1)              not null,
    total_credit        double                  not null
) TYPE=MyISAM;"
    );
    do_query("alter table team_delta
        add index team_delta_teamid (teamid, timestamp);"
    );
}

function update_9_26_2007() {
    // Change field type from unsigned to signed
    do_query("ALTER TABLE team CHANGE ping_user ping_user integer NOT NULL DEFAULT 0");
}

function update_9_28_2007() {
    do_query("alter table team engine=myisam");
    do_query("alter table team change description description text");
    do_query("alter table team add fulltext index team_name_desc(name, description)");
}

function update_10_25_2007() {
    do_query("update user set country='Serbia' where country='Serbia and Montenegro'");
    do_query("update team set country='Serbia' where country='Serbia and Montenegro'");
}

function update_10_26_2007() {
    do_query("create table banishment_vote (
        id                  serial                  primary key,
        userid              integer                 not null,
        modid               integer                 not null,
        start_time          integer                 not null,
        end_time            integer                 not null
        ) TYPE=MyISAM;"
    );
    do_query("create table banishment_votes (
        id                  serial                  primary key,
        voteid              integer                 not null,
        modid               integer                 not null,
        time                integer                 not null,
        yes                 tinyint(1)              not null
        ) TYPE=MyISAM;"
    );
}

function update_11_7_2007() {
    do_query("create table team_admin (
        teamid              integer                 not null,
        userid              integer                 not null,
        create_time         integer                 not null,
        rights              integer                 not null
        ) type=MyISAM;"
    );
    do_query("alter table team_admin add unique (teamid, userid);");
}

function update_11_8_2007() {
    do_query("alter table forum add parent_type integer not null");
}

function update_11_14_2007() {
    do_query("alter table forum drop index category");
    do_query("alter table forum add unique pct (parent_type, category, title)");
}

// pm_notification should be 0 by default.
// We don't know who really wants it to be 1, so set everyone to 0;
// projects might want to run a news item notifying user
// that they need to explicitly set this if they want PM notification
//
function update_11_18_2007() {
    do_query("update forum_preferences set pm_notification=0");
    do_query("alter table forum_preferences change pm_notification pm_notification tinyint not null default 0");
}

function update_11_20_2007() {
    do_query("alter table team add fulltext index team_name(name)");
}

function update_12_18_2007() {
    do_query("create table friend (
        user_src            integer         not null,
        user_dest           integer         not null,
        message             varchar(255)    not null,
        create_time         integer         not null,
        reciprocated        tinyint         not null
        )
    ");
    do_query("create table notify (
        id                  serial          primary key,
        userid              integer         not null,
        create_time         integer         not null,
        type                integer         not null,
        opaque              integer         not null
        )
    ");
    do_query("alter table friend
        add unique friend_u (user_src, user_dest)
    ");
    do_query("alter table notify
        add index notify_u (userid)
    ");
}

function update_12_28_2007() {
    do_query("alter table notify drop index notify_u");
    do_query("alter table notify
        add unique notify_un (userid, type, opaque)
    ");
}

function update_2_18_2008() {
    do_query("create table assignment (
        id                  integer     not null auto_increment,
        create_time         integer     not null,
        target_id           integer     not null,
        target_type         integer     not null,
        multi               tinyint     not null,
        workunitid          integer     not null,
        resultid            integer     not null,
        primary key (id)
        ) engine = InnoDB
    ");
}

// If you haven't done 3_7, skip both of the following:
//
function update_3_7_2008() {
    do_query("alter table workunit add column rsc_bandwidth_bound double not null after rsc_disk_bound");
}
function update_3_7_undo_2008() {
    do_query("alter table workunit drop column rsc_bandwidth_bound");
}

function update_3_10_2008() {
    do_query("alter table workunit add column rsc_bandwidth_bound double not null");
}

function update_3_13_2008() {
    do_query("alter table app_version drop index appid");
    do_query("alter table app_version add column plan_class varchar(254) not null default ''");
    do_query("alter table app_version add unique apvp (appid, platformid, version_num, plan_class)");
}

// The following cleans up from a bug that causes "team transfer pending"
// to be shown even after transfer is finished
//
function update_3_27_2008() {
    do_query("update team set ping_user=0, ping_time=0 where ping_user=userid");
}

function update_3_31_2008() {
    do_query("alter table app_version change column xml_doc xml_doc mediumblob");
}

function update_6_3_2008() {
    do_query("alter table app add target_nresults smallint not null default 0");
}

function update_7_28_2008() {
    do_query("create table credit_multiplier (
        id          serial          primary key,
        appid       integer         not null,
        time        integer         not null,
        multiplier  double          not null default 0
        ) engine=MyISAM
    ");
}

function update_10_05_2008(){
    do_query("alter table forum_preferences add highlight_special tinyint default '1' not null");
}

function update_10_7_2008() {
    do_query("alter table team add joinable tinyint default '1' not null");
}

function update_6_16_2009() {
    do_query("create table state_counts (
            appid               integer     not null, 
            last_update_time    integer     not null,
            result_server_state_2       integer not null,
            result_server_state_4       integer not null,
            result_file_delete_state_1  integer not null,
            result_file_delete_state_2  integer not null,
            result_server_state_5_and_file_delete_state_0       integer not null,
            workunit_need_validate_1    integer not null,
            workunit_assimilate_state_1 integer not null,
            workunit_file_delete_state_1        integer not null,
            workunit_file_delete_state_2        integer not null,
            primary key (appid) 
            ) engine=MyISAM
    ");
}

function update_9_3_2009() {
    do_query("alter table result add (
        elapsed_time double not null,
        flops_estimate double not null,
        app_version_id integer not null
        )
    ");
}

function update_3_5_2010() {
    do_query("alter table workunit add fileset_id integer not null");
}

function update_3_17_2010() {
    do_query("create table host_app_version (
            host_id             integer     not null,
            app_version_id      integer     not null,
            pfc_n               double      not null,
            pfc_avg             double      not null,
            et_n                double      not null,
            et_avg              double      not null,
            et_var              double      not null,
            et_q                double      not null,
            host_scale_time     double      not null,
            scale_probation     tinyint     not null default 1,
            error_rate          double      not null,
            max_jobs_per_day    integer     not null,
            n_jobs_today        integer     not null,
            turnaround_n        double      not null,
            turnaround_avg      double      not null,
            turnaround_var      double      not null,
            turnaround_q        double      not null
        ) engine = InnoDB
    ");
    do_query("alter table host_app_version
        add unique hap(host_id, app_version_id)
    ");
    do_query("alter table app_version
        add pfc_n       double not null default 0,
        add pfc_avg     double not null default 0,
        add pfc_scale   double not null default 0,
        add expavg_credit double not null default 0,
        add expavg_time double not null default 0
    ");
    do_query("alter table app
        add min_avg_pfc double not null,
        add host_scale_check tinyint not null,
        add max_jobs_in_progress integer not null,
        add max_gpu_jobs_in_progress integer not null,
        add max_jobs_per_rpc integer not null,
        add max_jobs_per_day_init integer not null
    ");
}

function update_4_21_2010() {
    do_query("alter table host_app_version
        drop column host_scale_time,
        drop column scale_probation,
        drop column error_rate,
        add column consecutive_valid integer not null
    ");
}

// Updates are done automatically if you use "upgrade".
//
// If you need to do updates manually,
// modify the following to call the function you want.
// Make sure you do all needed functions, in order.
// (Look at your DB structure using "explain" queries to see
// which ones you need).

//update_3_17_2010();

$db_updates = array (
    array(18490, "update_6_16_2009"),
    array(19001, "update_9_3_2009"),
    array(20807, "update_3_5_2010"),
    array(20932, "update_3_17_2010"),
    array(21226, "update_4_21_2010"),
);

?>
