<?php

/**
 * Implementation of hook_features_api().
 */
function user_features_api() {
  return array(
    'user_role' => array(
      'name' => t('Roles'),
      'feature_source' => TRUE,
      'default_hook' => 'user_default_roles',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
    'user_permission' => array(
      'name' => t('Permissions'),
      'feature_source' => TRUE,
      'default_hook' => 'user_default_permissions',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
    // DEPRECATED
    'user' => array(
      'name' => t('Permissions'),
      'feature_source' => FALSE,
    )
  );
}

/**
 * Implementation of hook_features_export().
 * DEPRECATED: This implementation simply migrates deprecated `user` items
 * to the `user_permission` type.
 */
function user_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $perm) {
    $pipe['user_permission'][] = $perm;
  }
  return $pipe;
}

/**
 * Implementation of hook_features_export().
 */
function user_permission_features_export($data, &$export, $module_name = '') {
  $export['dependencies']['features'] = 'features';

  // Ensure the modules that provide the given permissions are included as dependencies.
  $map = _user_features_permission_map();
  foreach ($data as $perm) {
    if (isset($map[$perm])) {
      $perm_module = $map[$perm];
      $export['dependencies'][$perm_module] = $perm_module;
      $export['features']['user_permission'][$perm] = $perm;
    }
  }

  return array();
}

/**
 * Implementation of hook_features_export_options().
 */
function user_permission_features_export_options() {
  $options = array();
  foreach (module_list() as $module) {
    $prefix = check_plain($module) .': ';
    if ($permissions = module_invoke($module, 'perm')) {
      foreach ($permissions as $permission) {
        $options[$permission] = $prefix . check_plain($permission);
      }
    }
  }
  asort($options);
  return $options;
}

/**
 * Implementation of hook_features_export_render().
 */
function user_permission_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $permissions = array();';
  $code[] = '';

  $permissions = _user_features_get_permissions();

  foreach ($data as $perm_name) {
    $permission = array();
    $permission['name'] = $perm_name;
    if (isset($permissions[$perm_name])) {
      sort($permissions[$perm_name]);
      $permission['roles'] = $permissions[$perm_name];
    }
    else {
      $permission['roles'] = array();
    }
    $perm_identifier = features_var_export($perm_name);
    $perm_export = features_var_export($permission, '  ');
    $code[] = "  // Exported permission: {$perm_name}";
    $code[] = "  \$permissions[{$perm_identifier}] = {$perm_export};";
    $code[] = "";
  }

  $code[] = '  return $permissions;';
  $code = implode("\n", $code);
  return array('user_default_permissions' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function user_permission_features_revert($module) {
  user_permission_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 * Iterate through default permissions and update the permissions map.
 *
 * @param $module
 *   The module whose default user permissions should be rebuilt.
 */
function user_permission_features_rebuild($module) {
  if ($defaults = features_get_default('user_permission', $module)) {
    $roles = _features_get_roles();
    foreach ($defaults as $permission) {
      $perm = $permission['name'];

      // Add permissions for any roles that should have them.
      foreach ($permission['roles'] as $role) {
        if (isset($roles[$role]) && !in_array($perm, $roles[$role]['perm'])) {
          $roles[$role]['perm'][] = $perm;
        }
      }

      // Remove permissions for any roles that shouldn't have them.
      foreach (array_keys($roles) as $role) {
        if (in_array($perm, $roles[$role]['perm']) && !in_array($role, $permission['roles'])) {
          $position = array_search($perm, $roles[$role]['perm']);
          unset($roles[$role]['perm'][$position]);
        }
      }
    }
    // Write the updated permissions.
    _user_features_save_roles($roles);
  }
}

/**
 * Implementation of hook_features_export().
 */
function user_role_features_export($data, &$export, $module_name = '') {
  $export['dependencies']['features'] = 'features';
  $map = features_get_default_map('user_role', 'name');
  foreach ($data as $role) {
    // Role is provided by another module. Add dependency.
    if (isset($map[$role]) && $map[$role] != $module_name) {
      $export['dependencies'][$map[$role]] = $map[$role];
    }
    // Export.
    else {
      $export['features']['user_role'][$role] = $role;
    }
  }
  return array();
}

/**
 * Implementation of hook_features_export_options().
 */
function user_role_features_export_options() {
  $options = drupal_map_assoc(array_keys(_features_get_roles()));
  asort($options);
  return $options;
}

/**
 * Implementation of hook_features_export_render().
 */
function user_role_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $roles = array();';
  $code[] = '';

  $roles = _features_get_roles();
  foreach ($data as $role) {
    if (isset($roles[$role])) {
      $object = array('name' => $role);
      $role_identifier = features_var_export($role);
      $role_export = features_var_export($object , '  ');
      $code[] = "  // Exported role: {$role}";
      $code[] = "  \$roles[{$role_identifier}] = {$role_export};";
      $code[] = "";
    }
  }

  $code[] = '  return $roles;';
  $code = implode("\n", $code);
  return array('user_default_roles' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function user_role_features_revert($module) {
  user_role_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 */
function user_role_features_rebuild($module) {
  if ($defaults = features_get_default('user_role', $module)) {
    $roles = _features_get_roles();
    foreach ($defaults as $role) {
      if (!isset($roles[$role['name']])) {
        drupal_write_record('role', $role);
      }
    }
  }
}

/**
 * Generate a perm to module mapping.
 */
function _user_features_permission_map($reset = FALSE) {
  static $map;
  if (!isset($map) || $reset) {
    $map = array();
    foreach (module_implements('perm') as $module) {
      if ($permissions = module_invoke($module, 'perm')) {
        foreach ($permissions as $permission) {
          $map[$permission] = $module;
        }
      }
    }
  }
  return $map;
}

/**
 * Represent the current state of permissions as a perm to role name array map.
 */
function _user_features_get_permissions() {
  $result = db_query("SELECT p.perm, r.name FROM {role} r INNER JOIN {permission} p ON p.rid = r.rid ORDER BY r.rid ASC, p.perm ASC");
  $permissions = array();
  while ($row = db_fetch_object($result)) {
    $role = $row->name;
    $role_perms = explode(', ', $row->perm);
    foreach ($role_perms as $perm) {
      if (!isset($permissions[$perm])) {
        $permissions[$perm] = array();
      }
      if(!in_array($role, $permissions[$perm])) {
        $permissions[$perm][] = $role;
      }
    }
  }
  return $permissions;
}

/**
 * Save/update an array of roles to the database.
 */
function _user_features_save_roles($roles) {
  // Write the updated permissions.
  foreach ($roles as $role) {
    db_query("DELETE FROM {permission} WHERE rid = %d", $role['rid']);
    $role['perm'] = implode(', ', $role['perm']);
    drupal_write_record('permission', $role);
  }
}
