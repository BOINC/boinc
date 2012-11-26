
alter table platform
    add unique(name);

alter table app
    add unique(name);

alter table app_version
    add unique apvp (appid, platformid, version_num, plan_class);

alter table user
    add unique(email_addr),
    add unique(authenticator),
    add index ind_tid (teamid),
    add index user_name(name),
    add index user_tot (total_credit desc),
        -- db_dump.C
    add index user_avg (expavg_credit desc);
        -- db_dump.C

alter table team
    add unique(name),
    add fulltext index team_name_desc(name, description),
    add fulltext index team_name(name),
    add index team_avg (expavg_credit desc),
        -- db_dump.C
    add index team_tot (total_credit desc),
        -- db_dump.C
    add index team_userid (userid);

alter table workunit
    add unique(name),
        -- not currently used but good invariant
    add index wu_val (appid, need_validate),
        -- validator
    add index wu_timeout (transition_time),
        -- transitioner
    add index wu_filedel (file_delete_state),
        -- file_deleter, db_purge
    add index wu_assim (appid, assimilate_state);
        -- assimilator

alter table result
    add unique(name),
        -- the scheduler looks up results by name

    add index res_wuid (workunitid),
        -- transitioner
        -- NOTE res_wu_user may suffice, could try dropping this one

    add index ind_res_st (server_state, priority),
        -- feeder

    add index res_app_state(appid, server_state),
        -- to get count of unsent results for given app (e.g. in work generator)

    add index res_filedel (file_delete_state),
        -- file_deleter

    add index res_userid_id(userid, id desc),
        -- html_user/results.php

    add index res_userid_val(userid, validate_state),
        -- to show pending credit

    add index res_hostid_id (hostid, id desc),
        -- html_user/results.php

    add index res_wu_user (workunitid, userid);
        -- scheduler (avoid sending mult results of same WU to one user)

alter table msg_from_host
    add index message_handled (handled);
        -- for message handler

alter table msg_to_host
    add index msg_to_host(hostid, handled);
        -- for scheduler

alter table host
    add index host_user (userid),
        -- html_user/host_user.php
    add index host_avg (expavg_credit desc),
        -- db_dump.C
    add index host_tot (total_credit desc);
        -- db_dump.C

alter table profile
    add fulltext index profile_reponse(response1, response2),
    add index pro_uotd (uotd_time desc),
    add unique profile_userid(userid);

alter table subscriptions
    add unique sub_unique(userid, threadid);

alter table category
    add unique cat1(name, is_helpdesk);

alter table forum
    add unique pct (parent_type, category, title);

alter table thread
    add fulltext index thread_title(title);
        
alter table post
    add index post_user (user),
    add index post_thread (thread),
    add fulltext index post_content(content);

alter table credited_job 
    add index credited_job_user (userid),
    add index credited_job_wu (workunitid),
    add unique credited_job_user_wu (userid, workunitid);

alter table team_delta
    add index team_delta_teamid (teamid, timestamp);

alter table team_admin
    add unique (teamid, userid);

alter table friend
    add unique friend_u (user_src, user_dest);

alter table notify
    add unique notify_un (userid, type, opaque);

alter table host_app_version
    add unique hap(host_id, app_version_id);

alter table assignment
    add index asgn_target(target_type, target_id);

alter table job_file
    add index md5(md5);
