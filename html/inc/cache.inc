<?

// mechanism for caching commonly-accessed pages

function start_cache($max_age, $unique_parameter_string){
   $cache_dir="../cache";

   $path = $cache_dir.$_SERVER["PHP_SELF"]."_".urlencode($unique_parameter_string);
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
function end_cache($unique_parameter_string){
   $cache_dir="../cache";

   $path = $cache_dir.$_SERVER["PHP_SELF"]."_".urlencode($unique_parameter_string);
   $fhandle=fopen($path, "w");
   $page=ob_get_contents();
   ob_end_flush();
   fwrite($fhandle, $page);
   fclose($fhandle);
}

?>
