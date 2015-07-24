<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function teams_user_default_permissions() {
  $permissions = array();

  // Exported permission: create boincteam
  $permissions['create boincteam'] = array(
    'name' => 'create boincteam',
    'roles' => array(
      '0' => 'community member',
    ),
  );

  // Exported permission: join boincteam
  $permissions['join boincteam'] = array(
    'name' => 'join boincteam',
    'roles' => array(
      '0' => 'community member',
    ),
  );

  return $permissions;
}
