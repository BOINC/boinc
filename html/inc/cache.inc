<?php

require_once("../project/cache_parameters.inc");

// mechanism for caching commonly-accessed pages

function make_cache_dirs() {
  if (!@filemtime("../cache")) mkdir("../cache");
  for ($i=0;$i<256;$i++) {
    $j=sprintf("%02x",$i);
    if (!@filemtime("../cache/$j")) {
      mkdir("../cache/$j", 0700);
    } 
  }
}

function get_path($params) {
    if (!@filemtime("../cache/00")) make_cache_dirs();
    $y = pathinfo($_SERVER["PHP_SELF"]);
    $z = $y["basename"];
    // add a layer of subdirectories for reducing file lookup time
    $sz = substr(md5($z."_".urlencode($params)),1,2);
    $path = "../cache/".$sz."/".$z;
    if ($params) {
        $path = $path."_".urlencode($params);
    }
    return $path;
}

function disk_usage($dir) {
   $usage=0;
   if ($handle=@opendir($dir)) {
     while ($file=readdir($handle)) {
       if (($file != ".") && ($file != "..")) {
         if (@is_dir($dir."/".$file)) {
	   $usage+=disk_usage($dir."/".$file);
	 } else {
	   $usage+=@filesize($dir."/".$file);
	 }
       }
     }
     @closedir($handle);
   }
   return $usage;
}

function clean_cache($max_age,$dir="../cache") {
   if ($handle=@opendir($dir)) {
     while ($file=readdir($handle)) {
       if (($file != ".") && ($file != "..")) {
         if (@is_dir($dir."/".$file)) {
	   clean_cache($max_age,$dir."/".$file);
	 } else {
	   if ((time()-@filemtime($file))>$max_age) @unlink($file);
	 }
       }
     }
     @closedir($handle);
   }
} 

function start_cache($max_age, $params=""){
    $path = get_path($params);
    // check free disk space every once in a while
    if (!(rand() % CACHE_SIZE_CHECK_FREQ)) {
       $too_old=86400;
       while ((disk_free_space("../cache") < MIN_FREE_SPACE) ||
           (disk_usage("../cache") > MAX_CACHE_USAGE)) {
	   clean_cache($too_old);
	   $too_old/=2;
       }
    }
    if ($max_age) {
      $request = getallheaders();
      // Check to see if this is a conditional fetch.
      $lastmodified = @filemtime($path);

      $if_modified_since = isset($request['If-Modified-Since']) ?
        (explode(';',$request['If-Modified-Since'])) :
        false;

      if ($if_modified_since) {
        $if_modified_since=strtotime($if_modified_since[0]);
      }

      if ($if_modified_since && ($if_modified_since == $lastmodified)) {
        Header("Last-Modified: " . gmdate("D, d M Y H:i:s",$lastmodified) . " GMT");
        Header('HTTP/1.0 304 Not Modified');
        exit;
      }

      if ($lastmodified<time()-$max_age){
         // If cached version is too old (or non-existent)
         // generate the page and write to cache
         //
         ob_start();
         ob_implicit_flush(0);
         Header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
         Header("Expires: " . gmdate("D, d M Y H:i:s",time()+$max_age) . " GMT");
         Header("Cache-Control: public, max-age=" . $max_age); 
       } else {
         // Otherwise serve the cached version and exit
         //
         if (strstr($params, "format=xml")) {
             header('Content-type: text/xml');
         }
         Header("Last-Modified: " . gmdate("D, d M Y H:i:s",$lastmodified) . " GMT");
         Header("Expires: " . gmdate("D, d M Y H:i:s",$lastmodified+$max_age) . " GMT");
         Header("Cache-Control: public, max-age=" . $max_age );
         readfile($path);
         exit;
     }
  }
}

// write output buffer both to client and to cache
//
function end_cache($max_age,$params=""){
   if ($max_age) {
     $path = get_path($params);
     $fhandle=fopen($path, "w");
     $page=ob_get_contents();
     ob_end_flush();
     fwrite($fhandle, $page);
     fclose($fhandle);
  }
}
?>
