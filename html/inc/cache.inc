<?php

require_once("../project/cache_parameters.inc");

// mechanism for caching commonly-accessed pages

function make_cache_dirs() {
  if (!@filemtime("../cache")) mkdir("../cache");
  for ($i=0;$i<256;$i++) {
    $j=sprintf("%02x",$i);
    if (!@filemtime("../cache/$j")) {
      mkdir("../cache/$j");
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

function start_cache($max_age, $params=""){
    $path = get_path($params);
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

// write output buffer both to client and to cache
//
function end_cache($params=""){
   $path = get_path($params);
   $fhandle=fopen($path, "w");
   $page=ob_get_contents();
   ob_end_flush();
   fwrite($fhandle, $page);
   fclose($fhandle);
}

?>
