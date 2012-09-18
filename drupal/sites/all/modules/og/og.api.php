<?php

/**
 * @file
 * Hooks provided by the Organic Groups module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Act on group subscription actions.
 *
 * This hook allows modules to react when operations are performed on group
 * subscriptions.
 *
 * @param $op
 *   What kind of action is being performed. Possible values (in
 *   alphabetical order):
 *   - admin new: A new user is added as an admin to a group.
 *   - user approve: A user has been approved for membership to a group.
 *   - user broadcast: Send notifications to group members.
 *   - user delete: A user deletes their subscription to a group.
 *   - user deny: A user is denied membership to a group.
 *   - user insert: New user joins a group.
 *   - user request: A user requests membership to a group.
 *   - user update: A user changes their subscription to a group.
 *
 * @param  $gid
 *   The group Node ID.
 * @param $uid
 *   The User ID affected by the message. For 'user request', array of uids for
 *   group administrators.
 * @param $args
 *   A set of parameters that defines extended arguments. Varies by operation.
 *   user create, user update:
 *   - is_active: 1 to create an active subscription, 0 to create a subscription
 *     request.
 *   - is_admin: 1 to create a group administrator subscription, 0 to create a
 *     typical member subscription.
 *   admin new, user broadcast, user deny, user request:
 *   - subject: Subject/Title of a notification message.
 *   - body: Text of a notification message.
 *   user broadcast:
 *   - from: The user account sending the message.
 */
function hook_og($op, $gid, $uid, $args) {
  switch ($op) {
    case 'user insert':
      drupal_set_message('User !uid added to group !gid.', array('!uid' => $uid, '!gid' => $gid));
      break;
    case 'user delete':
      drupal_set_message('User !uid removed from group !gid.', array('!uid' => $uid, '!gid' => $gid));
      break;
  }
}

/**
 * Define links to be added to the Group Details block.
 *
 * @param $group
 *   Group node object for currently active group.
 *
 * @return
 *   Associative array of links.
 */
function hook_og_create_links($goup) {
  $links = array();

  // Add a link to contact the group owner.
  if (module_exists('contact')) {
    $links['contact_owner'] = l(t('Contact Owner'), 'user/' . $group->uid . '/contact');
  }

  return $links;
}

/**
 * Remove or modify links in the Group Details block.
 *
 * @param $links
 *   Array of links.
 * @param $group
 *   Group node object for currently active group.
 */
function hook_og_links_alter(&$links, $group) {
  // Match a customized path to the contact form.
  if (isset($links['contact_owner'])) {
    $links['contact_owner'] = l(t('Contact Owner'), 'contact/user/' . $group->uid);
  }
}

/**
 * @} End of "addtogroup hooks".
 */
