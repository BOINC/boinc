<?php

/**
 * @file
 * Provides a panels_storage plugin for page_manager.
 */

// Plugin definition
$plugin = array(
  'access callback' => 'page_manager_panels_storage_access',
);

/**
 * Access callback for panels storage.
 */
function page_manager_panels_storage_access($storage_type, $storage_id, $op, $account) {
  // Only users with the 'use page manager' or administer page manager perms.
  return user_access('use page manager', $account) || user_access('administer page manager', $account);
}
