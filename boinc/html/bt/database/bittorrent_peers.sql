CREATE TABLE `bittorrent_peers` (
    `fileid` int(11) NOT NULL default '0',
    `peerid` varbinary(20) NOT NULL,
    `ip` varbinary(50) default NULL,
    `port` int(11) NOT NULL,
    `status` enum('started','stopped','completed') NOT NULL,
    `uploaded` int(11) NOT NULL,
    `downloaded` int(11) NOT NULL,
    `timestamp` int(14) NOT NULL,
    PRIMARY KEY  (`fileid`,`peerid`),
    KEY `timestamp` (`timestamp`)
) ENGINE=MEMORY;
	        