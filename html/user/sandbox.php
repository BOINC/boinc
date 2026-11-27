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

///////////// UTILITY FUNCTIONS

function get_files() {
    global $sbdir;
    $fnames = [];
    foreach (scandir($sbdir) as $f) {
        if ($f[0] == '.') continue;
        $fnames[] = $f;
    }
    return $fnames;
}

function get_file_desc($fname) {
    global $sbdir, $user;
    $f = new StdClass;
    [$f->md5, $f->size] = sandbox_parse_info_file($user, $fname);
    $f->name = $fname;
    $f->date = filemtime("$sbdir/$fname");
    return $f;
}

// get list of all tags
//
function get_tags() {
    global $sbdir;
    $tags = [];
    $path = "$sbdir/.tags.json";
    if (file_exists($path)) {
        $tags = json_decode(file_get_contents($path));
    }
    return $tags;
}

function get_file_tags($name) {
    global $sbdir;
    $path = "$sbdir/.tags/$name";
    if (file_exists($path)) {
        return json_decode(file_get_contents($path));
    }
    return [];
}

function put_file_tags($name, $tags) {
    global $sbdir;
    $tdir = "$sbdir/.tags";
    if (!is_dir($tdir)) {
        mkdir($tdir);
    }
    $path = "$tdir/$name";
    file_put_contents($path, json_encode($tags));
}

// return tag item list for form select
//
function tag_items($tags) {
    $items = [];
    foreach ($tags as $tag) {
        $items[] = [$tag, $tag];
    }
    return $items;
}

// PHP replaces . with _ in URL args.  Boo!!!!!
//
function parse_url_args() {
    $args = $_SERVER['QUERY_STRING'] ?? '';
    $params = [];
    foreach (explode('&', $args) as $pair) {
        if ($pair === '') continue;
        $parts = explode('=', $pair, 2);
        $key = urldecode($parts[0]);
        $value = isset($parts[1]) ? urldecode($parts[1]) : '';
        $params[$key] = $value;
    }
    return $params;
}

function delete_file($name) {
    global $sbdir;
    if (!is_valid_filename($name)) {
        error_page('Invalid filename');
    }
    unlink("$sbdir/$name");
    @unlink("$sbdir/.md5/$name");
    @unlink("$sbdir/.tags/$name");
}

///////////// END UTILITY FUNCTIONS

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
    echo form_tokens();
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
    echo form_tokens();
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
    echo form_tokens();
    form_input_text('URL', 'url');
    form_submit('OK');
    form_end();
    page_tail();
}

function list_files($notice=null) {
    global $sbdir;

    $sort_field = get_str('sort_field', true);
    if (!in_array($sort_field, ['name', 'date', 'size'], true)) {
         $sort_field = 'name';
    }
    $sort_rev = get_str('sort_rev', true);

    if (!is_dir($sbdir)) error_page("Can't open sandbox directory");
    page_head_select2("File sandbox");
    if ($notice) {
        echo "<p>$notice<hr>";
    }
    echo '
        Your <a href=https://github.com/BOINC/boinc/wiki/User-file-sandbox>file sandbox</a> stores files for job submission.<p> <p>
    ';
    show_button('sandbox.php?action=add_form', 'Add files');
    echo '<hr>
        You can organize your files by creating "tags"
        and adding them to files.<p> <p>
    ';
    show_button('sandbox.php?action=tags', 'Add or delete tags');
    echo '<p> <p>';

    $tag_url_args = '';
    $tags = get_tags();
    if ($tags) {
        $selected_tags = get_str('filter_tags', true);
        if (!$selected_tags) {
            $selected_tags = [];
        }
        form_start('sandbox.php');
        form_select2_multi('Show only files with tags', 'filter_tags',
            tag_items($tags),
            $selected_tags
        );
        form_input_hidden('sort_field', $sort_field);
        if ($sort_rev) {
            form_input_hidden('sort_rev', 1);
        }
        form_submit('OK');
        form_end();
        foreach ($selected_tags as $tag) {
            $tag_url_args .= 'filter_tags[]='.urlencode($tag).'&';
        }
    }
    echo "<hr>Click column title to sort on that attribute; click arrow to reverse sort.<p>\n";
    $fnames = get_files();
    if (count($fnames) == 0) {
        echo "Your sandbox is currently empty.";
    } else {
        $file_descs = [];
        foreach ($fnames as $fname) {
            $file_descs[] = get_file_desc($fname);
        }
        column_sort($file_descs, $sort_field, $sort_rev);

        form_start('sandbox.php');
        echo form_tokens();
        start_table('table-striped');
        table_header(
            column_sort_header(
                'name',
                'Name',
                "sandbox.php?$tag_url_args",
                $sort_field, $sort_rev
            ).'<br><small>(click for details)</small>',
            column_sort_header(
                'date',
                'Modified',
                "sandbox.php?$tag_url_args",
                $sort_field, $sort_rev
            ),
            [column_sort_header(
                'size',
                "Size (bytes)",
                "sandbox.php?$tag_url_args",
                $sort_field, $sort_rev
            ), 'class="bg-primary text-right"'],
            'Tags',
            'Select file',
        );
        foreach ($file_descs as $f) {
            $ct = time_str($f->date);
            $file_tags = get_file_tags($f->name);
            if (!empty($selected_tags)) {
                if (array_diff($selected_tags, $file_tags)) {
                    continue;
                }
            }
            table_row(
                sprintf(
                    '<a href=sandbox.php?action=file_details&name=%s>%s</a>',
                    $f->name, $f->name
                ),
                $ct,
                [$f->size, 'align=right'],
                implode('<br>', $file_tags),
                sprintf('<input type=checkbox name="%s">', $f->name)
            );
        }

        end_table();
        form_select2_multi('', 'select_tags',
            tag_items($tags)
        );
        form_submit('Add tags to selected files',
            'name="action" value="add_tags_to_files"'
        );
        form_submit('Remove tags from selected files',
            'name="action" value="remove_tags_from_files"'
        );
        echo "<hr>";
        form_submit('Delete selected files',
            'name="action" value="delete_files"',
            true
        );
        form_end();
    }
    page_tail();
}

