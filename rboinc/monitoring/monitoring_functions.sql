-- $Id$
-- Boinc Monitoring stored procedures
-- Only re-run this file upon changes


DELIMITER $$

-- Extract a delimited subset of a given string 
-- Arguments: string, from substring, to substring (deprecated)
DROP FUNCTION IF EXISTS mon_xt  
$$ 
CREATE FUNCTION mon_xt (s blob,f varchar(100), t varchar(100)) RETURNS varchar(1000) deterministic no sql
BEGIN
	return substring_index(substring_index(s,f,-1),t,1);
END 
$$


-- Split the given string at delimiter del, return the idx-th piece (1-based)
-- Arguments: string, delimiter, index
DROP FUNCTION IF EXISTS mon_split_index
$$ 
CREATE FUNCTION mon_split_index (s blob, del varchar(10), idx int) RETURNS varchar(1000) deterministic no sql
BEGIN
	return substring_index(substring_index(s,del,idx),del,-1);
END
$$


-- Extract the name from a result or wu-- Argument: wu name                                  
-- 1-OTTO_pYIpYV_1805-2-10-RND1289_0 ->  OTTO_pYIpYV_1805
DROP FUNCTION IF EXISTS mon_wuname
$$   
CREATE FUNCTION mon_wuname (s varchar(1000)) RETURNS varchar(1000) deterministic no sql
BEGIN
	return mon_split_index(s,"-",2);
END 
$$



-- Extract the step from a result or wu-- Argument: wu name                                  
-- 1-OTTO_pYIpYV_1805-2-10-RND1289_0 ->  2
DROP FUNCTION IF EXISTS mon_stepof
$$   
CREATE FUNCTION mon_stepof (s varchar(1000)) RETURNS varchar(1000) deterministic no sql
BEGIN
	return mon_split_index(s,"-",3);
END 
$$


-- Extract the sumbitter name from a result or wu
-- Argument: wu name
-- 1-OTTO_pYIpYV_1805-2-10-RND1289_0 ->  OTTO
DROP FUNCTION IF EXISTS mon_submitterof
$$
CREATE FUNCTION mon_submitterof (s varchar(1000)) RETURNS varchar(1000) deterministic no sql
BEGIN
	return mon_split_index(mon_wuname(s),"_",1);
END
$$


-- Extract the group name from a result or wu                                  
-- Argument: wu name                                  
-- 1-OTTO_pYIpYV_1805-2-10-RND1289_0 ->  pYIpYV_1805
DROP FUNCTION IF EXISTS mon_groupof  
$$
CREATE FUNCTION mon_groupof (s varchar(1000)) RETURNS varchar(1000) deterministic no sql
BEGIN
	declare wn varchar(1000);
        set wn=mon_wuname(s);
        return substring(wn,1+instr(wn,'_'));
END
$$



-- Parse stderr to find the CUDA device used (GPUGRID-SPECIFIC)
-- Argument: the stderr text
DROP FUNCTION IF EXISTS mon_cardof
$$
CREATE FUNCTION mon_cardof (str blob) returns varchar(1000) deterministic no sql
BEGIN
        declare bno int;
        declare bname varchar(100);
        declare bclock int;
        declare bmem int;
        declare bmproc int;
        declare bcores int;
        declare tmp varchar(100);
        set bno=mon_xt(str,'Using device ','#');
        set tmp=concat('Device ',bno,': ');
        set bname=mon_xt(str,tmp,'#');
        set bname=replace(bname,'"','');
        set bname=replace(bname,'\r','');
        set bname=replace(bname,'\n','');
        return bname;
END
$$


DELIMITER ;
