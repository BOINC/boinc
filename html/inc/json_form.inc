<?php

// the following functions return a [value, errmsg] pair.
// The errmsg is null if no error

function get_num($field) {
    $x = get_str($field->name);
    if ($field->type == 'integer') {
        if (filter_var($x, FILTER_VALIDATE_INT) === false) {
            return [null, 'not an integer'];
        }
        $x = intval($x);
    } else {
        if (filter_var($x, FILTER_VALIDATE_FLOAT) === false) {
            return [null, 'not a float'];
        }
        $x = floatval($x);
    }
    if (isset($field->min_value)) {
        if ($x < $field->min_value) {
            return [null, 'value is less than min'];
        }
    }
    if (isset($field->max_value)) {
        if ($x > $field->max_value) {
            return [null, 'value is greater than max'];
        }
    }
    return [$x, null];
}

function get_multi($field) {
    $x = get_array($field->name);
    $n = count($x);
    if (isset($field->min_items)) {
        if ($n < $field->min_items) {
            return [null, 'too few items selected'];
        }
    }
    if (isset($field->max_items)) {
        if ($n > $field->max_items) {
            return [null, 'too many items selected'];
        }
    }
    return [$x, null];
}

// returns either
// - an object containing the field values, or
// - a list of [title, errmsg] pairs
//
function get_inputs($form_desc) {
    $vals = new StdClass;
    $errs = [];
    foreach ($form_desc->fields as $field) {
        switch ($field->type) {
        case 'text':
        case 'select':
        case 'file_select':
            $err = null;
            $val = get_str($field->name);
            break;
        case 'integer':
        case 'float':
            [$val, $err] = get_num($field);
            break;
        case 'select_multi':
        case 'file_select_multi':
            [$val, $err] = get_multi($field);
            break;
        }
        if ($err) {
            $errs[] = [$field->title, $err];
        } else {
            $x = $field->name;
            $vals->$x = $val;
        }
    }
    return [$vals, $errs];
}

?>
