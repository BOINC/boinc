<?php
// $Id: flag_friend_links.inc,v 1.1.2.1 2010/03/24 17:01:25 sirkitree Exp $

/**
 * @file
 * Panel content type that exposes flag friend links for adding friends,
 * removing them or show the cancel link after request friendship.
 */

/**
 * Callback function to supply a list of content types.
 */
function flag_friend_flag_friend_links_ctools_content_types() {
  return array(
    'single' => TRUE,
    'title' => t('Flag Friend Links'),
    'description' => t('Flag Friend Links for adding / removing friends'),
    'required context' => new ctools_context_required(t('User'), 'user'),
    'category' => t('Miscellaneous'),
  );
}

/**
 * Render the unread messages content type.
 */
function flag_friend_flag_friend_links_content_type_render($subtype, $conf, $panel_args, $context) {
  if (!empty($context) && empty($context->data)) {
    return;
  }
  // $context in this case is a user context, so we can get the user object
  // from it and put it into $account.

  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  // Make a new empty "block" which will be a Pane you can add to your Panel.
  $block = new stdClass();
  if ($account) {
    $block->title = t("Flag friend links");
    $block->content = flag_friend_links_get_link_values($account);
  }
  else {
    // If somehow the user context is empty, this is a fallback message but
    // that should never happen.
    $block->content = t("Flag friend links not available");
  }
  return $block;
}


/**
 * Empty form so we can have the default override title.
 */
function flag_friend_flag_friend_links_content_type_edit_form(&$form, &$form_state) {
  // Does nothing!
}

function flag_friend_flag_friend_links_content_type_edit_form_submit(&$form, &$form_state) {
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

function flag_friend_flag_friend_links_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" flag friend links', array('@s' => $context->identifier));
}

/**
 * Get the links for adding or removing friends. Also cancel request link is
 * shown.
 */
function flag_friend_links_get_link_values($account = NULL) {
  global $user;
  // If is the current user, no link is shown.
  if ($user->uid <> $account->uid) {
    // Check if users are currently friends
    $flag = flag_get_flag('friend');
    $flag_status = flag_friend_determine_friend_status($flag, $user->uid, $account->uid);

    switch ($flag_status) {
      case(FLAG_FRIEND_FLAGGED):
        // Remove friend
        return l(t('Remove friend'), "flag/confirm/unfriend/friend/$account->uid", array('query' => drupal_get_destination()));
        break;

      case(FLAG_FRIEND_APPROVAL):
      case(FLAG_FRIEND_UNFLAGGED):
        // Add friend
        return l(t('Add friend'), "flag/confirm/flag/friend/$account->uid", array('query' => drupal_get_destination()));
        break;

      case(FLAG_FRIEND_PENDING):
        // Pending friend
        return l(t('Friend Requested. Cancel?'), "flag/confirm/unflag/friend/$account->uid", array('query' => drupal_get_destination()));
        break;
    }
  }
  return;
}
