<?php

require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

$db_name = parse_config($config, "<db_name>");

db_init();

admin_page_head("BOINC Database Info");

// if you have other db's just add more get_db_info lines
get_db_info($db_name);

admin_page_tail();

function get_db_info($mydb) 
{
  // Carl grabbed this from the mysql.com boards http://dev.mysql.com/doc/refman/5.0/en/show-table-status.html

  $MB = 1048576;
  $KB = 1024;

  echo "<table cols=2><th>Database</th><th>$mydb</td></table>";
  echo "<table cols=6><th>Table</th><th>Data Size</th><th>Index Size</th><th>Total size</th><th>Total Rows</th><th>Avg. Size per Row</th>";

  $result = mysql_query("SHOW TABLE STATUS FROM $mydb");

/* SQL output

 mysql> show table status from qcnalpha;

| Name                 | Engine | Version | Row_format | Rows    | Avg_row_length | Data_length | Max_data_length  | Index_length | Data_free  | Auto_increment | Create_time         | Update_time         | Check_time          | Collation         | Checksum | Create_options | Comment |

*/

  $gdata  = 0;
  $gindex = 0;
  $gtotal = 0;
  $grows  = 0;

  while($myarr = mysql_fetch_assoc($result)) {
     $total = $myarr["Data_length"]+$myarr["Index_length"];

     // sum grand totals
     $gdata += $myarr["Data_length"];
     $gindex += $myarr["Index_length"];
     $gtotal += $total;
     $grows += $myarr["Rows"];


     echo "<tr><td align=\"center\">";
     echo $myarr["Name"] . "<br/></td>";

     echo "<td align=\"center\">";
     if ( $myarr["Data_length"] < $KB) {
          echo " " . $myarr["Data_length"];
     } elseif ( ($myarr["Data_length"] > $KB) && ($myarr["Data_length"] < $MB) ) {
          printf("%.0fK",($myarr["Data_length"] / $KB) );    
     } elseif ( $myarr["Data_length"] >= $MB) {
          printf("%.2fMB",($myarr["Data_length"] / $MB) );
     }
     echo "</td><td align=\"center\">";

     if ( $myarr["Index_length"] < $KB) {
        echo " ".$myarr["Index_length"];
     } elseif ( ($myarr["Index_length"] > $KB) && ($myarr["Index_length"] < $MB) ) {
        printf("%.0fK",($myarr["Index_length"] / $KB) );    
     } elseif ( $myarr["Index_length"] >= $MB) {
        printf("%.2fMB",($myarr["Index_length"] / $MB) );
     }
     echo "<br/></td><td align=\"center\">";

     if ( $total < $KB) {
        echo " ".$total;
     } elseif ( ($total > $KB) && ($total < $MB) ) {
        printf("%.0fK",($total / $KB) );    
     } elseif ( $total >= $MB) {
        printf("%.2fMB",($total / $MB) );
     }    
     echo "<br/></td><td align=\"center\">";
                
     echo "
       " . $myarr["Rows"]."</td><td align=\"center\">
        ".$myarr["Avg_row_length"]."</td></tr>
      ";
  }

  $gdata /= $MB;
  $gindex /= $MB;
  $gtotal /= $MB;

  echo "<tr><td align=\"center\"><B>Totals:</B></td><td><B>";
  printf("%.2fMB", $gdata);
  echo "</B></td><td align=\"center\"><B>";
  printf("%.2fMB", $gindex);
  echo "</B></td><td align=\"center\"><B>";
  printf("%.2fMB", $gtotal);
  echo "</B></td><td align=\"center\"><B>
    " . $grows . "</B></td><td></td></tr></table><BR><BR>";
}

?>
