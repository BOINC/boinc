CREATE TABLE `bittorrent_files` (
    `id` int(11) NOT NULL auto_increment,
    `filename` varchar(256) default NULL,
    `info_hash` varbinary(22) default NULL,
    `timestamp` int(14) NOT NULL,
    PRIMARY KEY  (`id`),
    UNIQUE KEY `filename` (`filename`),
    KEY `info_hash` (`info_hash`)
) ENGINE=MEMORY;