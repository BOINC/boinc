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

require_once("../inc/db.inc");

// represents a connection to a database.
// Intended to be subclassed (e.g., BoincDb, BossaDb)
//
class DbConn {
    var $db_conn;       // a mysqli object
    var $db_name;       // the DB name

    function init_conn($user, $passwd, $host, $name) {
        $x = explode(":", $host);
        if (sizeof($x)>1) {
            $host = $x[0];
            $port = $x[1];
        } else {
            $port = null;
        }
        try {
            $this->db_conn = @new mysqli($host, $user, $passwd, $name, $port);
        } catch(Exception $e) {
            return false;
        }
        if (mysqli_connect_error()) {
            return false;
        }
        global $mysqli;
        $mysqli = $this->db_conn;
        if (!$this->db_conn) {
            return false;
        }
        $this->db_name = $name;
        return true;
    }

    function close() {
        if ($this->db_conn) {
            $this->db_conn->close();
        }
    }

    // in keeping with PHP/MySQL convention, return true (nonzero) on success.
    // (This is the opposite of the BOINC convention)
    //
    function do_query($q) {
        global $generating_xml;
        $q = str_replace('DBNAME', $this->db_name, $q);
        //echo "query: $q<br>\n";
        $ret = $this->db_conn->query($q);
        if (!$ret) {
            if (!$generating_xml) {
                echo "Database Error<br>\n";
            }
            //echo ": ", $this->base_error(), "\n<pre>";
            //var_dump(debug_backtrace());
            //echo "</pre>query: $q\n";
            return null;
        }
        return $ret;
    }

    // # rows affected by last query
    //
    function affected_rows() {
        return $this->db_conn->affected_rows;
    }

    function get_list($table1, $table2, $joinfield1, $joinfield2, $classname, $fields, $where_clause, $order_clause, $limit) {
        $query = "select $fields from DBNAME.$table1 a join DBNAME.$table2 b on a.$joinfield1=b.$joinfield2 where $where_clause order by $order_clause desc limit $limit";
        $result = $this->do_query($query);
        if (!$result) return null;
        $x = array();
        while ($obj = $result->fetch_object($classname)) {
            $x[] = $obj;
        }
        $result->free();
        return $x;
    }

    function lookup_fields($table, $classname, $fields, $clause) {
        $query = "select $fields from DBNAME.$table where $clause";
        $result = $this->do_query($query);
        if (!$result) {
            return null;
        }
        $obj = $result->fetch_object($classname);
        $result->free();
        return $obj;
    }

    function lookup($table, $classname, $clause) {
        return $this->lookup_fields($table, $classname, "*", $clause);
    }

    function lookup_id($id, $table, $classname) {
        $id = (int)$id;
        return $this->lookup($table, $classname, "id=$id");
    }

    function enum_general($classname, $query) {
        $result = $this->do_query($query);
        if (!$result) return [];
        $x = [];
        while ($obj = $result->fetch_object($classname)) {
            $x[] = $obj;
        }
        $result->free();
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
        return $this->enum_general($classname, $query);
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
    function update_aux($table, $clause, $where='') {
        if ($where) $where = " where $where";
        $query = "update DBNAME.$table set $clause $where";
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
        return $this->db_conn->insert_id;
    }
    function get_int($query, $field) {
        $result = $this->do_query($query);
        if (!$result) error_page("database error on query $query");
        $x = $result->fetch_object("StdClass");
        $result->free();
        if ($x) return $x->$field;
        return false;
    }
    function get_double($query, $field) {
        $result = $this->do_query($query);
        if (!$result) error_page("database error on query $query");
        $x = $result->fetch_object("StdClass");
        $result->free();
        if ($x) return (double)$x->$field;
        return false;
    }
    function count($table, $clause="TRUE") {
        $query = "select count(*) as total from DBNAME.$table where $clause";
        return $this->get_int($query, 'total');
    }
    function sum($table, $field, $clause="") {
        $query = "select sum($field) as total from DBNAME.$table $clause";
        return $this->get_double($query, 'total');
    }
    function max($table, $field, $clause="") {
        $query = "select max($field) as total from DBNAME.$table $clause";
        return $this->get_double($query, 'total');
    }
    function percentile($table, $field, $clause, $pct) {
        $n = $this->count($table, $clause);
        if (!$n) return false;
        $m = (int)($n*$pct/100);
        $query = "select $field from DBNAME.$table where $clause order by $field limit $m,1";
        return $this->get_double($query, $field);
    }
    function replace($table, $clause) {
        $query = "replace into DBNAME.$table set $clause";
        return $this->do_query($query);
    }
    function base_escape_string($string) {
        return $this->db_conn->escape_string($string);
    }
    function base_error() {
        return $this->db_conn->error;
    }
    function base_errno() {
        return $this->db_conn->errno;
    }
    function table_exists($table_name) {
        $result = $this->do_query("show tables from DBNAME like '$table_name'");
        if (!$result) error_page("database error on query $query");
        $t = _mysql_fetch_array($result);
        _mysql_free_result($result);
        if ($t[0] == "$table_name") return true;
        return false;
    }
}

?>
