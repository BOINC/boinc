<?php
// $Id: og.activity.inc,v 1.1.2.1 2010/02/21 17:32:35 amitaibu Exp $

/**
 * @file:
 * Provides Activity module hooks for Organic groups.
 */

/**
 * Implementation of hook_activity_grants().
 */
function og_activity_grants($activity) {
  $groups = array();
  $grants = array();

  if ($activity->type == 'og') {
    $groups = array($activity->nid);
  }
  elseif (!empty($activity->nid)) {
    // Find all the groups the node belongs to.
    $node = node_load($activity->nid);
    $groups = array_keys(og_get_node_groups($node));
  }

  if (!empty($groups)) {
    $grants = array(
      'og_member' => $groups,
    );
  }

  return $grants;
}

/**
 * Implementation of hook_activity_access_grants().
 */
function og_actvity_access_grants($account) {
  $grants = array();

  // Account is granted access to all activities within the groups account
  // is a member of.
  $groups = og_get_subscriptions($account->uid);
  if (!empty($groups)) {
    $grants['og_member'] = array_keys($groups);
  }
  return $grants;
}

/**
 * Implementation of hook_activity_messages_alter().
 */
function og_activity_messages_alter(&$messages, $type, $objects) {
  // If its been invoked from hook_og.
  if ($type == 'og') {
    $sql = og_list_users_sql(1, 1, 'u.uid');
    $admin_members = db_query($sql, $objects['node']->nid);

    // For every group admin, add the message per language.
    while ($member = db_fetch_object($admin_members)) {
      foreach (array_keys($messages) as $language_id) {
        if (!empty($messages[$language_id][$objects['node']->uid])) {
          $messages[$language_id][$member->uid] = $messages[$language_id][$objects['node']->uid];
        }
      }
    }
  }
}