// upload one or more files

function upload_file() {
    global $sbdir;
    $notice = "";
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
        if (file_exists("$sbdir/$name")) {
            $notice .= "can't upload $name; file exists.<br>";
            continue;
        }
        move_uploaded_file($tmp_name, "$sbdir/$name");

        // write info file
        //
        [$md5, $size] = get_file_info("$sbdir/$name");
        write_info_file("$sbdir/.md5/$name", $md5, $size);

        $notice .= "Uploaded file <strong>$name</strong><br/>";
    }
    list_files($notice);
}

function add_file() {
    global $sbdir;
    $name = post_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    if (!$name) error_page('No name given');
    if (file_exists("$sbdir/$name")) {
        error_page("file $name exists");
    }
    $contents = post_str('contents');
    $contents = str_replace("\r\n", "\n", $contents);
    file_put_contents("$sbdir/$name", $contents);

    [$md5, $size] = get_file_info("$sbdir/$name");
    write_info_file("$sbdir/.md5/$name", $md5, $size);

    $notice = "Added file <strong>$name</strong> ($size bytes)";
    list_files($notice);
}

function get_file() {
    global $sbdir;
    $url = post_str('url');
    if (!is_valid_https_url($url)) {
        error_page('Invalid URL');
    }
    $fname = basename($url);
    if (!is_valid_filename($fname)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $path = "$sbdir/$fname";
    if (file_exists($path)) {
        error_page("File $fname exists; delete it first.");
    }
    copy($url, $path);
    $notice = "Fetched file from <strong>$url</strong><br/>";
    list_files($notice);
}

function download_file() {
    global $sbdir;
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    do_download("$sbdir/$name");
}

function view_file() {
    global $sbdir;
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page('Invalid filename. '.filename_rules());
    }
    $path = "$sbdir/$name";
    if (!is_file($path)) {
        error_page("no such file");
    }
    echo "<pre>\n";
    readfile($path);
    echo "</pre>\n";
}

// show all tags, let user add/remove tags
//
function show_tags($msg) {
    page_head('Tags');
    echo "<p>$msg<hr>";
    $tags = get_tags();
    if ($tags) {
        start_table('table-striped');
        table_header('Tag name', 'Delete tag');
        foreach ($tags as $tag) {
            table_row($tag,
                sprintf('<a href=sandbox.php?action=delete_tag&tag=%s%s>Delete</a>',
                    urlencode($tag), url_tokens()
                )
            );
        }
    } else {
        echo 'No tags.';
    }
    end_table();
    form_start('sandbox.php');
    echo form_tokens();
    form_input_hidden('action', 'add_tag');
    form_input_text('Add tag', 'tag');
    form_submit('OK');
    form_end();
    page_tail();
}

