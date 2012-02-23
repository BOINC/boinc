<?php
// $Id: flag_friend_is_friend.inc,v 1.1.2.1 2010/03/24 17:06:15 sirkitree Exp $

/**
 * @file
 * Ctools access plugin to provide access/visiblity if two user contexts are equal.
 */

/**
 * Implement specially named hook_ctools_access().
 */
function flag_friend_flag_friend_is_friend_ctools_access() {
  return array(
    'title' => t("User: Flag friend"),
    'description' => t('Compare the current user and logged in user flag friend status.'),
    'callback' => 'flag_friend_flag_friend_is_friend_access_check',
    'default' => array(
      'is_friend' => 1,
    ),
    'settings form' => 'flag_friend_flag_friend_is_friend_settings',
    'summary' => 'flag_friend_flag_friend_is_friend_ctools_access_summary',
    'required context' => array(
      new ctools_context_required(t('First User'), 'user'),
      new ctools_context_required(t("Second User"), 'user')
    ),
  );
}

/**
 * Settings form for the 'by perm' access plugin
 */

function flag_friend_flag_friend_is_friend_settings(&$form, &$form_state, $conf) {
  $form['settings']['helptext'] = array(
    '#type' => 'markup',
    '#value' => '<div>'. t('Grant access based on flag_friend status between the two user contexts.') .'<br \>'. t('Note that this will NOT grant access if both users are the same.') . '</div>',
  );
  $form['settings']['is_friend'] = array(
    '#type' => 'radios',
    '#title' => t('Grant access if both users are'),
    '#options' => array(1 => t('Friends'), 0 => t('Not friends')),
    '#default_value' => $conf['is_friend'],
  );
}


/**
 * Check for access.
 */
function flag_friend_flag_friend_is_friend_access_check($conf, $context) {
  if (empty($context) || count($context) != 2 || empty($context[0]->data) || empty($context[1]->data)) {
    return FALSE;
  }

  $uid1 = $context[0]->data->uid;
  $uid2 = $context[1]->data->uid;
  $equal = ($uid1 == $uid2) ? TRUE : FALSE;
  $return = FALSE;

  // No need to check friend status if it's the same user
  if ($equal == FALSE) {
    $flag = flag_get_flag('friend');
    $isfriend = (flag_friend_determine_friend_status($flag, $uid1, $uid2) == 1) ? TRUE : FALSE;
    $return = $conf['is_friend'] ? ($isfriend == TRUE) : ($isfriend == FALSE);
  }

  return $return;
}

/**
 * Describe an instance of this plugin.
 */
function flag_friend_flag_friend_is_friend_ctools_access_summary($conf, $context) {
  $comparison = !empty($conf['is_friend']) ? "is a friend of" : 'is not a friend of';

  return t('@id1 @comp @id2', array('@comp' => $comparison, '@id1' => $context[0]->identifier, '@id2' => $context[1]->identifier));
}
