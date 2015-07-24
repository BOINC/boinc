<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function spam_controls_user_default_permissions() {
  $permissions = array();

  // Exported permission: administer CAPTCHA settings
  $permissions['administer CAPTCHA settings'] = array(
    'name' => 'administer CAPTCHA settings',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer recaptcha
  $permissions['administer recaptcha'] = array(
    'name' => 'administer recaptcha',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer spamicide
  $permissions['administer spamicide'] = array(
    'name' => 'administer spamicide',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: skip CAPTCHA
  $permissions['skip CAPTCHA'] = array(
    'name' => 'skip CAPTCHA',
    'roles' => array(),
  );

  return $permissions;
}
