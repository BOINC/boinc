<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// functions to query various entities, and display them,
// in admin web pages
// TODO: most of this code shouldn't exist;
// use the standard code for displaying WUs etc. instead

require_once("../inc/util_basic.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/result.inc");
require_once("../inc/boinc_db.inc");

class BoincAssignment {
    static function enum($where_clause = null) {
        $db = BoincDb::get();
        return $db->enum('assignment', 'BoincAssignment', $where_clause);
    }
}

define("NVALIDATE_STATES", 6);

// Converts a mysql timestamp to a user readable form
// NOTE: the result will say UTC but it's actually local time.
// Should fix at some point.
//
function mysqltime_str($x) {
    if(strpos($x,"-")==4) {
        // Syntax of supplied mysql-timestamp is YYYY-MM-DD HH:MM:SS
        $year = substr($x,0,4);
        $month = substr($x,5,2);
        $day = substr($x,8,2);
        $hour = substr($x,11,2);
        $minute = substr($x,14,2);
        $second = substr($x,17,2);
    } else {
        // Syntax of supplied mysql-timestamp is YYYYMMDDHHMMSS
        $year = substr($x,0,4);
        $month = substr($x,4,2);
        $day = substr($x,6,2);
        $hour = substr($x,8,2);
        $minute = substr($x,10,2);
        $second = substr($x,12,2);

    }
    $time = mktime($hour, $minute, $second, $month, $day, $year);
    return time_str($time);
}

// translate type_id into human readable text
// http://php.net/manual/de/mysqli-result.fetch-field-direct.php
//
function mysql_fieldtype_str($type_id) {
    static $types;
    // maps php constants to mysql type names
    static $mysql_data_type_hash = array(
        MYSQLI_TYPE_TINY=>'tinyint',
        MYSQLI_TYPE_SHORT=>'smallint',
        MYSQLI_TYPE_LONG=>'int',
        MYSQLI_TYPE_FLOAT=>'float',
        MYSQLI_TYPE_DOUBLE=>'double',
        MYSQLI_TYPE_TIMESTAMP=>'timestamp',
        MYSQLI_TYPE_LONGLONG=>'bigint',
        MYSQLI_TYPE_INT24=>'mediumint',
        MYSQLI_TYPE_DATE=>'date',
        MYSQLI_TYPE_TIME=>'time',
        MYSQLI_TYPE_DATETIME=>'datetime',
        MYSQLI_TYPE_YEAR=>'year',
        MYSQLI_TYPE_BIT=>'bit',
        MYSQLI_TYPE_BLOB=>'blob',
        MYSQLI_TYPE_VAR_STRING=>'varchar',
        MYSQLI_TYPE_STRING=>'char',
        MYSQLI_TYPE_DECIMAL=>'decimal'
    );
    if (array_key_exists($type_id, $mysql_data_type_hash)) {
        return $mysql_data_type_hash[$type_id];
    }
    // $type_id is not well known or new so get the constant name and return that
    if (!isset($types)) {
        $types = array();
        $constants = get_defined_constants(true);
        foreach ($constants['mysqli'] as $c => $n) {
            if (preg_match('/^MYSQLI_TYPE_(.*)/', $c, $m)) $types[$n] = $m[1];
        }
    }
    return array_key_exists($type_id, $types)? strtolower($types[$type_id]) : "unknown";
}

// print a description of $table
//
function print_describe_table_onecol($table, $which, $columns) {
    $db = BoincDb::get(true);
    $result = $db->do_query("SELECT * from $table LIMIT 1");
    $fields = $result->field_count;

    $avgnum=(int)($fields/$columns);
    if ($avgnum*$columns<$fields) {
        $avgnum++;
    }

    $actualcolumns=0;
    while ($avgnum*$actualcolumns<$fields) {
        $actualcolumns++;
    }

    if ($which>$actualcolumns) {
        return 0;
    }

    $bot=($which-1)*$avgnum;
    $top=$which*$avgnum;

    $width=100.0/$actualcolumns;

    echo "<td>";
    start_table('table-striped');
    echo "<tr><th align=\"left\">NAME</th><th align=\"left\">Type</th><th align=\"left\">Bytes</th>\n";
    for ($count=$bot; $count<$top; $count++) {
        if ($count<$fields) {
            $x = $result->fetch_field_direct($count);
            $name = $x->name;
            $type = mysql_fieldtype_str($x->type);
            $length = $x->length;
        } else {
            $name="<br/> ";
            $type="<br/>";
            $length="<br/>";
        }
        echo "\t<tr><td><b>$name</b></td><td>$type</td><td>$length</td></tr>\n";
    }
    end_table();
    echo "</td>";
    $result->free();
    return 0;
}

function print_describe_table($table, $how_many_columns) {
    // Number of columns for showing table description
    echo "<h2>Description of <b>$table</b> table fields:</h2>\n";
    start_table();
    echo "<tr>";
    for ($i=1; $i<=$how_many_columns; $i++) {
        print_describe_table_onecol($table, $i, $how_many_columns);
    }
    echo "</tr>";
    end_table();
    return 0;
}

function print_detail_field() {
    echo "<tr><td align=\"right\">Detail level</td><td>";
    echo "<select name=\"detail\">
        <option value=\"low\">low
        <option value=\"high\">high
        </select>
        </td></tr>
    ";
}

function print_query_field() {
    $currenttime=time();
    $hourago=$currenttime-3600;
    $dayago=$currenttime-24*3600;
    $weekago=$currenttime-7*24*3600;
    echo "
        <tr>
        <td align=\"right\">Additional clauses</td>
        <td><input name=\"clauses\" size=\"100\"></td>
        </tr><tr>
        <td align=\"right\">Unix times</td>
        <td>Now:<b> $currenttime</b> Hour ago: $hourago Day ago: $dayago Week ago: $weekago</td>
        </tr>
    ";
}

function join_query_string($s1, $s2) {
    if ($s1) {
        if ($s2) {
            return "$s1&s2";
        } else {
            return $s1;
        }
    } else {
        return $s2;
    }
}

function append_sql_query($original,$addition,$first) {
    if ($first == 1) {
        return $original . " where " . $addition;
    } else {
        return $original . " and " . $addition;
    }
}

