<?php

// generate a form from a JSON spec

require_once('../inc/util.inc');
require_once('../inc/sandbox.inc');

function sb_file_select($user, $suffix=null) {
    $sbfiles = sandbox_file_names($user);
    $sbitems = [];
    $exp = null;
    if ($suffix) {
        $exp = sprintf('/%s$/', $suffix);
    }
    foreach ($sbfiles as $f) {
        if ($exp && !preg_match($exp, $f)) continue;
        $sbitems[] = [$f, $f];
    }
    return $sbitems;
}

function form($user, $json_fname) {
    $form_desc = json_decode(file_get_contents($json_fname));
    page_head_select2($form_desc->title);
    form_start($form_desc->handler);
    foreach ($form_desc->fields as $field) {
        switch ($field->type) {
        case 'text':
        case 'integer':
        case 'float':
            form_input_text($field->title, $field->name);
            break;
        case 'file_select':
            $items = sb_file_select($user, $field->suffix);
            form_select($field->title, $field->name, $items);
            break;
        case 'file_select_multi':
            $items = sb_file_select($user, $field->suffix);
            form_select2_multi(
                $field->title, $field->name, $items, null, "id=$field->name"
            );
            break;
        case 'select':
            form_select($field->title, $field->name, $field->items);
            break;
        case 'select_multi':
            form_select2_multi(
                $field->title, $field->name, $field->items,
                null, "id=$field->name"
            );
            break;
        case 'radio':
            break;
        }
    }
    form_submit('OK');
    form_end();
    page_tail();
}

$json_fname = get_str('json_fname');
$user = get_logged_in_user();
form($user, $json_fname);
?>
