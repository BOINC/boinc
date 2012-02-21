-- $Id$

-- Boinc Monitoring schema and stored procedures
-- Re-running this file clears monitoring statistics
-- Only re-run this to add the monitoring mechanism to a plain boinc.
-- After this, re-run the monitoring_functions.sql script 



drop table IF EXISTS monitoring ;

create table monitoring (       
	id              int(11)         not null auto_increment,
	monitor_time    int(11)         not null,
	mod_time        timestamp       NOT NULL 
					default CURRENT_TIMESTAMP 
					on update CURRENT_TIMESTAMP,
	scientist        varchar(100), 
	cur_inprogress   int(11),
	cur_unsent       int(11),
	day_successful   int(11),
	day_unsuccessful int(11),
	day_credits      int(11),
	primary key (id),
	unique key record (monitor_time,scientist),
	key res_scientist (scientist)
) ENGINE=InnoDB;