// SqlQueryString maps a bunch of form items to a SQL query
//
// The items are
// table        (name of table)
// id
// platformid
// appid
// workunitid
// hostid
// userid
// teamid
// nsecs            (modified_time > now - nsecs)
// received_time    (> x)
// server_state     (== x if z nonzero)
// outcome          (== x if z nonzero)
// client_state     (== x if z nonzero)
// exit_status      (== x if z nonzero)
// clauses          (literals added after the above)
// sort_by          (order by x desc added after all else)
//
// Once you've parsed the items (using parse_form_items()):
//
// get_select_query(n, m) returns the SQL query to get items from m to m+n
// get_url() returns the URL-encoded version of everything
// count() returns the number of records satisfying the query

class SqlQueryString {
    var $table;
    var $query;
    var $urlquery;

    function __construct() {
        if (isset($_GET['table'])) {
            $this->table = $_GET['table'];
        } else {
            $this->table = "";
        }
        $this->query = "";
        $this->urlquery = "";
    }
    function add($clause) {
        //$clause=boinc_real_escape_string($clause);
        if (!$this->query) {
            $this->query .= "where $clause";
        } else {
            $this->query .= " and $clause";
        }
    }
    function addclause($clause) {
        if ($clause) {
            $c = urldecode($clause);
            $this->add("( $c )");
            $clause = urlencode($clause);
            $this->urlquery .= "&clauses=$clause";
        }
    }
    function addeq($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = "";
        }
        if (strlen($value)) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addeq_not_CHOOSE_ALL($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = "";
        }
        // On the selection menu, the ALL selection criteria sets (for example)
        // outcome=='CHOOSE_ALL' rather than a numeric value.
        // This means that we enter no condition for this case.
        //
        if (strlen($value) && strcmp("CHOOSE_ALL", $value)) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addgt($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = '';
        }
        if (strlen($value) && $value > 0) {
            $this->add("$name > '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addsort($name, $order) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value=null;
        }
        if (isset($_GET[$order])) {
             $order = $_GET[$order];
        } else {
             $order = null;
        }
        if ($value) {
            if ($order == 'asc') {
                $this->query .= " order by $value asc";
                $this->urlquery .= "&sort_by_order=".urlencode($order);
            } else {
                $this->query .= " order by $value desc";
                $this->urlquery .= "&$name=".urlencode($value);
            }
        }
    }

    function count() {
        $count_query = "select count(*) as cnt from $this->table $this->query";
        $db = BoincDb::get(true);
        $result = $db->do_query($count_query);
        if (!$result) return 0;
        $res = $result->fetch_object();
        $result->free();
        return $res->cnt;
    }

    function get_select_query($entries_to_show, $start_at) {
        if ($entries_to_show) {
            if ($start_at) {
                return "select * from $this->table $this->query limit $start_at,$entries_to_show";
            } else {
                return "select * from $this->table $this->query limit $entries_to_show";
            }
        } else {
            return "select * from $this->table $this->query";
        }
    }

    function get_url($base = "db_action.php") {
        $s = $base . "?table=$this->table$this->urlquery";
        return $s;
    }

    function process_form_items() {
        $this->addeq('id');
        $this->addeq('platformid');
        $this->addeq('appid');
        $this->addeq('workunitid');
        $this->addeq('hostid');
        $this->addeq('userid');
        $this->addeq('teamid');
        $this->addeq('app_version_id');
        $this->addeq('exit_status');
        if (isset($_GET['nsecs'])) {
            $_GET['mod_time'] = date("YmdHis",time() - $_GET['nsecs']);
        }
        $this->addgt('mod_time');
        $this->addeq_not_CHOOSE_ALL('server_state');
        $this->addeq_not_CHOOSE_ALL('outcome');
        $this->addeq_not_CHOOSE_ALL('client_state');
        $this->addeq_not_CHOOSE_ALL('validate_state');
        $clauses = get_str("clauses", true);
        if ($clauses && strstr($clauses, ";")) error_page("bad clause");
        if ($clauses) {
            $this->addclause($clauses);
        }
        $this->addsort('sort_by', 'sort_by_order');
    }
}


function link_results($n, $mq, $query, $clauses) {
    if ($n == '0') { // intentional compare by string
        return "0";
    } else {
        if(strlen($clauses)) {
            return "<a href=\"db_action.php?table=result&query=$mq&$query&clauses=".urlencode($clauses)."&sort_by=mod_time&detail=low\">$n</a>";
        } else {
            return "<a href=\"db_action.php?table=result&query=$mq&$query&sort_by=mod_time&detail=low\">$n</a>";
        }

    }
}

// Determines if stderr_out is an error reported and prints as human readable String
// @return String A human readable string if error otherwise FALSE
// @param String $stderr_out the stderr_out value to parse
//
function stderr_error_string($stderr_out){
    $y = parse_element($stderr_out, "<error_code>");
    $x = 0;
    if ($y) {
        $x = (int)$y;
    }
    if (0<=$x && $x<=9) {
        return FALSE;
    } else {
        return "$x ".error_code_str($x);
    }
}

