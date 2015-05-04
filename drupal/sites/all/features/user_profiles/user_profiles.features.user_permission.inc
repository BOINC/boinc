<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function user_profiles_user_default_permissions() {
  $permissions = array();

  // Exported permission: access user profiles
  $permissions['access user profiles'] = array(
    'name' => 'access user profiles',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer gravatar
  $permissions['administer gravatar'] = array(
    'name' => 'administer gravatar',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create profile content
  $permissions['create profile content'] = array(
    'name' => 'create profile content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'verified contributor',
    ),
  );

  // Exported permission: delete any profile content
  $permissions['delete any profile content'] = array(
    'name' => 'delete any profile content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: delete own profile content
  $permissions['delete own profile content'] = array(
    'name' => 'delete own profile content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
    ),
  );

  // Exported permission: disable own gravatar
  $permissions['disable own gravatar'] = array(
    'name' => 'disable own gravatar',
    'roles' => array(
      '0' => 'community member',
    ),
  );

  // Exported permission: edit any profile content
  $permissions['edit any profile content'] = array(
    'name' => 'edit any profile content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit own profile content
  $permissions['edit own profile content'] = array(
    'name' => 'edit own profile content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'verified contributor',
    ),
  );

  // Exported permission: use gravatar
  $permissions['use gravatar'] = array(
    'name' => 'use gravatar',
    'roles' => array(
      '0' => 'community member',
    ),
  );

  return $permissions;
}
