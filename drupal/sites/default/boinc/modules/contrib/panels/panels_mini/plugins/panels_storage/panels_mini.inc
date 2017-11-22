<?php

/**
 * @file
 * Provides a panels_storage plugin for mini panels.
 */

// Plugin definition
$plugin = array(
  'access callback' => 'panels_mini_panels_storage_access',
);

/**
 * Access callback for panels storage.
 */
function panels_mini_panels_storage_access($storage_type, $storage_id, $op, $account) {
  if ($op == 'create') {
    return user_access('create mini panels', $account);
  }

  return user_access('administer mini panels', $account);
}
