<?php

/**
 * Implementation of hook_user_default_roles().
 */
function discussion_forums_user_default_roles() {
  $roles = array();

  // Exported role: moderator
  $roles['moderator'] = array(
    'name' => 'moderator',
  );

  return $roles;
}
