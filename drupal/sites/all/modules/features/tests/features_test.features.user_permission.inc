<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function features_test_user_default_permissions() {
  $permissions = array();

  // Exported permission: create features_test content
  $permissions['create features_test content'] = array(
    'name' => 'create features_test content',
    'roles' => array(
      '0' => 'anonymous user',
      '1' => 'authenticated user',
    ),
  );

  return $permissions;
}
