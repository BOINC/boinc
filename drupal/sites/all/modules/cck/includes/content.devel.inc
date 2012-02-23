<?php
// $Id: content.devel.inc,v 1.4.2.6 2009/01/20 07:31:30 karens Exp $

/**
 * @file
 * Functions needed for Devel module integration.
 * 
 * TODO
 * This is not really working correctly yet. It is getting called once
 * for every field and then generating every field each time. But if
 * you only process one field at a time the earlier ones lose their values.
 * The current method works to create values, but is overly processor-
 * intensive and needs to be reworked in a way that each field is 
 * only processed once and all values are retained.
 */

/**
 * Enrich the $node that is about to be saved with arbitrary
 * information in each of its CCK fields.
 **/
function content_generate_fields(&$node, $field) {
  $type_name = $node->type;
  $type = content_types($type_name);
  $field_types = _content_field_types();

  if (!empty($type['fields'])) {
    foreach ($type['fields'] as $field) {
      $node_field = array();
      // If module handles own multiples, then only call its hook once.
      if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
        $max = 0;
      }
      else {
        switch ($field['multiple']) {
          case 0:
            $max = 0;
            break;
          case 1:
            $max = rand(0, 3); //just an arbitrary number for 'unlimited'
            break;
          default:
            $max = $field['multiple'];
            break;
        }
      }
      for ($i = 0; $i <= $max; $i++) {
        $module = $field_types[$field['type']]['module'];
        $function = $module .'_content_generate';
        if (function_exists($function)) {
          $result = $function($node, $field); // $items, $teaser, $page
          if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
            // Fields that handle their own multiples will add their own deltas.
            $node_field = $result;
          }
          else {
            // When multiples are handled by the content module, add a delta for each result.
            $node_field[$i] = $result;
          }
        }
      }
      $node->{$field['field_name']} = $node_field;
    }
  }
}

/**
 * A simple function to return multiple values for fields that use
 * custom multiple value widgets but don't need any other special multiple
 * values handling. This will call the field generation function
 * a random number of times and compile the results into a node array.
 */
function content_devel_multiple($function, $node, $field) {
  $node_field = array();
  if (function_exists($function)) {
    switch ($field['multiple']) {
      case 0:
        $max = 0;
        break;
      case 1:
        $max = rand(0, 3); //just an arbitrary number for 'unlimited'
        break;
      default:
        $max = $field['multiple'];
        break;
    }
    for ($i = 0; $i <= $max; $i++) {
      $node_field[$i] = $function($node, $field);
    }
  }
  return $node_field;
}

if (module_exists('text')) {
  function text_content_generate($node, $field) {
    if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
      return content_devel_multiple('_text_content_generate', $node, $field);
    }
    else {
      return _text_content_generate($node, $field);
    }
  }

  function _text_content_generate($node, $field) {
    $node_field = array();
    if ($field['widget']['type'] == 'text_textarea') {
      $format = $field['text_processing'] ? rand(0, 3) : 0;
      $node_field['value'] = devel_create_content($format);
      $node_field['format'] = $format;
    }
    else {
      $allowed_values = content_allowed_values($field);
      if (!empty($allowed_values)) {
        // Just pick one of the specified allowed values.
        $node_field['value'] = array_rand($allowed_values);
      }
      else {
        // Generate a value that respects max_length.
        if (empty($field['max_length'])) {
          $field['max_length'] = 12;
        }
        $node_field['value'] = user_password($field['max_length']);
      }
    }
    return $node_field;
  }
}

if (module_exists('number')) {
  function number_content_generate($node, $field) {
    if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
      return content_devel_multiple('_number_content_generate', $node, $field);
    }
    else {
      return _number_content_generate($node, $field);
    }
  }

  function _number_content_generate($node, $field) {
    $node_field = array();
    $allowed_values = content_allowed_values($field);
    if (!empty($allowed_values)) {
      // Just pick one of the specified allowed values.
      $node_field['value'] = array_rand($allowed_values);
    }
    else {
      $min = is_numeric($field['min']) ? $field['min'] : 0;
      switch ($field['type']) {
        case 'number_integer':
          $max = is_numeric($field['max']) ? $field['max'] : 10000;
          $decimal = 0;
          $scale = 0;
          break;

        case 'number_decimal':
          $precision = is_numeric($field['precision']) ? $field['precision'] : 10;
          $scale = is_numeric($field['scale']) ? $field['scale'] : 2;
          $max = is_numeric($field['max']) ? $field['max'] : pow(10, ($precision - $scale));
          $decimal = rand(0, (10 * $scale)) / 100;
          break;

        case 'number_float':
          $precision = rand(10, 32);
          $scale = rand(0, 2);
          $decimal = rand(0, (10 * $scale)) / 100;
          $max = is_numeric($field['max']) ? $field['max'] : pow(10, ($precision - $scale));
          break;
      }
      $node_field['value'] = round((rand($min, $max) + $decimal), $scale);
    }
    return $node_field;
  }
}

if (module_exists('nodereference')) {
  function nodereference_content_generate($node, $field) {
    if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
      return content_devel_multiple('_nodereference_content_generate', $node, $field);
    }
    else {
      return _nodereference_content_generate($node, $field);
    }
  }

  function _nodereference_content_generate($node, $field) {
    $node_field = array();
    $allowed_values = nodereference_allowed_values($field);
    unset($allowed_values[0]);
    if (!empty($allowed_values)) {
      // Just pick one of the specified allowed values.
      $node_field['nid'] = array_rand($allowed_values);
    }
    return $node_field;
  }
}

if (module_exists('userreference')) {
  function userreference_content_generate($node, $field) {
    if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_MODULE) {
      return content_devel_multiple('_userreference_content_generate', $node, $field);
    }
    else {
      return _userreference_content_generate($node, $field);
    }
  }

  function _userreference_content_generate($node, $field) {
    $node_field = array();
    $allowed_values = userreference_allowed_values($field);
    if (isset($allowed_values['none'])) {
      unset($allowed_values['none']);
    }
    if (!empty($allowed_values)) {
      // Just pick one of the specified allowed values.
      $node_field['uid'] = array_rand($allowed_values);
    }
    return $node_field;
  }
}