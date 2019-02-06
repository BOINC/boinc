<?php

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('User profile'),
  'icon' => 'icon_user.png',
  'description' => t('The profile of a user.'),
  'required context' => new ctools_context_required(t('User'), 'user'),
  'category' => t('User'),
);

/**
 * Render the user profile content type.
 */
function ctools_user_profile_content_type_render($subtype, $conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  if (!$account || ($account->access == 0 && !user_access('administer users'))) {
    return NULL;
  }

  // Retrieve all profile fields and attach to $account->content.
  user_build_content($account);

  $block = new stdClass();
  $block->module = 'user-profile';
  $block->title = check_plain($account->name);
  $block->content = theme('user_profile', $account);

  return $block;
}

/**
 * Display the administrative title for a panel pane in the drag & drop UI.
 */
function ctools_user_profile_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" user profile', array('@s' => $context->identifier));
}

function ctools_user_profile_content_type_edit_form(&$form, &$form_state) {
  // provide a blank form so we have a place to have context setting.
}