function admin_show_result_summary() {
    $ntotal =0;     // TODO: how to count $result?
    $nvalid = 0;    // for SUCCESS results
    $ninvalid = 0;
    $nfile_deleted = 0;

    $server_state = array();
    $outcome = array();
    $client_state = array();

    for ($ss=1; $ss<6; $ss++) {
        $server_state[$ss] = 0;
    }
    for ($ro=0; $ro<8; $ro++) {
        $outcome[$ro] = 0;
    }
    for ($cs=1; $cs<7; $cs++) {
        $client_state[$cs] = 0;
    }
    for ($fds=0; $fds<4; $fds++) {
        $delete_state[$fds] = 0;
    }
    for ($vs=0; $vs<NVALIDATE_STATES; $vs++) {
        $validate_state[$vs]=0;
    }

    $_GET['table'] = 'result';
    $_GET['sort_by'] = ''; // ignore sort

    if (isset($_GET['appid'])) {
        $query_appid = $_GET['appid'];
    } else {
        $query_appid = "";
    }
    $query_mod_time = 0;
    if (isset($_GET['nsecs'])) {
        //$query_mod_time = time() - $_GET['nsecs'];
        $query_mod_time = $_GET['nsecs'];
    }
    if (isset($_GET['workunitid'])) {
        $query_wuid = $_GET['workunitid'];
    } else {
        $query_wuid = null;
    }
    $q = new SqlQueryString();
    $q->process_form_items();

// Important: these need to be kept consistent with db/boinc_db.h and lib/result_state.h
    $main_query = "
SELECT COUNT(id) AS nTotal,
       SUM(case when server_state = '1' then 1 else 0 end) AS serverstate_inactive,
       SUM(case when server_state = '2' then 1 else 0 end) AS serverstate_unset,
       SUM(case when server_state = '3' then 1 else 0 end) AS serverstate_unset_seq,
       SUM(case when server_state = '4' then 1 else 0 end) AS serverstate_inprogress,
       SUM(case when server_state = '5' then 1 else 0 end) AS serverstate_over,
       SUM(case when server_state = '5' and outcome = '0' then 1 else 0 end) AS outcome_init,
       SUM(case when server_state = '5' and outcome = '1' then 1 else 0 end) AS outcome_success,
       SUM(case when server_state = '5' and outcome = '2' then 1 else 0 end) AS outcome_couldntsend,
       SUM(case when server_state = '5' and outcome = '3' then 1 else 0 end) AS outcome_failure,
       SUM(case when server_state = '5' and outcome = '4' then 1 else 0 end) AS outcome_noreply,
       SUM(case when server_state = '5' and outcome = '5' then 1 else 0 end) AS outcome_didntneed,
       SUM(case when server_state = '5' and outcome = '6' then 1 else 0 end) AS outcome_validateerror,
       SUM(case when server_state = '5' and outcome = '7' then 1 else 0 end) AS outcome_clientdetached,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '0' then 1 else 0 end) AS validate_init,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '1' then 1 else 0 end) AS validate_valid,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '2' then 1 else 0 end) AS validate_invalid,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '3' then 1 else 0 end) AS validate_nocheck,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '4' then 1 else 0 end) AS validate_inconclusive,
       SUM(case when server_state = '5' and outcome = '1' and validate_state = '5' then 1 else 0 end) AS validate_too_late,
       SUM(case when server_state = '5' and outcome = '1' and file_delete_state = '0' then 1 else 0 end) AS filedeletestate_init,
       SUM(case when server_state = '5' and outcome = '1' and file_delete_state = '1' then 1 else 0 end) AS filedeletestate_ready,
       SUM(case when server_state = '5' and outcome = '1' and file_delete_state = '2' then 1 else 0 end) AS filedeletestate_done,
       SUM(case when server_state = '5' and outcome = '1' and file_delete_state = '3' then 1 else 0 end) AS filedeletestate_error,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '0' then 1 else 0 end) AS clientstate_init,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '1' then 1 else 0 end) AS clientstate_downloading,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '2' then 1 else 0 end) AS clientstate_downloaded,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '3' then 1 else 0 end) AS clientstate_computedone,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '4' then 1 else 0 end) AS clientstate_uploading,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '5' then 1 else 0 end) AS clientstate_uploaded,
       SUM(case when server_state = '5' and outcome = '3' and client_state = '6' then 1 else 0 end) AS clientstate_aborted
