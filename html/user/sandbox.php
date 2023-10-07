<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// Per-user "file sandboxes" for job submission.
// These are stored in project-root/sandbox/USERID/
//
// The entries in a sandbox directory have contents
// size md5
//
// The actual files are stored in the download hierarchy,
// with sb_userid_MD5 as the physical name

// NOTE: PHP's default max file upload size is 2MB.
// To increase this, edit /etc/php.ini, and change, e.g.
//
// upload_max_filesize = 64M
// post_max_size = 64M

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/sandbox.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/submit_util.inc");

function list_files($user, $notice) {
    $dir = sandbox_dir($user);
    $d = opendir($dir);
    if (!$d) error_page("Can't open sandbox directory");
    page_head("File sandbox");
    if ($notice) {
        echo "<p>$notice<hr>";
    }
    echo "
        <p>
        <form action=sandbox.php method=post ENCTYPE=\"multipart/form-data\">
        <input type=hidden name=action value=upload_file>
        Upload files to your sandbox:
        <p><p><input size=80 type=file name=\"new_file[]\" multiple=\"multiple\">
        <p> <input class=\"btn btn-success\" type=submit value=Upload>
        </form>
        <hr>
    ";
    $files = array();
    while (($f = readdir($d)) !== false) {
        if ($f == '.') continue;
        if ($f == '..') continue;
        $files[] = $f;
    }
    if (count($files) == 0) {
        echo "Your sandbox is currently empty.";
    } else {
        sort($files);
        start_table();
        table_header("Name<br><p class=\"text-muted\">(click to view)</p>", "Modified", "Size (bytes)", "MD5", "Delete","Download");
        foreach ($files as $f) {
            $path = "$dir/$f";
            list($error, $size, $md5) = sandbox_parse_link_file($path);
            if ($error) {
                table_row($f, "Can't parse link file", "", "<a href=sandbox.php?action=delete_files&name=$f>delete</a>");
                continue;
            }
            $p = sandbox_physical_path($user, $md5);
            if (!is_file($p)) {
                table_row($f, "Physical file not found", "", "");
                continue;
            }
            $ct = time_str(filemtime($path));
            table_row(
                "<a href=sandbox.php?action=view_file&name=$f>$f</a>",
                $ct,
                $size,
                $md5,
                button_text(
                    "sandbox.php?action=delete_file&name=$f",
                    "Delete"
                ),
                button_text(
                    "sandbox.php?action=download_file&name=$f",
                    "Download"
                )
            );
        }
        end_table();
    }
    page_tail();
}

function upload_file($user) {
    $notice = "";
    $count = count($_FILES['new_file']['tmp_name']);
    for ($i = 0; $i < $count; $i++) {
        $tmp_name = $_FILES['new_file']['tmp_name'][$i];
        if (!is_uploaded_file($tmp_name)) {
            error_page("$tmp_name is not uploaded file");
        }
        $name = $_FILES['new_file']['name'][$i];
        if (strstr($name, "/")) {
            error_page("no / allowed");
        }
        $md5 = md5_file($tmp_name);
        $s = stat($tmp_name);
        $size = $s['size'];
        [$exists, $elf] = sandbox_lf_exists($user, $md5);
        if (!$exists){
            // move file to download dir
            //
            $phys_path = sandbox_physical_path($user, $md5);
            move_uploaded_file($tmp_name, $phys_path);
        }

        // write link file
        //
        $dir = sandbox_dir($user);
        $link_path = "$dir/$name";
        sandbox_write_link_file($link_path, $size, $md5);
        $notice .= "Uploaded file <strong>$name</strong><br/>";
    }
    list_files($user, $notice);
}

// delete a link to a file.
// check if currently being used by a batch.
// If the last link w/ that contents, delete the file itself
//
function delete_file($user) {
    $name = get_str('name');
    $dir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$dir/$name");
    if ($error) {
        error_page("can't parse link file");
    }
    $p = sandbox_physical_path($user, $md5);
    if (!is_file($p)) {
        error_page("physical file is missing");
    }
    $bused = sandbox_file_in_use($user, $name);
    if ($bused){
        $notice = "<strong>$name</strong> is being used by batch(es), you can not delete it now!<br/>";
    } else{
        unlink("$dir/$name");
        $notice = "<strong>$name</strong> was deleted from your sandbox<br/>";
        [$exists, $elf] = sandbox_lf_exists($user, $md5);
        if (!$exists) {
            unlink($p);
        }

    }
    list_files($user, $notice);
    //Header("Location: sandbox.php");
}
function download_file($user) {
    $name = get_str('name');
    $dir = sandbox_dir($user);
    list($err, $size, $md5) = sandbox_parse_link_file("$dir/$name");
    if ($err) {
        error_page("can't parse link file");
    }
    $p = sandbox_physical_path($user, $md5);
    if (!is_file($p)) {
        error_page("$p does not exist!");
    }
    do_download($p, $name);
}
function view_file($user) {
    $name = get_str('name');
    $dir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$dir/$name");
    if ($error) error_page("no such link file");
    $p = sandbox_physical_path($user, $md5);
    if (!is_file($p)) error_page("no such physical file");
    echo "<pre>\n";
    readfile($p);
    echo "</pre>\n";
}

$user = get_logged_in_user();
if (!submit_permissions($user)) error_page("no job submission access");

$action = get_str('action', true);
if (!$action) $action = post_str('action', true);

switch ($action) {
case '': list_files($user,""); break;
case 'upload_file': upload_file($user); break;
case 'delete_file': delete_file($user); break;
case 'download_file': download_file($user); break;
case 'view_file': view_file($user); break;
default: error_page("no such action: $action");
}

?>
