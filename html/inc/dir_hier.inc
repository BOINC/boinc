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

// PHP versions of functions in sched/sched_util.cpp

function filename_hash($filename, $fanout) {
    $m = md5($filename);
    $s = substr($m, 1, 7);
    sscanf($s, "%x", $n);
    return sprintf("%x", $n%$fanout);
}

function dir_hier_path($filename, $root, $fanout) {
    $dir = filename_hash($filename, $fanout);
    $dirpath = "$root/$dir";
    if (!is_dir($dirpath)) {
        if (!mkdir($dirpath)) {
            echo "failed to mkdir: $dirpath\n";
        }
    }
    return "$dirpath/$filename";
}

function upload_path($filename) {
    static $upload_dir=null;
    if (!$upload_dir) {
        $config = get_config();
        $upload_dir = parse_config($config, '<upload_dir>');
        $uldl_dir_fanout = parse_config($config, '<uldl_dir_fanout>');
    }
    return dir_hier_path($filename, $upload_dir, $uldl_dir_fanout);
}

function dir_hier_url($filename, $base, $fanout) {
    $dir = filename_hash($filename, $fanout);
    return "$base/$dir/$filename";
}

// we're about to put a file with the given path into the download hier;
// return
// 0 if same file is already there and we don't need to copy
// 1 if file isn't there, need to copy
// -1 if a different file is there
// -2 if a file operation failed
//
// in cases 0 and 1 we make sure a .md5 file is there; create if needed
//
// Assume that if there's a .md5 file, it's correct.
//
function check_download_file($path, $dl_path) {
    $dl_md5_path = "$dl_path.md5";
    $have_md5_file = false;
    $md5 = md5_file($path);
    if ($md5 === FALSE) {
        return -2;
    }
    $size = filesize($path);
    if ($size === FALSE) {
        return -2;
    }

    if (file_exists($dl_md5_path)) {
        $x = file_get_contents($dl_md5_path);
        if ($x === FALSE) {
            return -2;
        }
        $x = explode(" ", $x);
        $dl_md5 = $x[0];
        $dl_size = $x[1];
        $have_md5_file = true;
    }

    if (file_exists($dl_path)) {
        if ($have_md5_file) {
            $s = filesize($dl_path);
            if ($s === FALSE) {
                return -2;
            }
            if ($s == $dl_size && $dl_md5 == $md5) {
                // looks like the right file is there
                //
                return 0;
            }
            return -1;
        } else {
            // missing the .md5 file; need to look at the file
            //
            $m = md5_file($dl_path);
            if ($m === FALSE) {
                return -2;
            }
            if ($m == $md5) {
                if (file_put_contents($dl_md5_path, "$md5 $size\n") === FALSE) {
                    return -2;
                }
                return 0;
            }
            return -1;
        }
    } else {
        if (file_put_contents($dl_md5_path, "$md5 $size\n") === FALSE) {
            return -2;
        }
        return 1;
    }
}

function get_hier_info() {
    static $dl_dir = null;
    static $fanout = null;
    if (!$dl_dir) {
        $conf = get_config();
        $dl_dir = parse_config($conf, "<download_dir>");
        $fanout = parse_config($conf, "<uldl_dir_fanout>");
    }
    return [$dl_dir, $fanout];
}

function download_hier_path($fname) {
    [$dl_dir, $fanout] = get_hier_info();
    return dir_hier_path($fname, $dl_dir, $fanout);
}

// Stage (move, not copy) the given file,
// which is assumed to not be in download dir already.
//
function stage_file_basic($dir, $fname) {
    [$dl_dir, $fanout] = get_hier_info();
    $old_path = "$dir/$fname";
    $new_path = download_hier_path($fname);
    $md5 = md5_file($old_path);
    $size = filesize($old_path);
    file_put_contents("$new_path.md5", "$md5 $size\n");
    rename($old_path, $new_path);
}

// copy the given file (with given md5/size)
// to the download dir w/ given phys name; and create .md5 file
// If phys name is already there, do nothing.
//
function stage_file_aux($path, $md5, $size, $phys_name) {
    [$dl_dir, $fanout] = get_hier_info();
    $phys_path = dir_hier_path($phys_name, $dl_dir, $fanout);
    if (file_exists($phys_path)) return;
    copy($path, $phys_path);
    file_put_contents("$phys_path.md5", "$md5 $size\n");
}

?>