FROM result WHERE true
    ";

    if ($query_appid) {
        $main_query .= " and appid=$query_appid";
    }
    if ($query_wuid) {
        $main_query .= " and workunitid=$query_wuid";
    }
    if ($query_mod_time) {
        $start = time() - $query_mod_time;

        // If file deletion is delayed by X,
        // subtract X from mod time of  file-deleted results.
        // Otherwise we'll show lots of irrelevant results
        //
        $delay = parse_config(get_config(), "<delete_delay_hours>");
        if ($delay) {
            $start2 = $start - $delay*3600;;
            $main_query .= " and ((file_delete_state>1 and mod_time>FROM_UNIXTIME($start2)) or (mod_time>FROM_UNIXTIME($start)))";
        } else {
            $main_query .= " and mod_time > FROM_UNIXTIME($start)";
        }
    }

    $urlquery = $q->urlquery;
    $db = BoincDb::get(true);
    $result = $db->do_query($main_query);

    // echo "Main query was $main_query<br/>";

    if ($result) {

        $res = $result->fetch_object();
        $ntotal          = $res->nTotal;

        $server_state[1] = $res->serverstate_inactive;
        $server_state[2] = $res->serverstate_unset;
        $server_state[3] = $res->serverstate_unset_seq;
        $server_state[4] = $res->serverstate_inprogress;
        $server_state[5] = $res->serverstate_over;

        $outcome[0]      = $res->outcome_init;
        $outcome[1]      = $res->outcome_success;
        $outcome[2]      = $res->outcome_couldntsend;
        $outcome[3]      = $res->outcome_failure;
        $outcome[4]      = $res->outcome_noreply;
        $outcome[5]      = $res->outcome_didntneed;
        $outcome[6]      = $res->outcome_validateerror;
        $outcome[7]      = $res->outcome_clientdetached;

        $client_state[1] = $res->clientstate_downloading;
        $client_state[2] = $res->clientstate_downloaded;
        $client_state[3] = $res->clientstate_computedone;
        $client_state[4] = $res->clientstate_uploading;
        $client_state[5] = $res->clientstate_uploaded;
        $client_state[6] = $res->clientstate_aborted;

        $validate_state[0] = $res->validate_init;
        $validate_state[1] = $res->validate_valid;
        $validate_state[2] = $res->validate_invalid;
        $validate_state[3] = $res->validate_nocheck;
        $validate_state[4] = $res->validate_inconclusive;
        $validate_state[5] = $res->validate_too_late;

        $file_delete[0]  = $res->filedeletestate_init;
        $file_delete[1]  = $res->filedeletestate_ready;
        $file_delete[2]  = $res->filedeletestate_done;
        $file_delete[3]  = $res->filedeletestate_error;

        $nfile_deleted   = $res->filedeletestate_ready + $res->filedeletestate_done + $res->filedeletestate_error;
        $result->free();
    }


    start_table();
    echo "<tr valign=\"top\">";
    echo "<td><h2>" . link_results("$ntotal results", $urlquery, '', '') . "</h2></td>";
    echo "<td><h2>" . link_results("'Over' results", $urlquery, sprintf('server_state=%d', RESULT_SERVER_STATE_OVER), '') . "</h2></td>";
    echo "<td><h2>" . link_results("'Success' results", $urlquery, sprintf('outcome=%d', RESULT_OUTCOME_SUCCESS), '') . "</h2></td>";
    echo "<td><h2>" . link_results("'Client error' results", $urlquery, sprintf('outcome=%d', RESULT_OUTCOME_CLIENT_ERROR), '') . "</h2></td>";
    echo "</tr>";
    echo "<tr valign=\"top\">";
    echo "<td>";
    start_table('table-striped');
    echo "<tr><th>Server state</th><th># results</th></tr>\n";
    for ($ss=1; $ss<6; $ss++) {
        $res = new StdClass;
        $res->server_state = $ss;
        row2(result_server_state_string($res),
            link_results(
                "$server_state[$ss]",  $urlquery, "server_state=$ss", ''
            )
        );
    }
    end_table();
    echo "</td>";

    echo "<td>";
    start_table('table-striped');
    echo "<tr><th>Outcome</th><th># results</th></tr>\n";

    for ($ro=0; $ro<8; $ro++) {
        $res = new StdClass;
        $res->outcome = $ro;
        $res->exit_status = 0;
        c_row2(
            $outcome[$ro]?outcome_color($ro):'',
            result_outcome_string($res),
            link_results("$outcome[$ro]", $urlquery, "outcome=$ro", '')
        );
    }
    end_table();
    echo "</td>";

    echo "<td>";
    start_table('table-striped');
    echo "<tr><th>Validate state</th><th># results</th></tr>\n";
    for ($vs=0; $vs<NVALIDATE_STATES; $vs++) {
        $res = new StdClass;
        $res->validate_state = $vs;
        $res->exit_status = 0;
        c_row2(
            $validate_state[$vs]?validate_color($vs):'',
            validate_state_str($res),
            link_results(
                "$validate_state[$vs]",
                $urlquery,
                "validate_state=$vs",
                sprintf('outcome=%d', RESULT_OUTCOME_SUCCESS)
            )
        );
    }
    end_table();
    start_table('table-striped');
    echo "<tr><th>File Delete state</th><th># results</th></tr>\n";

    for ($fds=0; $fds<4; $fds++) {
        row2(
            file_delete_state_str($fds),
            link_results(
                "$file_delete[$fds]",
                $urlquery,
                sprintf('outcome=%d', RESULT_OUTCOME_SUCCESS),
                "file_delete_state=$fds"
            )
        );
    }
    row2(
        "Total files deleted",
        link_results(
            "$nfile_deleted",
            $urlquery,
            sprintf('outcome=%d', RESULT_OUTCOME_SUCCESS),
            sprintf(
                '(file_delete_state=%d or file_delete_state=%d or file_delete_state=%d)',
                FILE_DELETE_READY, FILE_DELETE_DONE, FILE_DELETE_ERROR
            )
        )
    );
    end_table();
    echo "</td>";

    echo "<td>";
    start_table('table-striped');
    echo "<tr><th>Client state</th><th># results</th></tr>\n";
    for ($cs=1; $cs<7; $cs++) {
        $res = new StdClass;
        $res->client_state = $cs;
        $res->exit_status = 0;
        row2(result_client_state_string($res),
            link_results(
                "$client_state[$cs]",
                $urlquery,
                "client_state=$cs",
                sprintf('outcome=%d', RESULT_OUTCOME_CLIENT_ERROR)
            )
        );
    }
    end_table();
    echo "</td>";
    end_table();
}

function server_state_select() {
    echo "
        <select name=\"server_state\">
        <option value=\"CHOOSE_ALL\" selected=\"selected\"> Any </option>
    ";
    for ($i=1; $i<6; $i++) {
        $res = new StdClass;
        $res->server_state=$i;
        echo "<option value=\"$i\"> "."[$i]&nbsp;&nbsp;".'   '.result_server_state_string($res)."</option>\n";
    }
    echo "</select>\n";
}

function outcome_select() {
    echo "
        <select name=\"outcome\">
        <option value=\"CHOOSE_ALL\" selected=\"selected\"> Any </option>
    ";
    for ($i=0; $i<8; $i++) {
        $res = new StdClass;
        $res->outcome = $i;
        $res->exit_status = 0;
        echo "<option value=\"$i\"> "."[$i]&nbsp;&nbsp;".'   '.result_outcome_string($res)."</option>\n";
    }
    echo "</select>\n";
}

function validate_state_select() {
    echo "
        <select name=\"validate_state\">
        <option value=\"CHOOSE_ALL\" selected=\"selected\"> Any </option>
        ";
    for ($vs=0; $vs<NVALIDATE_STATES; $vs++) {
        $res = new StdClass;
        $res->validate_state = $vs;
        $res->exit_status = 0;
        echo "<option value=\"$vs\"> "."[$vs]&nbsp;&nbsp;".'   '.validate_state_str($res)."</option>\n";
    }
    echo "</select>\n";
}

function client_state_select() {
    echo "
        <select name=\"client_state\">
        <option value=\"CHOOSE_ALL\" selected=\"selected\"> Any </option>
    ";
    for ($i=0; $i<7; $i++) {
        $res = new StdClass;
        $res->client_state = $i;
        $res->exit_status = 0;
        echo "<option value=\"$i\"> "."[$i]&nbsp;&nbsp;".result_client_state_string($res)."</option>\n";
    }
    echo "</select>\n";
}

function result_sort_select() {
    echo "
        <select name=\"sort_by\">
        <option value=\"\">None
        <option value=\"id\">ID
        <option value=\"sent_time\">Sent time
        <option value=\"mod_time\">Modification time
        <option value=\"received_time\">Received time
        <option value=\"exit_status\">Exit status
        <option value=\"hostid\">Host ID
        <option value=\"userid\">User ID
        <option value=\"app_version_num\">App Version Number
        <option value=\"cpu_time\">CPU time
        <option value=\"workunitid\">Work Unit ID
        </select>
    ";
}

function sort_order_select() {
    echo "
        <select name=\"sort_by_order\">
        <option value=\"asc\">Ascending
        <option value=\"desc\" selected>Descending
        </select>
    ";
}

