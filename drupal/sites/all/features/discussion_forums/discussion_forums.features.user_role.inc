<?php

/**
 * Implementation of hook_user_default_roles().
 */
function discussion_forums_user_default_roles() {
  $roles = array();

  // Exported role: assistant moderator
  $roles['assistant moderator'] = array(
    'name' => 'assistant moderator',
  );

  return $roles;
}
