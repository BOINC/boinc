<?php

require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

class DB_REC {

    var $name;
    var $data_size;
    var $index_size;
    var $total_size;
    var $rows;
    var $size_per_row;

    function __construct($n, $d, $i, $t, $r, $s) {
        $this->name = $n;
        $this->data_size = $d;
		$this->index_size = $i;
		$this->total_size = $t;
		$this->rows = $r;
		$this->size_per_row = $s;
    }
    function __destruct() {
    }
}


$db_name = parse_config($config, "<db_name>");

db_init();

admin_page_head("BOINC Database Info");

// if you have other db's just add more get_db_info lines
$db_rec = get_db_info($db_name);

// show_db_info($db_name, $db_rec); 
sort_db_info($db_name, $db_rec);

admin_page_tail();


// returns formatted data size
function size_format($size){
	$retval = 0;

	$KB = 1024;
    $MB = 1024*1024;
    $GB = 1024*1024*1024;
	$TB = 1024*1024*1024*1024;
	
    if ($size < $KB) {
	    $retval = $size;
    } elseif (($size > $KB) && ($size < $MB)) {
		$retval = sprintf("%.0fK", ($size / $KB));    
    } elseif ( ($size >= $MB) && ($size < $GB)) {
		$retval = sprintf("%.2fMB", ($size / $MB));
    } elseif ( ($size >= $GB) && ($size < $TB)) {
		$retval = sprintf("%.2fGB", ($size / $GB));
	} elseif ( $size >= $TB ) {
		$retval = sprintf("%.2fTB", ($size / $TB));
	}
	return $retval;
}


// returns the DB data structure as DB_REC
function get_db_info($db_name) 
{
	// Carl grabbed this from the mysql.com boards http://dev.mysql.com/doc/refman/5.0/en/show-table-status.html  
    $result = mysql_query("SHOW TABLE STATUS FROM $db_name");

    // SQL output
    // mysql> show table status from [table_name];
    // | Name | Engine | Version | Row_format | Rows    
	// | Avg_row_length | Data_length | Max_data_length  
	// | Index_length | Data_free  | Auto_increment | Create_time         
	// | Update_time | Check_time | Collation | Checksum | Create_options | Comment |
    //

    $gdata  = 0;
    $gindex = 0;
    $gtotal = 0;
    $grows  = 0;

	$i = 0;
	$db_rec = array();
	while($myarr = mysql_fetch_assoc($result)) {

		// sum grand totals
		$total  =  $myarr["Data_length"] + $myarr["Index_length"];
		$gindex += $myarr["Index_length"];
        $gdata  += $myarr["Data_length"];
		$grows  += $myarr["Rows"]; 
		$gtotal += $total;
 
	    $db_rec[$i] = new DB_REC ($myarr["Name"], $myarr["Data_length"], $myarr["Index_length"], $total, $myarr["Rows"], $myarr["Avg_row_length"] );	
		$i++;
	}

    $db_rec[$i] = new DB_REC ("Total", $gdata, $gindex, $gtotal, $grows, "" );	

	return $db_rec;
}


// shows the plain db structure
function show_db_info($db_name, $db_rec) 
{ 

	echo "<table cols=6>";
	echo "<tr>";
	echo "<td class=heading colspan=6> Database $db_name </td>";
	echo "</tr>";

	echo "<tr>";
	echo "<td class=heading>Table</td>";
	echo "<td class=heading>Data Size</td>";
	echo "<td class=heading>Index Size</td>";
	echo "<td class=heading>Total Size</td>";
	echo "<td class=heading>Total Rows</td>";
	echo "<td class=heading>Avg. Size per Row</td>";
	echo "</tr>";

	for ($i = 0; $i < sizeof($db_rec)-1; $i++){ 
		echo "<tr>";
		echo "<td align=left valign=top class=fieldname>" . $db_rec[$i]->name . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($db_rec[$i]->data_size)  . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($db_rec[$i]->index_size) . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($db_rec[$i]->total_size) . "</td>";
        echo "<td align=left valign=top class=fieldname>" . number_format($db_rec[$i]->rows)     . "</td>";
        echo "<td align=left valign=top class=fieldname>" . size_format($db_rec[$i]->size_per_row) . "</td>";
		echo "</tr>";
	}

	// Last record is just a summary
	$i = sizeof($db_rec)-1;
	echo "<tr>";
    echo "<td align=left class=heading>" . $db_rec[$i]->name . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->data_size)  . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->index_size) . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->total_size) . "</td>";
    echo "<td align=left class=heading>" . number_format($db_rec[$i]->rows)     . "</td>";
    echo "<td align=left class=heading></td>";
	echo "</tr>";

    echo "</table>";
}