function table_title($table) {
    switch($table) {
    case "platform": return "Platforms";
    case "app": return "Applications";
    case "app_version": return "Application Versions";
    case "host": return "Hosts";
    case "workunit": return "Workunits";
    case "result": return "Results";
    case "team": return "Teams";
    case "user": return "Users";
    case "profile": return "Profiles";
    default: return "????";
    }
}

function admin_show_platform($platform) {
    start_table();
    row("ID", $platform->id);
    row("Created", time_str($platform->create_time));
    row("Name", $platform->name);
    row("User friendly name", $platform->user_friendly_name);
    row("","<a href=\"db_action.php?table=app_version&platformid=$platform->id\">App versions for this platform</a>");
    end_table();
}

function admin_show_app($app) {
    start_table();
    row("ID", $app->id);
    row("Created", time_str($app->create_time));
    row("Name", $app->name);
    row("User-friendly name", $app->user_friendly_name);
    row("Deprecated", $app->deprecated);
    row("Homogeneous redundancy", $app->homogeneous_redundancy);
    row("","<a href=\"db_action.php?table=app_version&appid=$app->id\">App Versions for this application</a>");
    row("","<a href=\"db_action.php?table=workunit&appid=$app->id&detail=low\">Workunits for this application</a>");
    end_table();
}

function admin_show_app_version($app_version) {
    start_table();
    row("ID", $app_version->id);
    row("Created", time_str($app_version->create_time));
    row("Application", "<a href=\"db_action.php?table=app&id=$app_version->appid\">" . app_name_by_id($app_version->appid) . "</a>");
    row("Version num", $app_version->version_num);
    row("Platform", "<a href=\"db_action.php?table=platform&id=$app_version->platformid\">" . platform_name_by_id($app_version->platformid) . "</a>" );
    row("Plan Class", $app_version->plan_class);
    row("XML doc", "<pre>".htmlspecialchars($app_version->xml_doc)."</pre>");
    row("min_core_version", $app_version->min_core_version);
    row("max_core_version", $app_version->max_core_version);
    row("deprecated", $app_version->deprecated);
    end_table();
}

function app_version_short_header() {
    echo "
        <tr>
        <th>ID</th>
        <th>Application</th>
        <th>Version</th>
        <th>Platform</th>
        <th>Plan Class</th>
        </tr>
    ";
}

function admin_show_app_version_short($app_version) {
    $x = app_name_by_id($app_version->appid);
    $y = platform_name_by_id($app_version->platformid);
    echo "
        <tr>
        <td><a href=\"db_action.php?table=app_version&id=$app_version->id\">$app_version->id</a></td>
        <td><a href=\"db_action.php?table=app&id=$app_version->appid\">$x</a></td>
        <td>$app_version->version_num</td>
        <td><a href=\"db_action.php?table=platform&id=$app_version->platformid\">$y</a></td>
        <td>$app_version->plan_class</td>
        </tr>
    ";
}

function host_short_header() {
    echo "
        <tr>
        <th>host ID</th>
        <th>IP address</th>
        <th>name</th>
        <th>RAC</th>
        <th>total credit</th>
        <th>CPU</th>
        <th>OS</th>
        </tr>
    ";
}

function admin_show_host_short($host) {
    echo "
        <tr>
        <td><a href=\"db_action.php?table=host&id=$host->id\">$host->id</a></td>
        <td>$host->last_ip_addr</td>
        <td>$host->domain_name</td>
    ";
    printf("<td>%.2f</td>", $host->expavg_credit);
    printf("<td>%.1f</td>", $host->total_credit);
    echo "<td>$host->p_vendor $host->p_model</td>
        <td>$host->os_name $host->os_version</td>
        </tr>
    ";
}

function days_string($x) {
    return number_format($x/86400, 2)." days";
}

function resource_name($x) {
    switch ($x) {
    case 2: return "CPU";
    case 3: return "NVIDIA";
    case 4: return "AMD";
    }
    return "Unknown resource: $x";
}
function hav_app_version_string($avid) {
    if ($avid > 1000000) {
        $appid = (int)($avid/1000000);
        $platform_name = "Anonymous platform";
        $version_info = resource_name($avid%1000000);
    } else {
        $av = BoincAppVersion::lookup("id=$avid");
        if (!$av) return "Missing app version $avid";
        $appid = $av->appid;
        $platform = BoincPlatform::lookup_id($av->platformid);
        if (!$platform) return "Missing platform $av->platformid";
        $platform_name = $platform->user_friendly_name;
        $version_info = "$av->version_num $av->plan_class
            <br>PFC: $av->pfc_avg ($av->pfc_n)
            <br>scale: $av->pfc_scale
        ";
    }
    $app = BoincApp::lookup_id($appid);
    if (!$app) return "Missing app $appid";
    return "$app->user_friendly_name
        <br>$platform_name
        <br>$version_info
    ";
}

function admin_show_host_app_versions($hostid) {
    start_table();
    table_header("app version", "PFC", "Elapsed", "Turnaround");
    $havs = BoincHostAppVersion::enum("host_id=$hostid");
    foreach ($havs as $hav) {
        table_row(
            hav_app_version_string($hav->app_version_id),
            "$hav->pfc_avg ($hav->pfc_n)",
            "$hav->et_avg ($hav->et_n)",
            days_string($hav->turnaround_avg)." ($hav->turnaround_n)"
        );
    }
    end_table();
}

