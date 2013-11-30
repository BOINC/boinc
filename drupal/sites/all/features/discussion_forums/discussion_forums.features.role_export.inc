<?php

/**
 * Implementation of hook_role_export_defaults().
 */
function discussion_forums_role_export_defaults() {
  $roles = array();

  $roles['moderator'] = array(
    'rid' => '1271379760',
    'name' => 'moderator',
    'machine_name' => 'moderator',
  );

  return $roles;
}
