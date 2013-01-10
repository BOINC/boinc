<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function friends_user_default_permissions() {
  $permissions = array();

  // Exported permission: receive friend email notification
  $permissions['receive friend email notification'] = array(
    'name' => 'receive friend email notification',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  return $permissions;
}