function admin_show_host($host) {
    start_table();

    row("ID", $host->id);
    row("Created", time_str($host->create_time));
    row("User",
        "<a href=\"db_action.php?table=user&id=$host->userid\">".user_name_by_id($host->userid)."($host->userid)</a>"
    );

    row("Venue", $host->venue);
    row("Info", $host->misc);
    row("Total credit", $host->total_credit);
    row("Average credit", $host->expavg_credit);
    row("Average update time", time_str($host->expavg_time));
    row("IP address", "$host->last_ip_addr<br>(same the last $host->nsame_ip_addr times)");
    row("External IP address", "$host->external_ip_addr<br>");
    row("Domain name", $host->domain_name);
    $x = $host->timezone/3600;
    if ($x >= 0) $x="+$x";
    row("Local Standard Time", "UTC $x hours");
    row("Number of CPUs", $host->p_ncpus);
    row("CPU", "$host->p_vendor $host->p_model");
    row("GFLOPS", number_format($host->p_fpops/1e9, 2));
    row("GIOPS", number_format($host->p_iops/1e9, 2));

    row("Number of GPUs", $host->p_ngpus);
    row("GPU GFLOPS", number_format($host->p_gpu_fpops/1e9, 2));

    $x = $host->p_membw/(1024*1024);
    $y = number_format($x, 2);
    row("Memory bandwidth", "$y MB/sec");
    row("Operating System", "$host->os_name $host->os_version");
    $x = $host->m_nbytes/(1024*1024);
    $y = number_format($x, 2);
    row("Memory", "$y MB");
    $x = $host->m_cache/1024;
    $y = number_format($x, 2);
    row("Cache", "$y KB");
    $x = $host->m_swap/(1024*1024);
    $y = number_format($x, 2);
    row("Swap Space", "$y MB");
    $x = $host->d_total/(1024*1024*1024);
    $y = number_format($x, 2);
    row("Total Disk Space", "$y GB");
    $x = $host->d_free/(1024*1024*1024);
    $y = number_format($x, 2);
    row("Free Disk Space", "$y GB");
    $x = number_format($host->n_bwup/1024);
    row("Avg network bandwidth (upstream)", "$x kB/sec");
    $x = number_format($host->n_bwdown/1024);
    row("Avg network bandwidth (downstream)", "$x kB/sec");
    row("Average turnaround", days_string($host->avg_turnaround));
    row("Number of RPCs", $host->rpc_seqno);
    row("Last RPC", time_str($host->rpc_time));
    row("% of time client on", 100*$host->on_frac." %");
    row("% of time host connected", 100*$host->connected_frac." %");
    row("% of time user active", 100*$host->active_frac." %");
    row("# of results today", $host->nresults_today);
    row("Results", "<a href=\"db_action.php?table=result&detail=low&hostid=$host->id&sort_by=sent_time\">click here</a>");
    end_table();
    admin_show_host_app_versions($host->id);
}

function admin_show_workunit($wu) {
    $_GET = array('workunitid' => $wu->id);
    admin_show_result_summary();

    start_table();
    row("Created", time_str($wu->create_time));
    row("Transition Time", time_str($wu->transition_time));
    row("Last time modified", mysqltime_str($wu->mod_time));
    row("Name", $wu->name);
    row("XML doc", "<pre>".htmlspecialchars($wu->xml_doc)."</pre>");
    row("Application", "<a href=\"db_action.php?table=app&id=$wu->appid\">" . app_name_by_id($wu->appid) . " [".$wu->appid."]</a>");
    row("Application version number", $wu->app_version_num);
    row("Batch", $wu->batch);
    $x = number_format($wu->rsc_fpops_est/1e9, 2);
    row("Estimated FP Operations", "$x GFLOPS");
    $x = number_format($wu->rsc_fpops_bound/1e9, 2);
    row("Max FP Operations", "$x GFLOPS");
    $x = $wu->rsc_memory_bound/(1024*1024);
    $y = number_format($x, 2);
    row("Max Memory Usage", "$y MB");
    $x = $wu->rsc_disk_bound/(1024*1024);
    $y = number_format($x, 2);
    row("Max Disk Usage", "$y MB");
    row("Need validate?", ($wu->need_validate?"yes [":"no [").$wu->need_validate."]");
    row("Canonical resultid",
            "<a href=\"db_action.php?table=result&id=$wu->canonical_resultid\">".$wu->canonical_resultid."</a>");
    row("Canonical credit", $wu->canonical_credit);
    //row("Timeout check time", time_str($wu->timeout_check_time));
    row("Delay bound", "$wu->delay_bound" . " =  " . time_diff($wu->delay_bound) );
    row("Error mask", wu_error_mask_str($wu->error_mask, true));
    row("File delete state", file_delete_state_str($wu->file_delete_state)." [".$wu->file_delete_state."]");
    row("Assimilation state", assimilate_state_str($wu->assimilate_state)." [".$wu->assimilate_state."]");
    // row("","<a href=db_action.php?table=result&workunitid=$wu->id&detail=low>Show associated results</a>");
    row("min quorum", $wu->min_quorum);
    row("target results", $wu->target_nresults);
    row("max error results", $wu->max_error_results);
    row("max total results", $wu->max_total_results);
    row("max success results", $wu->max_success_results);
    row("result template file",$wu->result_template_file);
    row("hr_class", $wu->hr_class);
    row("opaque", $wu->opaque);
    row("Priority", $wu->priority);
    row("Keywords", ($wu->keywords ? $wu->keywords : "<em>empty</em>"));
    end_table();
    echo "<div align=\"center\">";
    echo "<a href=\"show_log.php?s=$wu->name\">GREP LOGS FOR THIS WORKUNIT</a>";
    echo "</div>";
    echo "<p>";
}

function workunit_short_header() {
    echo "
        <tr>
        <th>ID</th>
        <th>name</th>
        <th>canonical result</th>
        <th>error_mask</th>
        <th>file delete</th>
        <th>assimilate</th>
        </tr>
    ";
}

function admin_show_workunit_short($wu) {
    if ($wu->canonical_resultid) {
        $cr = sprintf('<a href="db_action.php?table=result&id=%d">%d</a>',
            $wu->canonical_resultid,
            $wu->canonical_resultid
        );
    } else {
        $cr = "none";
    }
    $cr .= sprintf(
        ' <a href="db_action.php?table=result&workunitid=%d&detail=low">all</a>',
        $wu->id
    );
    $wu_link = sprintf(
        '<a href="db_action.php?table=workunit&id=%d&detail=high">%d</a>',
        $wu->id,
        $wu->id
    );
    $e = wu_error_mask_str($wu->error_mask, true);
    $f = file_delete_state_str($wu->file_delete_state);
    $a = assimilate_state_str($wu->assimilate_state);
    echo "
        <tr>
        <td>$wu_link</td>
        <td>$wu->name</td>
        <td>$cr</td>
        <td>$e</td>
        <td>$f</td>
        <td>$a</td>
        </tr>
    ";
}

function host_user_link($hostid) {
    if (!$hostid) return '---';

    $h = "<a href=\"db_action.php?table=host&id=$hostid\">$hostid</a>";
    $host = BoincHost::enum_fields("userid", "id=".$hostid, "limit 1");
    if (!$host) return $h;
    $user = BoincUser::enum_fields("id, name", "id=".$host[0]->userid, "limit 1");
    if (!$user) return $h;
    return "$h<br><small>(<a href=\"db_action.php?table=user&id=".$user[0]->id."\">".$user[0]->name."</a>)</small>";
}

