<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function private_messages_user_default_permissions() {
  $permissions = array();

  // Exported permission: administer privatemsg settings
  $permissions['administer privatemsg settings'] = array(
    'name' => 'administer privatemsg settings',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete privatemsg
  $permissions['delete privatemsg'] = array(
    'name' => 'delete privatemsg',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: read all private messages
  $permissions['read all private messages'] = array(
    'name' => 'read all private messages',
    'roles' => array(),
  );

  // Exported permission: read privatemsg
  $permissions['read privatemsg'] = array(
    'name' => 'read privatemsg',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: write privatemsg
  $permissions['write privatemsg'] = array(
    'name' => 'write privatemsg',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
    ),
  );

  return $permissions;
}
