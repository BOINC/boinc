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

function list_files($user, $err_msg) {
    $dir = sandbox_dir($user);
    $d = opendir($dir);
    if (!$d) error_page("Can't open sandbox directory");
    page_head("File sandbox");
    echo "
        <form action=sandbox.php method=post ENCTYPE=\"multipart/form-data\">
        <input type=hidden name=action value=upload_file>
        Upload files to your sandbox:
        <p><input size=80 type=file name=\"new_file[]\" multiple=\"multiple\">
        <p> <input class=\"btn btn-default\" type=submit value=Upload>
        </form>
        <hr>
    ";

    form_start('sandbox.php', 'post');
    form_input_hidden('action', 'add_file');
    form_input_text('Name', 'name');
    form_input_textarea('Contents', 'contents');
    form_submit('OK');
    form_end();
    echo "
        <hr>
        <h3>Get web file</h3>
    ";
    form_start('sandbox.php', 'post');
    form_input_hidden('action', 'get_file');
    form_input_text('URL', 'url');
    form_submit('OK');
    form_end();
    page_tail();
}

function list_files($user) {
    $dir = sandbox_dir($user);
    if (!is_dir($dir)) error_page("Can't open sandbox directory");
    page_head("File sandbox");
    $notice = htmlspecialchars(get_str('notice', true));
    if ($notice) {
        echo "<p>$notice<hr>";
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
        list($exist, $elf) = sandbox_lf_exist($user, $md5);
        if ($exist){
            $notice .= "<strong>Notice:</strong> Invalid Upload<br/>";
            $notice .= "You are trying to upload file  <strong>$name</strong><br/>";
            $notice .= "Another file <strong>$elf</strong> with the same content (md5: $md5) already exists!<br/>";
        } else {
            // move file to download dir
            //
            $phys_path = sandbox_physical_path($user, $md5);
            rename($tmp_name, $phys_path);

            // write link file
            //
            $dir = sandbox_dir($user);
            $link_path = "$dir/$name";
            sandbox_write_link_file($link_path, $size, $md5);
            $notice .= "Uploaded file <strong>$name</strong><br/>";
        }
    }
    list_files($user, $notice);
}

function add_file($user) {
    $dir = sandbox_dir($user);
    $name = post_str('name');
    if (!is_valid_filename($name)) {
        error_page('bad filename');
    }
    if (!$name) error_page('No name given');
    if (file_exists("$dir/$name")) {
        error_page("file $name exists");
    }
    $contents = post_str('contents');
    $contents = str_replace("\r\n", "\n", $contents);
    file_put_contents("$dir/$name", $contents);

    [$md5, $size] = get_file_info("$dir/$name");
    write_info_file("$dir/.md5/$name", $md5, $size);

    $notice = "Added file <strong>$name</strong> ($size bytes)";
    header(sprintf('Location: sandbox.php?notice=%s', urlencode($notice)));
}

function get_file($user) {
    $dir = sandbox_dir($user);
    $url = post_str('url');
    if (filter_var($url, FILTER_VALIDATE_URL) === FALSE) {
        error_page('Not a valid URL');
    }
    $fname = basename($url);
    $path = "$dir/$fname";
    if (file_exists($path)) {
        error_page("File $fname exists; delete it first.");
    }
    copy($url, $path);
    $notice = "Fetched file from <strong>$url</strong><br/>";
    header(sprintf('Location: sandbox.php?notice=%s', urlencode($notice)));
}

// delete a sandbox file.
//
function delete_file($user) {
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('bad filename');
    }
    $dir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$dir/$name");
    if ($error) {
        error_page("can't parse link file");
    }
    $p = sandbox_physical_path($user, $md5);
    if (!is_file($p)) {
        error_page("no such physical file");
    }
    $bused = sandbox_file_in_use($user, $name);
    if ($bused){
        $notice = "<strong>$name</strong> is being used by batch(es), you can not delete it now!<br/>";
    } else{ 
        $notice = "<strong>$name</strong> is not being used by any batch(es) and successfully deleted from your sandbox<br/>";
        unlink("$dir/$name");
        unlink($p);
    
    }
    list_files($user,$notice);
    //Header("Location: sandbox.php");
}
function download_file($user) {
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('bad filename');
    }
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
    if (!is_valid_filename($name)) {
        error_page('bad filename');
    }
    $dir = sandbox_dir($user);
    $path = "$dir/$name";
    if (!is_file($path)) {
        error_page("no such file");
    }
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
case 'add_form': add_form($user); break;
default: error_page("no such action: ".htmlspecialchars($action));
}

?>
