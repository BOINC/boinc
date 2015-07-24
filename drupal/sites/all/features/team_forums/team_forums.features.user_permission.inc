<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function team_forums_user_default_permissions() {
  $permissions = array();

  // Exported permission: create team_forum content
  $permissions['create team_forum content'] = array(
    'name' => 'create team_forum content',
    'roles' => array(
      '0' => 'verified contributor',
    ),
  );

  // Exported permission: delete any team_forum content
  $permissions['delete any team_forum content'] = array(
    'name' => 'delete any team_forum content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete own team_forum content
  $permissions['delete own team_forum content'] = array(
    'name' => 'delete own team_forum content',
    'roles' => array(
      '0' => 'community member',
    ),
  );

  // Exported permission: edit any team_forum content
  $permissions['edit any team_forum content'] = array(
    'name' => 'edit any team_forum content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit own team_forum content
  $permissions['edit own team_forum content'] = array(
    'name' => 'edit own team_forum content',
    'roles' => array(
      '0' => 'verified contributor',
    ),
  );

  // Exported permission: manage boincteam forum
  $permissions['manage boincteam forum'] = array(
    'name' => 'manage boincteam forum',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  return $permissions;
}
