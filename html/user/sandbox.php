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

function list_files($user,$err_msg) {
    $dir = sandbox_dir($user);
    $d = opendir($dir);
    if (!$d) error_page("Can't open sandbox directory");
    page_head("file sandbox for $user->name");
    echo "
        <form action=sandbox.php method=post ENCTYPE=\"multipart/form-data\">
        <input type=hidden name=action value=upload_file>
        Upload a file to your sandbox:
        <p><input size=80 type=file name=new_file>
        <p> <input type=submit value=Upload>
        </form>
        <hr>
    ";
    if(strcmp($err_msg,"")!=0){
	echo "<p>$err_msg<hr>";
    }
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
        table_header("Name<br><span class=note>(click to view)</span>", "Modified", "Size (bytes)", "MD5", "");
        foreach($files as $f) {
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
                )
            );
        }
        end_table();
    }
    page_tail();
}

function upload_file($user) {
    $tmp_name = $_FILES['new_file']['tmp_name'];
    if (is_uploaded_file($tmp_name)) {
        $name = $_FILES['new_file']['name'];
        if (strstr($name, "/")) {
            error_page("no / allowed");
        }
        $md5 = md5_file($tmp_name);
        $s = stat($tmp_name);
        $size = $s['size'];
	list($exist,$elf)=sandbox_lf_exist($user,$md5);
	if($exist){
		$notice="<strong>Notice:</strong> Invalid Upload<br/>";
		$notice.="You are trying to upload file  <strong>$name</strong><br/>";
		$notice.="Another file <strong>$elf</strong> with the same content(md5: $md5) already exist!<br/>";
		list_files($user,$notice);
		return;
	}
	else{
        // move file to download dir
        //
        	$phys_path = sandbox_physical_path($user, $md5);
        	rename($tmp_name, $phys_path);

        // write link file
        //
        	$dir = sandbox_dir($user);
        	$link_path = "$dir/$name";
        	sandbox_write_link_file($link_path, $size, $md5);
	
		$notice="Successfully uploaded file <strong>$name</strong>!<br/>";
		list_files($user,$notice);    
	}
    	
    }
}

function delete_file($user) {
    $name = get_str('name');
    $dir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$dir/$name");
    unlink("$dir/$name");
    if ($error) {
        error_page("can't parse link file");
    }
    $p = sandbox_physical_path($user, $md5);
    if (!is_file($p)) {
        error_page("no such physical file");
    }
    //unlink($p);
    Header("Location: sandbox.php");
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
//print_r($user);
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error_page("no job submission access");

$action = get_str('action', true);
if (!$action) $action = post_str('action', true);

switch ($action) {
case '': list_files($user,""); break;
case 'upload_file': upload_file($user); break;
case 'delete_file': delete_file($user); break;
case 'view_file': view_file($user); break;
default: error_page("no such action: $action");
}

?>
