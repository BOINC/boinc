<?php
// $Id: flag_friend_count.inc,v 1.1.2.1 2010/03/24 17:01:25 sirkitree Exp $

/**
 * @file
 * Panels content type that shows the number of friends of a given user.
 */

/**
 * Callback function to supply a list of content types.
 */
function flag_friend_flag_friend_count_ctools_content_types() {
  return array(
    'single' => TRUE,
    'title' => t('Flag Friend Count'),
    'description' => t('Count of friends of a given user'),
    'required context' => new ctools_context_required(t('User'), 'user'),
    'category' => t('Miscellaneous'),
  );
}

/**
 * Render the unread messages content type.
 */
function flag_friend_flag_friend_count_content_type_render($subtype, $conf, $panel_args, $context) {
  if (!empty($context) && empty($context->data)) {
    return;
  }
  // $context in this case is a user context, so we can get the user object
  // from it and put it into $account.

  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  // Make a new empty "block" which will be a Pane you can add to your Panel.
  $block = new stdClass();
  if ($account) {
    $block->title = t("Number of friends");
    $block->content = flag_friend_count_get_friend_number($account);
  }
  else {
    // If somehow the user context is empty, this is a fallback message but
    // that should never happen.
    $block->content = t("Flag friend count not available");
  }
  return $block;
}


/**
 * Empty form so we can have the default override title.
 */
function flag_friend_flag_friend_count_content_type_edit_form(&$form, &$form_state) {
  // Does nothing!
}

function flag_friend_flag_friend_count_content_type_edit_form_submit(&$form, &$form_state) {
  // For each part of the form defined in the 'defaults' array set when you
  // defined the content type, copy the value from the form into the array
  // of items to be saved. We don't ever want to use
  // $form_state['conf'] = $form_state['values'] because values contains
  // buttons, form id and other items we don't want stored. CTools will handle
  // the actual form submission.
  if (is_array($form_state['plugin']['defaults'])) {
    foreach (array_keys($form_state['plugin']['defaults']) as $key) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}

function flag_friend_flag_friend_count_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" flag friend number of friends', array('@s' => $context->identifier));
}

/**
 * Get the number of friends for the panel.
 */
function flag_friend_count_get_friend_number($account = NULL) {
  $friend_count = flag_friend_get_friend_count($account->uid);
  return t('!friend_count friends', array('!friend_count' => $friend_count));
}
