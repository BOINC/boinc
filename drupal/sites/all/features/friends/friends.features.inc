<?php

/**
 * Implementation of hook_flag_default_flags().
 */
function friends_flag_default_flags() {
  $flags = array();
  // Exported flag: "Friend".
  $flags['friend'] = array(
    'content_type' => 'user',
    'title' => 'Friend',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Add friend',
    'flag_long' => 'Add this user to your list of friends.',
    'flag_message' => '',
    'unflag_short' => 'Remove friend',
    'unflag_long' => 'Remove this user from your list of friends.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'show_on_profile' => 1,
    'access_uid' => '',
    'flag_confirmation' => 'Add [display-name] as a friend?',
    'unflag_confirmation' => 'Remove [display-name] as a friend?',
    'module' => 'friends',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  return $flags;

}

/**
 * Implementation of hook_views_api().
 */
function friends_views_api() {
  return array(
    'api' => '2',
  );
}
