
CREATE TABLE `app` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `name` varchar(254) NOT NULL default '',
  `min_version` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) TYPE=MyISAM;

CREATE TABLE `app_version` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `appid` int(11) NOT NULL default '0',
  `version_num` int(11) NOT NULL default '0',
  `platformid` int(11) NOT NULL default '0',
  `xml_doc` blob,
  `min_core_version` int(11) NOT NULL default '0',
  `max_core_version` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `appid` (`appid`,`platformid`,`version_num`)
) TYPE=MyISAM;

CREATE TABLE `category` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `orderID` int(10) unsigned NOT NULL default '0',
  `lang` int(10) unsigned NOT NULL default '0',
  `name` varchar(255) binary NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `langID` (`lang`,`orderID`)
) TYPE=MyISAM;

CREATE TABLE `core_version` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `version_num` int(11) NOT NULL default '0',
  `platformid` int(11) NOT NULL default '0',
  `xml_doc` blob,
  `message` varchar(254) default NULL,
  `deprecated` smallint(6) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `forum` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `category` int(10) unsigned NOT NULL default '0',
  `orderID` int(10) unsigned NOT NULL default '0',
  `title` varchar(254) NOT NULL default '',
  `description` varchar(254) NOT NULL default '',
  `timestamp` int(10) unsigned NOT NULL default '0',
  `threads` int(10) unsigned NOT NULL default '0',
  `posts` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `orderID` (`orderID`,`category`)
) TYPE=MyISAM;

CREATE TABLE `host` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `userid` int(11) NOT NULL default '0',
  `rpc_seqno` int(11) NOT NULL default '0',
  `rpc_time` int(11) NOT NULL default '0',
  `total_credit` double NOT NULL default '0',
  `expavg_credit` double NOT NULL default '0',
  `expavg_time` double NOT NULL default '0',
  `timezone` int(11) NOT NULL default '0',
  `domain_name` varchar(254) default NULL,
  `serialnum` varchar(254) default NULL,
  `last_ip_addr` varchar(254) default NULL,
  `nsame_ip_addr` int(11) NOT NULL default '0',
  `on_frac` double NOT NULL default '0',
  `connected_frac` double NOT NULL default '0',
  `active_frac` double NOT NULL default '0',
  `p_ncpus` int(11) NOT NULL default '0',
  `p_vendor` varchar(254) default NULL,
  `p_model` varchar(254) default NULL,
  `p_fpops` double NOT NULL default '0',
  `p_iops` double NOT NULL default '0',
  `p_membw` double NOT NULL default '0',
  `os_name` varchar(254) default NULL,
  `os_version` varchar(254) default NULL,
  `m_nbytes` double NOT NULL default '0',
  `m_cache` double NOT NULL default '0',
  `m_swap` double NOT NULL default '0',
  `d_total` double NOT NULL default '0',
  `d_free` double NOT NULL default '0',
  `d_boinc_used_total` double NOT NULL default '0',
  `d_boinc_used_project` double NOT NULL default '0',
  `d_boinc_max` double NOT NULL default '0',
  `n_bwup` double NOT NULL default '0',
  `n_bwdown` double NOT NULL default '0',
  `credit_per_cpu_sec` double NOT NULL default '0',
  `venue` varchar(254) NOT NULL default '',
  `projects` blob,
  PRIMARY KEY  (`id`),
  KEY `host_user` (`userid`),
  KEY `host_avg` (`expavg_credit`),
  KEY `host_tot` (`total_credit`)
) TYPE=MyISAM;

CREATE TABLE `lang` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(254) NOT NULL default '',
  `charset` varchar(254) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `platform` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `name` varchar(254) NOT NULL default '',
  `user_friendly_name` varchar(254) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) TYPE=MyISAM;

CREATE TABLE `post` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `thread` int(10) unsigned NOT NULL default '0',
  `user` int(10) unsigned NOT NULL default '0',
  `timestamp` int(10) unsigned NOT NULL default '0',
  `content` text NOT NULL,
  `modified` int(10) unsigned default NULL,
  `parent_post` int(10) unsigned default NULL,
  PRIMARY KEY  (`id`),
  KEY `threadID` (`thread`),
  KEY `userID` (`user`),
  FULLTEXT KEY `content` (`content`)
) TYPE=MyISAM;

CREATE TABLE `profile` (
  `userid` int(11) NOT NULL default '0',
  `language` varchar(30) default NULL,
  `response1` text,
  `response2` text,
  `has_picture` tinyint(1) NOT NULL default '0',
  `recommend` int(11) NOT NULL default '0',
  `reject` int(11) NOT NULL default '0',
  `posts` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`userid`)
) TYPE=MyISAM;

