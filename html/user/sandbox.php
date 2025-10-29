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

// Per-user "file sandboxes".
// Files are stored in <project>/sandbox/<userid>/
// File infos (md5/size) are scored in a parallel dir
// <project>/sandbox/<userid>/.md5/

// NOTE: PHP's default max file upload size is 2MB.
// To increase this, edit /etc/php.ini, and change, e.g.
//
// upload_max_filesize = 64M
// post_max_size = 64M

require_once("../inc/sandbox.inc");
require_once("../inc/submit_util.inc");

display_errors();

function add_form() {
    page_head('Add files to your sandbox');
    echo "
        There are several ways to add files:
        <p>
        <hr>
        <h3>Upload files from this computer</h3>
        <p>
        NOTE: if you upload text files from Windows,
        they will be given CRLF line endings.
        If they are shell scripts, they won't work on Linux.
        Add shell scripts using 'Add text file' below.
    ";
    form_start('sandbox.php', 'post', 'ENCTYPE="multipart/form-data"');
    form_input_hidden('action', 'upload_file');
    form_general('',
        '<input size=80 type=file name="new_file[]" multiple="multiple">'
    );
    form_submit('Upload');
    form_end();
if (0) {
    echo "
        <form action=sandbox.php method=post ENCTYPE=\"multipart/form-data\">
        <input type=hidden name=action value=upload_file>
        <p><p><input size=80 type=file name=\"new_file[]\" multiple=\"multiple\">
        <p> <input class=\"btn btn-success\" type=submit value=Upload>
        </form>
    ";
}
    echo "
        <hr>
        <h3>Add text file</h3>
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

function list_files($user, $notice=null) {
    $dir = sandbox_dir($user);
    if (!is_dir($dir)) error_page("Can't open sandbox directory");
    page_head("File sandbox");
    if ($notice) {
        echo "<p>$notice<hr>";
    }
    echo "<p>Click a column title to sort on that attribute.<p>\n";
    $fnames = array();
    foreach (scandir($dir) as $f) {
        if ($f[0] == '.') continue;
        $fnames[] = $f;
    }
    if (count($fnames) == 0) {
        echo "Your sandbox is currently empty.";
    } else {
        $files = [];
        foreach ($fnames as $fname) {
            [$md5, $size] = sandbox_parse_info_file($user, $fname);
            $f = new StdClass;
            $f->name = $fname;
            $f->size = $size;
            $f->md5 = $md5;
            $f->date = filemtime("$dir/$fname");
            $files[] = $f;
        }
        $sort_field = get_str('sort_field', true);
        if (!$sort_field) $sort_field = 'name';
        $sort_rev = get_str('sort_rev', true);
        column_sort($files, $sort_field, $sort_rev);

        start_table('table-striped');
        table_header(
            column_sort_header(
                'name',
                'Name',
                'sandbox.php?',
                $sort_field, $sort_rev
            ).'<br><small>(click to view text files)</small>',
            column_sort_header(
                'date',
                'Modified',
                'sandbox.php?',
                $sort_field, $sort_rev
            ),
            column_sort_header(
                'size',
                "Size (bytes)",
                'sandbox.php?',
                $sort_field, $sort_rev
            ),
            "MD5",
            "Delete",
            "Download"
        );
        foreach ($files as $f) {
            $ct = time_str($f->date);
            table_row(
                "<a href=sandbox.php?action=view_file&name=$f->name>$f->name</a>",
                $ct,
                $f->size,
                $f->md5,
                button_text_small(
                    "sandbox.php?action=delete_file&name=$f->name",
                    "Delete"
                ),
                button_text_small(
                    "sandbox.php?action=download_file&name=$f->name",
                    "Download"
                )
            );
        }
        end_table();
    }
    show_button('sandbox.php?action=add_form', 'Add files');
    page_tail();
}

// upload one or more files

function upload_file($user) {
    $notice = "";
    $dir = sandbox_dir($user);
    $count = count($_FILES['new_file']['tmp_name']);
    for ($i=0; $i<$count; $i++) {
        $tmp_name = $_FILES['new_file']['tmp_name'][$i];
        if (!is_uploaded_file($tmp_name)) {
            error_page("$tmp_name is not uploaded file");
        }
        $name = $_FILES['new_file']['name'][$i];
        if (!is_valid_filename($name)) {
            error_page('Invalid filename. '.filename_rules());
        }
        if (file_exists("$dir/$name")) {
            $notice .= "can't upload $name; file exists.<br>";
            continue;
        }
        move_uploaded_file($tmp_name, "$dir/$name");

        // write info file
        //
        [$md5, $size] = get_file_info("$dir/$name");
        write_info_file("$dir/.md5/$name", $md5, $size);

        $notice .= "Uploaded file <strong>$name</strong><br/>";
    }
    list_files($user, $notice);
}

function add_file($user) {
    $dir = sandbox_dir($user);
    $name = post_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
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
    list_files($user, $notice);
}

function get_file($user) {
    $dir = sandbox_dir($user);
    $url = post_str('url');
    if (filter_var($url, FILTER_VALIDATE_URL) === FALSE) {
        error_page('Not a valid URL');
    }
    $fname = basename($url);
    if (!is_valid_filename($fname)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $path = "$dir/$fname";
    if (file_exists($path)) {
        error_page("File $fname exists; delete it first.");
    }
    copy($url, $path);
    $notice = "Fetched file from <strong>$url</strong><br/>";
    list_files($user, $notice);
}

// delete a sandbox file.
//
function delete_file($user) {
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $dir = sandbox_dir($user);
    unlink("$dir/$name");
    unlink("$dir/.md5/$name");
    $notice = "<strong>$name</strong> was deleted from your sandbox<br/>";
    list_files($user, $notice);
}

function download_file($user) {
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $dir = sandbox_dir($user);
    do_download("$dir/$name");
}

function view_file($user) {
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $dir = sandbox_dir($user);
    $path = "$dir/$name";
    if (!is_file($path)) {
        error_page("no such file");
    }
    echo "<pre>\n";
    readfile($path);
    echo "</pre>\n";
}

$user = get_logged_in_user();
if (!has_file_access($user)) error_page("no job submission access");

$action = get_str('action', true);
if (!$action) $action = post_str('action', true);

switch ($action) {
case '': list_files($user); break;
case 'upload_file': upload_file($user); break;
case 'add_file': add_file($user); break;
case 'get_file': get_file($user); break;
case 'delete_file': delete_file($user); break;
case 'download_file': download_file($user); break;
case 'view_file': view_file($user); break;
case 'add_form': add_form($user); break;
default: error_page("no such action: ".htmlspecialchars($action));
}

?>
