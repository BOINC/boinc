<?php

/**
 * Implementation of hook_user_default_roles().
 */
function boinc_standard_user_default_roles() {
  $roles = array();

  // Exported role: administrator
  $roles['administrator'] = array(
    'name' => 'administrator',
  );

  // Exported role: anonymous user
  $roles['anonymous user'] = array(
    'name' => 'anonymous user',
  );

  // Exported role: authenticated user
  $roles['authenticated user'] = array(
    'name' => 'authenticated user',
  );

  // Exported role: community member
  $roles['community member'] = array(
    'name' => 'community member',
  );

  // Exported role: content editor
  $roles['content editor'] = array(
    'name' => 'content editor',
  );

  // Exported role: moderator
  $roles['moderator'] = array(
    'name' => 'moderator',
  );

  // Exported role: verified contributor
  $roles['verified contributor'] = array(
    'name' => 'verified contributor',
  );

  return $roles;
}
