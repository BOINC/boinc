<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function discussion_forums_user_default_permissions() {
  $permissions = array();

  // Exported permission: access comments
  $permissions['access comments'] = array(
    'name' => 'access comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
    ),
  );

  // Exported permission: administer comments
  $permissions['administer comments'] = array(
    'name' => 'administer comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: administer forums
  $permissions['administer forums'] = array(
    'name' => 'administer forums',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: assign community member role
  $permissions['assign community member role'] = array(
    'name' => 'assign community member role',
    'roles' => array(
      '0' => 'moderator',
    ),
  );

  // Exported permission: create forum topics
  $permissions['create forum topics'] = array(
    'name' => 'create forum topics',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
      '2' => 'moderator',
      '3' => 'verified contributor',
    ),
  );

  // Exported permission: delete any forum topic
  $permissions['delete any forum topic'] = array(
    'name' => 'delete any forum topic',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete own forum topics
  $permissions['delete own forum topics'] = array(
    'name' => 'delete own forum topics',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit any forum topic
  $permissions['edit any forum topic'] = array(
    'name' => 'edit any forum topic',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: edit own forum topics
  $permissions['edit own forum topics'] = array(
    'name' => 'edit own forum topics',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
      '2' => 'verified contributor',
    ),
  );

  // Exported permission: post comments
  $permissions['post comments'] = array(
    'name' => 'post comments',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
      '2' => 'moderator',
      '3' => 'verified contributor',
    ),
  );

  // Exported permission: post comments without approval
  $permissions['post comments without approval'] = array(
    'name' => 'post comments without approval',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'community member',
      '2' => 'moderator',
      '3' => 'verified contributor',
    ),
  );

  return $permissions;
}
