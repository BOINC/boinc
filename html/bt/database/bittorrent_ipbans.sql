CREATE TABLE `bittorrent_ipbans` (
    `ip` varbinary(256) NOT NULL default '',
    `timestamp` int(14) NOT NULL,
    PRIMARY KEY  (`ip`)
) ENGINE=MEMORY COMMENT='Banned IPs and when they can be unbanned (reset when SQL-server restarts)';