function add_tag() {
    global $sbdir;
    check_tokens();
    $tag = get_str('tag');
    if (!is_valid_filename($tag)) error_page('invalid tag');
    $tags = [];
    $path = "$sbdir/.tags.json";
    if (file_exists($path)) {
        $tags = json_decode(file_get_contents($path));
    }
    if (in_array($tag, $tags)) error_page('tag already exists');
    $tags[] = $tag;
    file_put_contents($path, json_encode($tags));
    show_tags("Added tag $tag.");
}

function delete_tag() {
    global $sbdir;
    check_tokens();
    $tag = get_str('tag');
    $tags = [];
    $path = "$sbdir/.tags.json";
    if (file_exists($path)) {
        $tags = json_decode(file_get_contents($path));
    }
    if (!in_array($tag, $tags)) error_page("tag $tag doesn't exist");
    $tags = array_values(array_diff($tags, [$tag]));
    file_put_contents($path, json_encode($tags));

    // delete tag from files
    //
    $tagdir = "$sbdir/.tags";
    if (is_dir($tagdir)) {
        foreach (scandir($tagdir) as $f) {
            if ($f[0] == '.') continue;
            $path = "$tagdir/$f";
            $tags = json_decode(file_get_contents($path));
            if (in_array($tag, $tags)) {
                $tags = array_values(array_diff($tags, [$tag]));
                file_put_contents($path, json_encode($tags));
            }
        }
    }
    show_tags("Removed tag $tag.");
}

function file_details() {
    global $sbdir;
    $name = urldecode(get_str('name'));
    if (!is_valid_filename($name)) error_page('bad filename');
    $path = "$sbdir/$name";
    if (!file_exists($path)) error_page('no such file');
    page_head("Sandbox file: $name");
    start_table('table-striped');
    $f = get_file_desc($name);
    row2('Modified', time_str($f->date));
    row2('Size', $f->size);
    row2('MD5 hash', $f->md5);
    row2('Tags', implode('<br>', get_file_tags($name)));
    row2('',
        sprintf('%s<p> <p>%s',
            button_text_small(
                sprintf(
                    'sandbox.php?action=download_file&name=%s',
                    $name
                ),
                'Download'
            ),
            button_text_small(
                sprintf(
                    'sandbox.php?action=view_file&name=%s',
                    $name
                ),
                'View text file'
            )
        )
    );
    end_table();
    page_tail();
}

function list_action($add) {
    $files = get_files();
    $tags = array_intersect(get_array('select_tags'), get_tags());
    if (empty($tags)) error_page('No tags selected');
    $params = parse_url_args();
    foreach ($files as $name) {
        if (empty($params[$name])) continue;
        $file_tags = get_file_tags($name);
        foreach ($tags as $tag) {
            if ($add) {
                if (!in_array($tag, $file_tags)) {
                    $file_tags[] = $tag;
                }
            } else {
                $file_tags = array_diff(array_values($file_tags), [$tag]);
            }
        }
        put_file_tags($name, $file_tags);
    }
    list_files($add?'Added tags to files':'Removed tags from files');
}

function delete_files() {
    $files = get_files();
    $params = parse_url_args();
    foreach ($files as $name) {
        if (empty($params[$name])) continue;
        delete_file($name);
    }
    list_files('Deleted files');
}

$user = get_logged_in_user();
if (!has_file_access($user)) error_page("no job submission access");

$sbdir = sandbox_dir($user);
if (!is_dir($sbdir)) {
    mkdir($sbdir);
}

$action = get_str('action', true);
if (!$action) $action = post_str('action', true);

switch ($action) {
case '':
    list_files();
    break;
case 'upload_file':
    check_tokens();
    upload_file();
    break;
case 'add_file':
    check_tokens();
    add_file();
    break;
case 'get_file':
    check_tokens();
    get_file();
    break;
case 'download_file':
    download_file();
    break;
case 'view_file':
    view_file();
    break;
case 'add_form':
    add_form();
    break;
case 'tags':
    show_tags('');
    break;
case 'add_tag':
    add_tag();
    break;
case 'delete_tag':
    delete_tag();
    break;
case 'file_details':
    file_details();
    break;
case 'add_tags_to_files':
    check_tokens();
    list_action(true);
    break;
case 'remove_tags_from_files':
    check_tokens();
    list_action(false);
    break;
case 'delete_files':
    delete_files();
    break;
default:
    error_page("no such action: ".htmlspecialchars($action));
}

?>
