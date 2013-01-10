<?php

/**
 * Implementation of hook_captcha_default_points().
 */
function spam_controls_captcha_default_points() {
  return array(
    'edit_profile_node_form' => array(
      'form_id' => 'edit_profile_node_form',
      'module' => NULL,
      'captcha_type' => 'default',
    ),
  );
}