function validate_color($validate_state) {
    switch ($validate_state) {
   case 1: return '33cc33'; // valid, green
   case 2: return 'ffa500'; // invalid result, orange
    }
    return '';
}

function outcome_color($outcome) {
    switch($outcome) {
    case 0: return '9900cc'; // "Init", purple
    case 1: return '33cc33'; // "Success", green
    case 3: return 'ff3333'; // "Client error", red
    case 4: return 'ff6699'; // "No reply", pink
    case 6: return 'ffff33'; // "Validate error", yellow
    }
    return '';
}

function credit_str($c) {
    if ($c) {
        return sprintf("%.3f", $c);
    } else {
        return '---';
    }
}

function admin_show_result($result) {
    $wu_name = wu_name_by_id($result->workunitid);

    start_table();

    row("Created", time_str($result->create_time));
    row("Sent", time_str($result->sent_time));
    row("Report deadline", time_str($result->report_deadline));
    row("Received", time_str($result->received_time));
    row("Last time modified", mysqltime_str($result->mod_time));
    row("Name", $result->name);
    row("Workunit", "<a href=\"db_action.php?table=workunit&id=$result->workunitid\">" . wu_name_by_id($result->workunitid) . "</a> [$result->workunitid]" );
    row("Server state", result_server_state_string($result)." [$result->server_state]");
    row("Outcome", result_outcome_string($result)." [$result->outcome]");
    row("Client state", result_client_state_string($result)." [$result->client_state]");
    row("Exit status", exit_status_string($result->exit_status));
    row("Host ID", "<a href=\"db_action.php?table=host&id=$result->hostid\">" . host_name_by_id($result->hostid) . "</a> [$result->hostid]");
    row("User ID", "<a href=\"db_action.php?table=user&id=$result->userid\">" . user_name_by_id($result->userid) . "</a> [$result->userid]");
    row("CPU time", $result->cpu_time);
    row("Elapsed time", $result->elapsed_time);
    if($error=stderr_error_string($result->stderr_out)) {
        row("error in stderr out", $error);
    }
    row("Batch", $result->batch);
    row("File delete state", file_delete_state_str($result->file_delete_state)." [$result->file_delete_state]");
    row("Validate state", validate_state_str($result)." [$result->validate_state]");
    row("Granted credit", $result->granted_credit);
    row("Application", "<a href=\"db_action.php?table=app&id=$result->appid\">".app_name_by_id($result->appid)."</a>");
    if ($result->app_version_id > 0) {
        $x1 = "<a href=\"db_action.php?table=app_version&amp;id=$result->app_version_id\">";
        $x2 = "</a>";
    } else {
        $x1 = $x2 = "";
    }
    row("App version", $x1.app_version_string($result).$x2);
    row("App version ID", $result->app_version_id);
    row("Estimated GFLOPS", number_format($result->flops_estimate/1e9, 2));
    row("Random",$result->random);
    row("Opaque",$result->opaque);
    row("Teamid",$result->teamid);
    row("Priority",$result->priority);
    row("XML doc in", "<pre>".htmlspecialchars($result->xml_doc_in)."</pre>");
    row("XML doc out", "<pre>".htmlspecialchars($result->xml_doc_out)."</pre>");
    row("stderr out", "<pre>"
        .htmlspecialchars(
            $result->stderr_out,
            ENT_QUOTES | (defined('ENT_SUBSTITUTE')?ENT_SUBSTITUTE:0),
            'utf-8'
        )
        ."</pre>"
    );
    end_table();
    echo "
        <center>
        <a href=\"show_log.php?s=$result->name\">GREP LOGS FOR THIS RESULT</a>
        </center>
        <p>
    ";
}

function result_short_header() {
    echo "
        <tr>
        <th>result ID</th>
        <th>WU ID</th>
        <th>server<br>state</th>
        <th>outcome</th>
        <th>client<br>state</th>
        <th>validate<br>state</th>
        <th>delete<br>state</th>
        <th>exit<br>status</th>
        <th>host<br>(user)</th>
        <th>app<br>ver</th>
        <th>received <br><i>or</i> <font color=\"#ff3333\">dead</font><font color=\"#33cc33\">line</font> <br><i>or</i> <font color=\"#9900cc\">created</font></th>
        <th>CPU<br>hours</th>
        <th>granted<br>credit</th>
        </tr>
    ";
}

function admin_show_result_short($result) {
    $ss = result_server_state_string($result)." [$result->server_state]";
    $oc = result_outcome_string($result)." [$result->outcome]";
    $vs = validate_state_str($result)." [$result->validate_state]";
    $cs2 = result_client_state_string($result)." [$result->client_state]";
    if ($result->outcome == 3) {
        $cs = result_client_state_string($result);
        $oc = "$oc ($cs)";
    }
    if ($result->received_time) {
        $received = time_str($result->received_time);
    } else {
        // result has not been received yet, so show report deadline either
        // in green if in the future or in red if in the past.
        $timenow=time();
        if ($result->report_deadline==0)  {
            // not sent -- show create time in purple
            $received = "<font color=\"9900cc\">". time_str($result->create_time) . "</font>";
        } else if ($result->report_deadline>=$timenow) {
            // overdue -- show deadline in red
            $received = "<font color=\"#33cc33\">". time_str($result->report_deadline) . "</font>";
        } else {
            // in progress and not overdue -- show deadline in green
            $received = "<font color=\"#ff3333\">". time_str($result->report_deadline) . "</font>";
        }
    }
    $version = app_version_string($result)." (<a href=\"db_action.php?table=app_version&id=$result->app_version_id\">$result->app_version_id</a>)";
    $outcome_color = outcome_color($result->outcome);
    $validate_color = validate_color($result->validate_state);
    $host_user = host_user_link($result->hostid);
    $cpu_hours = sprintf("%.1f",$result->cpu_time / 3600);
    $granted_credit = "<a href=credit.php?wu_id=$result->workunitid>".credit_str($result->granted_credit)."</a>";
    $delete_state = file_delete_state_str($result->file_delete_state);

    echo "
        <tr>
        <td><a href=\"db_action.php?table=result&id=$result->id\">$result->id</a></td>
        <td><a href=\"db_action.php?table=workunit&id=$result->workunitid\">$result->workunitid</a></td>
        <td>$ss</td>
        <td bgcolor=$outcome_color>$oc</td>
        <td>$cs2</td>
        <td bgcolor=$validate_color>$vs</td>
        <td>$delete_state</td>
        <td>", exit_status_string($result->exit_status), "</td>
        <td>$host_user</td>
        <td>$version</td>
        <td>$received</td>
        <td>$cpu_hours</td>
        <td>$granted_credit</td>
        </tr>
    ";
}

