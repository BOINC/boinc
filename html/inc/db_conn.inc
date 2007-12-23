<?php

/* Should not be enabled for public code */
//ini_set('display_errors', 'stdout');
//error_reporting(E_ALL);

// represents a connection to a database.
// Intended to be subclassed (e.g., BoincDb, BossaDb)

class DbConn {
    var $db_conn;
    var $db_name;

    function init_conn($user, $passwd, $host, $name) {
        $this->db_conn = mysql_pconnect($host, $user, $passwd);
        if (!$this->db_conn) {
            return false;
        }
        $this->db_name = $name;
        return true;
    }

    function do_query($query) {
        $q = str_replace('DBNAME', $this->db_name, $query);
        //echo "query: $q<br>\n";
        $ret = mysql_query($q, $this->db_conn);
        if (!$ret) {
            echo "Database Error<br>\n";
            //echo ": ", mysql_error(), "\n<pre>";
            //var_dump(debug_backtrace());
            //echo "</pre>query: $q\n";
            return null;
        }
        return $ret;
    }

    function lookup_fields($table, $classname, $fields, $clause) {
        $query = "select $fields from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        if (!$result) {
            return null;
        }
        $obj = mysql_fetch_object($result, $classname);
        mysql_free_result($result);
        return $obj;
    }

    function lookup($table, $classname, $clause) {
        $query = "select * from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        if (!$result) {
            return null;
        }
        $obj = mysql_fetch_object($result, $classname);
        mysql_free_result($result);
        return $obj;
    }

    function lookup_id($id, $table, $classname) {
        return $this->lookup($table, $classname, "id=$id");
    }

    function enum_general($classname, $query) {
        $result = $this->do_query($query);
        if (!$result) return null;
        while ($obj = mysql_fetch_object($result, $classname)) {
            $x[] = $obj;
        }
        mysql_free_result($result);
        return $x;
    }

    function enum_fields(
        $table, $classname, $fields, $where_clause, $order_clause
    ) {
        $x = array();
        if ($where_clause) {
            $where_clause = "where $where_clause";
        }
        $query = "select $fields from DBNAME.$table $where_clause $order_clause";
        $result = $this->do_query($query);
        if (!$result) return null;
        while ($obj = mysql_fetch_object($result, $classname)) {
            $x[] = $obj;
        }
        mysql_free_result($result);
        return $x;
    }
    function enum($table, $classname, $where_clause=null, $order_clause=null) {
        return self::enum_fields(
            $table, $classname, '*', $where_clause, $order_clause
        );
    }

    function update($obj, $table, $clause) {
        $query = "update DBNAME.$table set $clause where id=$obj->id";
        return $this->do_query($query);
    }
    function update_aux($table, $clause) {
        $query = "update DBNAME.$table set $clause";
        return $this->do_query($query);
    }
    function insert($table, $clause) {
        $query = "insert into DBNAME.$table $clause";
        return $this->do_query($query);
    }
    function delete($obj, $table) {
        $query = "delete from DBNAME.$table where id=$obj->id";
        return $this->do_query($query);
    }
    function delete_aux($table, $clause) {
        $query = "delete from DBNAME.$table where $clause";
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
    function replace($table, $clause) {
        $query = "replace into DBNAME.$table set $clause";
        return $this->do_query($query);
    }
    function base_escape_string($string) {
        return mysql_real_escape_string($string, $this->db_conn);
    }
    function base_error() {
        return mysql_error($this->db_conn);
    }
}

?>
