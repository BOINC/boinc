<?php

/**
 * Implementation of hook_role_export_defaults().
 */
function boinc_standard_role_export_defaults() {
  $roles = array();

  $roles['administrator'] = array(
    'rid' => '3519698132',
    'name' => 'administrator',
    'machine_name' => 'administrator',
  );

  $roles['community_member'] = array(
    'rid' => '1110965548',
    'name' => 'community member',
    'machine_name' => 'community_member',
  );

  $roles['content_editor'] = array(
    'rid' => '2938987599',
    'name' => 'content editor',
    'machine_name' => 'content_editor',
  );

  $roles['verified_contributor'] = array(
    'rid' => '933038561',
    'name' => 'verified contributor',
    'machine_name' => 'verified_contributor',
  );

  return $roles;
}
