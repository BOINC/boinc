CREATE TABLE q_list (
    id          integer NOT NULL auto_increment,
    user        integer NOT NULL default '0',
    workunit    integer NOT NULL default '0',
    PRIMARY KEY  (id)
) TYPE=MyISAM;

CREATE TABLE q_restricted_apps (
    id          integer NOT NULL auto_increment,
    appid       integer NOT NULL default '0',
    PRIMARY KEY  (id)
) TYPE=MyISAM;

CREATE TABLE q_users (
    id          integer NOT NULL auto_increment,
    user        integer NOT NULL default '0',
    app         integer NOT NULL default '0',
    qmax        integer NOT NULL default '0',
    PRIMARY KEY  (id)
) TYPE=MyISAM;
