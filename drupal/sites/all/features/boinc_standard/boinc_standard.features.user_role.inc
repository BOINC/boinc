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

  // Exported role: content editor
  $roles['content editor'] = array(
    'name' => 'content editor',
  );

  return $roles;
}
