<?php

// mechanism for caching commonly-accessed pages

function get_path($params) {
    $y = pathinfo($_SERVER["PHP_SELF"]);
    $z = $y["basename"];
    $path = "../cache/".$z;
    if ($params) {
        $path = $path."_".urlencode($params);
    }
    return $path;
}

function start_cache($max_age, $params=""){
    $path = get_path($params);

   if (@filemtime($path)<time()-$max_age){
       // If cached version is too old (or non-existent)
       // generate the page and write to cache
       //
       ob_start();
       ob_implicit_flush(0);
   } else {
       // Otherwise serve the cached version and exit
       //
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