CREATE TABLE `project` (
  `id` int(11) NOT NULL auto_increment,
  `short_name` varchar(254) NOT NULL default '',
  `long_name` varchar(254) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `result` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `workunitid` int(11) NOT NULL default '0',
  `server_state` int(11) NOT NULL default '0',
  `outcome` int(11) NOT NULL default '0',
  `client_state` int(11) NOT NULL default '0',
  `hostid` int(11) NOT NULL default '0',
  `report_deadline` int(11) NOT NULL default '0',
  `sent_time` int(11) NOT NULL default '0',
  `received_time` int(11) NOT NULL default '0',
  `name` varchar(254) NOT NULL default '',
  `cpu_time` double NOT NULL default '0',
  `xml_doc_in` blob,
  `xml_doc_out` blob,
  `stderr_out` blob,
  `batch` int(11) NOT NULL default '0',
  `file_delete_state` int(11) NOT NULL default '0',
  `validate_state` int(11) NOT NULL default '0',
  `claimed_credit` double NOT NULL default '0',
  `granted_credit` double NOT NULL default '0',
  `opaque` int(11) NOT NULL default '0',
  `random` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `res_wuid` (`workunitid`),
  KEY `ind_res_st` (`server_state`,`random`),
  KEY `res_filedel` (`file_delete_state`),
  KEY `res_hostid` (`hostid`),
  KEY `received_time` (`received_time`)
) TYPE=MyISAM;

CREATE TABLE `team` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `userid` int(11) NOT NULL default '0',
  `name` varchar(254) NOT NULL default '',
  `name_lc` varchar(254) default NULL,
  `url` varchar(254) default NULL,
  `type` int(11) NOT NULL default '0',
  `name_html` varchar(254) default NULL,
  `description` blob,
  `nusers` int(11) NOT NULL default '0',
  `country` varchar(254) default NULL,
  `total_credit` double NOT NULL default '0',
  `expavg_credit` double NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `team_avg` (`expavg_credit`),
  KEY `team_tot` (`total_credit`)
) TYPE=MyISAM;

CREATE TABLE `thread` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `forum` int(10) unsigned NOT NULL default '0',
  `owner` int(10) unsigned NOT NULL default '0',
  `title` varchar(254) NOT NULL default '',
  `timestamp` int(10) unsigned NOT NULL default '0',
  `views` int(10) unsigned NOT NULL default '0',
  `replies` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `forumID` (`forum`)
) TYPE=MyISAM;

CREATE TABLE `user` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `email_addr` varchar(254) NOT NULL default '',
  `name` varchar(254) default NULL,
  `authenticator` varchar(254) default NULL,
  `country` varchar(254) default NULL,
  `postal_code` varchar(254) default NULL,
  `total_credit` double NOT NULL default '0',
  `expavg_credit` double NOT NULL default '0',
  `expavg_time` double NOT NULL default '0',
  `global_prefs` blob,
  `project_prefs` blob,
  `teamid` int(11) NOT NULL default '0',
  `venue` varchar(254) NOT NULL default '',
  `url` varchar(254) default NULL,
  `send_email` smallint(6) NOT NULL default '0',
  `show_hosts` smallint(6) NOT NULL default '0',
  `posts` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `email_addr` (`email_addr`),
  UNIQUE KEY `authenticator` (`authenticator`),
  KEY `ind_tid` (`teamid`),
  KEY `user_tot` (`total_credit`),
  KEY `user_avg` (`expavg_credit`)
) TYPE=MyISAM;

CREATE TABLE `workseq` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `state` int(11) NOT NULL default '0',
  `hostid` int(11) NOT NULL default '0',
  `wuid_last_done` int(11) NOT NULL default '0',
  `wuid_last_sent` int(11) NOT NULL default '0',
  `workseqid_master` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `workunit` (
  `id` int(11) NOT NULL auto_increment,
  `create_time` int(11) NOT NULL default '0',
  `appid` int(11) NOT NULL default '0',
  `name` varchar(254) NOT NULL default '',
  `xml_doc` blob,
  `batch` int(11) NOT NULL default '0',
  `rsc_fpops` double NOT NULL default '0',
  `rsc_iops` double NOT NULL default '0',
  `rsc_memory` double NOT NULL default '0',
  `rsc_disk` double NOT NULL default '0',
  `need_validate` smallint(6) NOT NULL default '0',
  `canonical_resultid` int(11) NOT NULL default '0',
  `canonical_credit` double NOT NULL default '0',
  `timeout_check_time` int(11) NOT NULL default '0',
  `delay_bound` int(11) NOT NULL default '0',
  `error_mask` int(11) NOT NULL default '0',
  `file_delete_state` int(11) NOT NULL default '0',
  `assimilate_state` int(11) NOT NULL default '0',
  `workseq_next` int(11) NOT NULL default '0',
  `opaque` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `wu_val` (`appid`,`need_validate`),
  KEY `wu_timeout` (`appid`,`timeout_check_time`),
  KEY `wu_filedel` (`file_delete_state`),
  KEY `wu_assim` (`appid`,`assimilate_state`)
) TYPE=MyISAM;
