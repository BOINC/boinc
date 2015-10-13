<?php
// $Id$

/**
 * @file hook_diff() implementations for CCK (especially fields).
 *
 * These should use a field-hook so the data for the diff is
 * field-type specific.
 */

/**
 * Implementation of hook_diff()
 */
function content_diff($old_node, $new_node) {
  $result = array();
  // Prevent against invalid 'nodes' built by broken 3rd party code.
  if (isset($new_node->type)) {
    $type = content_types($new_node->type);
    $field_types = _content_field_types();
    foreach ($type['fields'] as $field) {
      // Ignore fields the current user is not allowed to view.
      if (!content_access('view', $field, NULL, $new_node)) {
        continue;
      }
      $function = $field_types[$field['type']]['module'] . '_content_diff_values';
      $function = function_exists($function) ? $function : 'content_content_diff_values';
      $old_values = array();
      $new_values = array();
      if (isset($old_node->$field['field_name'])) {
        $old_values = $function($old_node, $field, $old_node->$field['field_name']);
      }
      if (isset($new_node->$field['field_name'])) {
        $new_values = $function($new_node, $field, $new_node->$field['field_name']);
      }
      if ($old_values || $new_values) {
        $result[$field['field_name']] = array(
          '#name' => $field['widget']['label'],
          '#old' => $old_values,
          '#new' => $new_values,
          '#weight' => $field['widget']['weight'],
          '#format' => array(
            'show_header' => FALSE,
          ),
        );
      }
    }
  }
  return $result;
}

/**
 * Default 'implementation' of hook_content_diff_values.
 *
 * Note that diff.module takes care of running check_plain on the output.
 */
function content_content_diff_values($node, $field, $items) {
  $return = array();
  foreach ($items as $item) {
    foreach (explode("\n", $item['value']) as $i) {
      $return[] = $i;
    }
  }
  return $return;
}

if (module_exists('userreference')) {
  /**
   * Implementation of hook_content_diff_values.
   */
  function userreference_content_diff_values($node, $field, $items) {
    static $titles = array();
    // Gather ids.
    $ids = array();
    foreach ($items as $item) {
      if ($item['uid'] && is_numeric($item['uid'])) {
        $ids[] = $item['uid'];
      }
    }
    // Fetch titles we don't know yet.
    $queried_ids = array_diff($ids, array_keys($titles));
    if ($queried_ids) {
      $result = db_query('SELECT uid, name FROM {users} WHERE uid IN ('. db_placeholders($queried_ids) .')', $queried_ids);
      while ($row = db_fetch_array($result)) {
        $titles[$row['uid']] = $row['name'];
      }
    }
    // Return result.
    $return = array();
    foreach ($items as $item) {
      if ($item['uid'] && isset($titles[$item['uid']])) {
        $return[] = $titles[$item['uid']];
      }
    }
    return $return;
  }
}

if (module_exists('nodereference')) {
  /**
   * Implementation of hook_content_diff_values.
   */
  function nodereference_content_diff_values($node, $field, $items) {
    static $titles = array();
    // Gather ids.
    $ids = array();
    foreach ($items as $item) {
      if ($item['nid'] && is_numeric($item['nid'])) {
        $ids[] = $item['nid'];
      }
    }
    // Fetch titles we don't know yet.
    $queried_ids = array_diff($ids, array_keys($titles));
    if ($queried_ids) {
      $result = db_query('SELECT nid, title FROM {node} WHERE nid IN ('. db_placeholders($queried_ids) .')', $queried_ids);
      while ($row = db_fetch_array($result)) {
        $titles[$row['nid']] = $row['title'];
      }
    }
    // Return result.
    $return = array();
    foreach ($items as $item) {
      if ($item['nid'] && isset($titles[$item['nid']])) {
        $return[] = $titles[$item['nid']];
      }
    }
    return $return;
  }
}
