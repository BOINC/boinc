<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function internationalization_user_default_permissions() {
  $permissions = array();

  // Exported permission: administer all languages
  $permissions['administer all languages'] = array(
    'name' => 'administer all languages',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer languages
  $permissions['administer languages'] = array(
    'name' => 'administer languages',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer translations
  $permissions['administer translations'] = array(
    'name' => 'administer translations',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: translate content
  $permissions['translate content'] = array(
    'name' => 'translate content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: translate interface
  $permissions['translate interface'] = array(
    'name' => 'translate interface',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  return $permissions;
}