function admin_show_user($user) {
    start_table();
    row("ID", $user->id);
    row("Created", time_str($user->create_time));
    row("Name", $user->name);
    if(!in_rops()) {
        row("Authenticator", $user->authenticator);
    }
    row("Email address", $user->email_addr);
    row("Previous email address", $user->previous_email_addr);
    row("Email change time", time_str($user->email_addr_change_time));
    row("OK to send Email?", $user->send_email);
    row("Country", $user->country);
    row("Postal code", $user->postal_code);
    row("Total credit", $user->total_credit);
    row("Average credit", $user->expavg_credit);
    row("Last average time", time_str($user->expavg_time));
    row("Default venue", $user->venue);
    row("Hosts", "<a href=\"db_action.php?table=host&userid=$user->id&detail=low\">click</a>");
    row("Cross project ID", $user->cross_project_id);
    if(!in_rops()) {
        row("Password Hash", $user->passwd_hash);
    }
    row("Donated", $user->donated);
    end_table();
}

function admin_show_user_summary($maxuser) {
    $top_country = array();
    $top_language = array();
    $db = BoincDb::get(true);
    $stats_res = $db->do_query("select max(id) as maxuser,
        SUM(case when has_profile = '1' then 1 else 0 end) as profuser,
        SUM(case when teamid != '0' then 1 else 0 end) as teamuser
        from user"
    );
    $stats = $stats_res->fetch_assoc();
    if ($maxuser > $stats['maxuser']) {
        $maxuser = $stats['maxuser'];
    }
    // TODO: what is in profile.posts? A user can post in the forums without a profile.
    $users = BoincUser::enum(null, "order by posts desc limit ".$maxuser);
    $profiles = BoincProfile::enum(null, "order by posts desc limit ".$maxuser);
    foreach ($users as $user) {
        $top10poster[$i] = $user;
        $top_country[$user->country] += 1;
    }
    foreach ($profiles as $profile) {
        if ($profile->language != '') {
            $top_language[$profile->language] += 1;
        }
    }
    $stats_res->free();
    echo "<table>
          <tr valign=\"top\">
            <td><h2>General</h2></td>
            <td><h2>top10 Poster</h2></td>
            <td><h2>top$maxuser Countries</h2></td>
            <td><h2>top$maxuser Languages</h2></td>
          </tr>
    ";
    echo "<tr valign=\"top\">";
    echo "<td><table border=\"1\">
          <tr><th>&nbsp;</th><th>&nbsp;</th></tr>
    ";
    row2_plain("Users:", $stats['maxuser']);
    row2_plain("Profiles:", $stats['profuser']);
    row2_plain("Team members:", $stats['teamuser']);
    echo "</table></td>";
    echo "<td><table border=\"2\">\n";
    echo "<tr><th>User</th><th># posts</th></tr>\n";
    for ($p=1; $p<=10; $p++) {
        row2_plain(user_links_ops($top10poster[$p]), $top10poster[$p]->posts);
    }
    echo "</table></td>";
    echo "<td><table border=\"2\">\n";
    echo "<tr><th>Country</th><th># users</th></tr>\n";
    arsort($top_country);

    foreach ($top_country as $key => $value) {
        row2_plain($key, $value);
    }
    echo "</table></td>";
    echo "<td><table border=\"2\">\n";
    echo "<tr><th>Language</th><th># users</th></tr>\n";
    arsort($top_language);
    foreach ($top_language as $key => $value) {
        row2_plain($key, $value);
    }
    echo "</table></td>";

    echo "</tr></table></td></tr>";
}

function team_type_string($s) {
    switch ($s) {
    case 1: return "Unclassified";
    case 2: return "Company";
    case 3: return "Primary school";
    case 4: return "Secondary school";
    case 5: return "Junior college";
    case 6: return "University or department";
    case 7: return "Government agency";
    default: return "Unknown";
    }
}

function admin_show_team($team) {
    start_table();
    row("ID", $team->id);
    row("Team Founder", "<a href=\"db_action.php?table=user&id=$team->userid\">" . user_name_by_id($team->userid) . "</a>");
    row("Name", $team->name);
    row("Name (HTML Formatted)", "<pre>" . htmlspecialchars($team->name_html) . "</pre>" );
    row("Url", "<a href=\"http://$team->url\">" . $team->url . "</a>");
    row("Type", team_type_string($team->type));
    row("Description", $team->description);
    row("", "<a href=\"db_action.php?table=user&teamid=$team->id\">List All Members</a>");
    end_table();
}

function team_name_by_id($teamid) {
    $team = BoincTeam::lookup_id($teamid);
    if (!$team) return "No team";
    return $team->name;
}

function user_name_by_id($user_id) {
    $user = BoincUser::lookup_id($user_id);
    if (!$user) return "No user";
    return $user->name;
}

function app_name_by_id($appid) {
    $app = BoincApp::lookup_id($appid);
    if (!$app) return "No app";
    return $app->name;
}

function wu_name_by_id($workunitid) {
    $wu = BoincWorkunit::lookup_id($workunitid);
    if (!$wu) return "Missing workunit";
    return $wu->name;
}

function platform_name_by_id($platformid) {
    $plat = BoincPlatform::lookup_id($platformid);
    if (!$plat) return "Missing platform";
    return $plat->name;
}

function host_name_by_id($hostid) {
    $host = BoincHost::lookup_id($hostid);
    if (!$host) return "No host";
    if (!strlen($host->domain_name) && !strlen($host->last_ip_addr)) {
        return "(blank)";
    } else {
        return $host->domain_name . " (" . $host->last_ip_addr . ")";
    }
}

?>
