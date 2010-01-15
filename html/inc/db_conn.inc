<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Should not be enabled for public code
//ini_set('display_errors', 'stdout');
//error_reporting(E_ALL);

/// represents a connection to a database.
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

    function affected_rows() {
        return mysql_affected_rows($this->db_conn);
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
        $x = array();
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
        $x = array();
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
        mysql_free_result($result);
        if ($cnt) return $cnt->total;
        return null;
    }
    function replace($table, $clause) {
        $query = "replace into DBNAME.$table set $clause";
        return $this->do_query($query);
    }
    function base_escape_string($string) {
        return mysql_real_escape_string($string);
    }
    function base_error() {
        return mysql_error($this->db_conn);
    }
    function table_exists($table_name) {
        $result = $this->do_query("show tables from DBNAME like '$table_name'");
        $t = mysql_fetch_array($result);
        mysql_free_result($result);
        if ($t[0] == "$table_name") return true;
        return false;
    }
}

?>