// NB: same as show_db_info but with sortable cloumns
function sort_db_info($db_name, $db_rec) 
{ 
	// sort 
	$file_list = array();
    $file_sort = array();

	$sort =  get_str("sort", true);
	$r = get_str("r", true);

	// check if its empty
	if(empty($sort)) $sort = "name";
	// check for allowed keys 
	if ((strcmp($sort, "name")!=0) && 
		(strcmp($sort, "data_size")!=0) && 
		(strcmp($sort, "index_size")!=0) && 
		(strcmp($sort, "total_size")!=0)  && 
		(strcmp($sort, "rows")!=0) && 
		(strcmp($sort, "size_per_row")!=0)) 
		$sort = "name";
	if(empty($r)) $r=0;

	for ($i = 0; $i < sizeof($db_rec)-1; $i++){
		$file_details["name"]         = $db_rec[$i]->name;
		$file_details["data_size"]    = $db_rec[$i]->data_size;
		$file_details["index_size"]   = $db_rec[$i]->index_size;
		$file_details["total_size"]   = $db_rec[$i]->total_size;
	    $file_details["rows"]         = $db_rec[$i]->rows;
		$file_details["size_per_row"] = $db_rec[$i]->size_per_row;

		$file_list[$i] = $file_details;
		$key = strtolower($file_details[$sort]);
		$file_sort[$i] = $key;
	}
	
	if($r)arsort($file_sort);
    else  asort($file_sort);
	// -- end sort 

	echo "<table cols=6>";
	echo "<tr>";
	echo "<td class=heading colspan=6> Database $db_name </td>";
	echo "</tr>";

	echo "<tr>";
	echo "<td class=heading><a href='dbinfo.php?sort=name&r=" . (!$r) . "'>Table </a></td>";
	echo "<td class=heading><a href='dbinfo.php?sort=data_size&r=" . (!$r) . "'>Data Size</a></td>";
	echo "<td class=heading><a href='dbinfo.php?sort=index_size&r=" . (!$r) . "'>Index Size</a></td>";
	echo "<td class=heading><a href='dbinfo.php?sort=total_size&r=" . (!$r) . "'>Total Size</a></td>";
	echo "<td class=heading><a href='dbinfo.php?sort=rows&r=" . (!$r) . "'>Total Rows</a></td>";
	echo "<td class=heading><a href='dbinfo.php?sort=size_per_row&r=" . (!$r) . "'>Avg. Size per Row</a></td>";
	echo "</tr>";

	$i = 0;
	while ( list($key, $value) = each($file_sort) ) {
		$value = $file_list[$key];
		echo "<tr>"; 
		echo "<td align=left valign=top class=fieldname>" . $value["name"] . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($value["data_size"])  . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($value["index_size"]) . "</td>";
		echo "<td align=left valign=top class=fieldname>" . size_format($value["total_size"]) . "</td>";
        echo "<td align=left valign=top class=fieldname>" . number_format($value["rows"])     . "</td>";
        echo "<td align=left valign=top class=fieldname>" . size_format($value["size_per_row"]) . "</td>";
		echo "</tr>";
		$i++;
	}

	// Last record is just a summary
	$i = sizeof($db_rec)-1;
	echo "<tr>";
    echo "<td align=left class=heading>" . $db_rec[$i]->name . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->data_size)  . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->index_size) . "</td>";
    echo "<td align=left class=heading>" . size_format($db_rec[$i]->total_size) . "</td>";
    echo "<td align=left class=heading>" . number_format($db_rec[$i]->rows)     . "</td>";
    echo "<td align=left class=heading></td>";
	echo "</tr>";

    echo "</table>";
}


?>
