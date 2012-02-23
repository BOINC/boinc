<?php
// $Id: user_picture.inc,v 1.3.2.1 2010/01/29 20:18:26 merlinofchaos Exp $

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('User picture'),
  'icon' => 'icon_user.png',
  'description' => t('The picture of a user.'),
  'required context' => new ctools_context_required(t('User'), 'user'),
  'category' => t('User'),
);

function ctools_user_picture_content_type_render($subtype, $conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'term-list';

  if ($account === FALSE || ($account->access == 0 && !user_access('administer users'))) {
    return drupal_not_found();
  }

  $block->title = check_plain($account->name);
  $block->content = theme('user_picture', $account);

  return $block;
}

/**
 * Display the administrative title for a panel pane in the drag & drop UI
 */
function ctools_user_picture_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" user picture', array('@s' => $context->identifier));
}

function ctools_user_picture_content_type_edit_form(&$form, &$form_state) {
  // provide a blank form so we have a place to have context setting.
}
