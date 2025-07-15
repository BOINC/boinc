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

// mechanism for caching commonly-accessed pages

require_once("../project/cache_parameters.inc");

// If we can't see request headers, don't do caching
//
$no_cache = false;
if (!function_exists("apache_request_headers")) {
    $no_cache = true;
}

function make_cache_dirs() {
    if (!@filemtime("../cache")) {
        mkdir("../cache", 0770);
        chmod("../cache", 0770);
    }
    for ($i=0;$i<256;$i++) {
        $j=sprintf("%02x",$i);
        if (!@filemtime("../cache/$j")) {
            mkdir("../cache/$j", 0770);
            chmod("../cache/$j", 0770);
        }
    }
}

function get_path($params, $phpfile=null) {
    if (!@filemtime("../cache/00")) make_cache_dirs();
    if ($phpfile) {
        $z = $phpfile;
    } else {
        $y = pathinfo($_SERVER["PHP_SELF"]);
        $z = $y["basename"];
    }

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

function clean_cache($max_age, $dir) {
    $start_dir = getcwd();
    if (!chdir($dir)) {
        return;
    }
    if ($handle=@opendir(".")) {
        while ($file=readdir($handle)) {
            if ($file == ".") continue;
            if ($file == "..") continue;

            // don't let hackers trick us into deleting other files!
            if (strstr($file, "..")) {
                continue;
            }
            if (@is_dir($file)) {
                clean_cache($max_age, $file);
            } else {
                if ((time()-@filemtime($file))>$max_age) {
                    //echo "unlinking ".getcwd()."/$file\n";
                    @unlink($file);
                }
            }
        }
       @closedir($handle);
    }
    chdir($start_dir);
}


// check cache size every once in a while, purge if too big
//
function cache_check_diskspace(){
    if ((rand() % CACHE_SIZE_CHECK_FREQ)) return;
    if (disk_usage("../cache") < MAX_CACHE_USAGE) return;
    $x = max(TEAM_PAGE_TTL, USER_PAGE_TTL, USER_HOST_TTL,
        USER_PROFILE_TTL, TOP_PAGES_TTL, INDEX_PAGE_TTL
    );
    clean_cache($x, "../cache");
}

function cache_need_to_regenerate($path, $max_age){
    $regenerate = false;
    $request = apache_request_headers();

    clearstatcache();
    $lastmodified = @filemtime($path);
    if ($lastmodified) {
        // See if cached copy is too old.
        // If so regenerate,
        // and touch the cached copy so other processes
        // don't regenerate at the same time
        //
        if ($lastmodified<time()-$max_age) {
            $regenerate = true;
            @touch($path);
        }
    } else {
        $regenerate = true;
    }
    return $regenerate;
}

// Returns cached data or false if nothing was found
function get_cached_data($max_age, $params=""){
    global $no_cache;

    if ($no_cache) return false;

    $path = get_path($params);
    if ($max_age) {
        if (defined('MEMCACHE_SERVERS')) {
            $cache = BoincMemcache::get()->get($path);
            if (isset($cache['content'])) {
                return $cache['content'];
            } else {
                return $cache;
            }
        } else {
            cache_check_diskspace();
            $regenerate=cache_need_to_regenerate($path, $max_age);
            if (!$regenerate) {
                return file_get_contents($path);
            }
        }
    }
    return false; //No data was cached, just return
}

// DEPRECATED
function start_cache($max_age, $params=""){
    global $no_cache, $caching, $memcache;

    if ($no_cache) return;
    $caching = true;

    if ($max_age) {
        $path = get_path($params);
        $cache = null;
        if (defined('MEMCACHE_SERVERS')) {
            $cache = BoincMemcache::get()->get($path);
            if ($cache) {
                $regenerate = false;
                $lastmodified = abs($cache->timestamp);
            } else {
                $regenerate = true;
            }
        } else {
            $lastmodified = @filemtime($path);
            cache_check_diskspace(); //Check free disk space once in a while
            $regenerate = cache_need_to_regenerate($path, $max_age);
        }
        //Is the stored version too old, do we need to regenerate it?
        if ($regenerate){
            // If cached version is too old (or non-existent)
            // generate the page and write to cache
            //
            ob_start();
            ob_implicit_flush(0);
            Header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
            Header("Expires: " . gmdate("D, d M Y H:i:s",time()+$max_age) . " GMT");
            Header("Cache-Control: public, max-age=" . $max_age);

            // allow the calling page to see cache period
            //
            global $cached_max_age;
            $cached_max_age = $max_age;
        } else {
            // Otherwise serve the cached version and exit
            //
            if (strstr($params, "format=xml")) {
                header('Content-type: text/xml');
            }
            Header("Last-Modified: " . gmdate("D, d M Y H:i:s",$lastmodified) . " GMT");
            Header("Expires: " . gmdate("D, d M Y H:i:s",$lastmodified+$max_age) . " GMT");
            Header("Cache-Control: public, max-age=" . $max_age );
            if ($cache && $cache->content) {
                echo $cache->content;
                exit;
            }
            if (!@readfile($path)) {
                //echo "can't read $path; lastmod $lastmodified\n";
                @unlink($path);
                //Proceed to regenerate content
            } else {
                exit;
            }
        }
    }
}

// write output buffer both to client and to cache
// DEPRECATED
function end_cache($max_age,$params=""){
    global $no_cache;
    if ($no_cache) return;

    // for the benefit of hackers
    if (strstr($params, "..")) {
        return;
    }
    if ($max_age) {
        $path = get_path($params);

        if (defined('MEMCACHE_SERVERS')) {
            $cache = array('content' => ob_get_contents(), 'timestamp' => time());
            ob_end_flush();
            $cache = BoincMemcache::get()->set($path, $cache, $max_age);
        } else {
            $fhandle = fopen($path, "w");
            $page = ob_get_contents();
            ob_end_flush();
            fwrite($fhandle, $page);
            fclose($fhandle);
        }
    }
}

function set_cached_data($max_age, $data, $params=""){
    // for the benefit of hackers
    if (strstr($params, "..")) {
        return "bad params";
    }
    $path = get_path($params);
    if (defined('MEMCACHE_SERVERS')) {
        $cache = array('content' => $data, 'timestamp' => time());
        BoincMemcache::get()->set($path, $cache, $max_age);
    } else {
        $fhandle = @fopen($path, "w");
        if (!$fhandle) {
            return "can't open $path";
        }
        fwrite($fhandle, $data);
        fclose($fhandle);
    }
    return "";
}

function clear_cache_entry($phpfile, $params) {
    if (strstr($phpfile, "..")) {
        return;
    }
    if (strstr($params, "..")) {
        return;
    }
    $path = get_path($params, $phpfile);
    @unlink($path);
}

// Memcached class
class BoincMemcache {
    static $instance;

    static function get() {
        self::$instance = new Memcached;
        if (defined('MEMCACHE_PREFIX')) {
            self::$instance->setOption(Memcached::OPT_PREFIX_KEY, MEMCACHE_PREFIX);
        }
        $server_arr = array();
        $servers = explode('|', MEMCACHE_SERVERS);
        foreach($servers as &$server) {
            list($ip, $port, $weight) = explode(':', $server);
            if (!$port) { $port = 11211; }
            $server_arr[] = array($ip, $port, $weight);
        }
        self::$instance->addServers($server_arr);
        return self::$instance;
    }
}

?>
