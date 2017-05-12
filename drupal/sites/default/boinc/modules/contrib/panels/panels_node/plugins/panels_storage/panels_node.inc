<?php

/**
 * @file
 * Provides a panels_storage plugin for panels node.
 */

// Plugin definition
$plugin = array(
  'access callback' => 'panels_node_panels_storage_access',
);

/**
 * Access callback for panels storage.
 */
function panels_node_panels_storage_access($storage_type, $storage_id, $op, $account) {
  if ($node = node_load($storage_id)) {
    if ($op == 'read') {
      $op = 'view';
    }
    return node_access($op, $node, $account);
  }

  return FALSE;
}
