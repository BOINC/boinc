<?php

ini_set('display_errors', 'stdout');
error_reporting(E_ALL);

// represents a connection to a database.
// Intended to be subclassed (e.g., BoincDb, BossaDb)

class DbConn {
    private $db_conn;
    private $db_name;

    function init_conn($user_tag, $passwd_tag, $host_tag, $name_tag) {
        $config = get_config();
        $user = parse_config($config, $user_tag);
        $pass = parse_config($config, $passwd_tag);
        $host = parse_config($config, $host_tag);
        if ($host == null) {
            $host = "localhost";
        }
        $this->db_conn = mysql_pconnect($host, $user, $pass);
        if (!$this->db_conn) {
            return false;
        }
        $this->db_name = parse_config($config, $name_tag);
        return true;
    }

    function do_query($query) {
        $q = str_replace('DBNAME', $this->db_name, $query);
        //echo $q;
        return mysql_query($q, $this->db_conn);
    }

    function lookup($table, $classname, $clause) {
        $query = "select * from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        if (!$result) {
            echo $query;
            echo mysql_error();
            return null;
        }
        $obj = mysql_fetch_object($result, $classname);
        mysql_free_result($result);
        return $obj;
    }

    function lookup_id($id, $table, $classname) {
        return $this->lookup($table, $classname, "id=$id");
    }

    function enum($table, $classname, $clause=null) {
        $x = array();
        if ($clause) {
            $query = "select * from DBNAME.$table where $clause";
        } else {
            $query = "select * from DBNAME.$table";
        }
        $result = $this->do_query($query);
        if (!$result) return null;
        while ($obj = mysql_fetch_object($result, $classname)) {
            $x[] = $obj;
        }
        mysql_free_result($result);
        return $x;
    }
    function update($obj, $table, $clause) {
        $query = "update DBNAME.$table set $clause where id=$obj->id";
        return $this->do_query($query);
    }
    function insert_id() {
        return mysql_insert_id($this->db_conn);
    }
    function count($table, $clause) {
        $query = "select count(*) as total from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        $cnt = mysql_fetch_object($result);
        if ($cnt) return $cnt->total;
        return null;
        mysql_free_result($result);
    }
}

?>
