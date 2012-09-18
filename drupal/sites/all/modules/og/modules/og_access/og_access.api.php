<?php

/**
 * @file
 * Hooks provided by the Organic Groups Access module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Modify the policies put in place for group-based access control.
 *
 * This hook allows easier modification and integration with OG Access
 * by allowing manipulation of the node access grants defined by
 * og_access_node_access_records(). hook_node_access_records() and the node
 * grants system generally are advanced development topics and should not be
 * attempted without background reading.
 *
 * It is easily possible to destroy expected behaviors of OG access, or to
 * extend OG Access to affect nodes not otherwise associated with OG.
 *
 * @param $grants
 *   Array of grant arrays.
 *
 * @param $node
 *   A node object. Remember to test the node type to see if it is a group node,
 *   group post, or neither.
 *
 * @see hook_node_access_records()
 * @see og_access_node_access_records()
 * @see http://drupal.org/node/1260948
 */
function hook_og_access_grants(&$grants, $node) {
  // Define a new grant allowing any group member to delete wiki content.
  $is_wiki = og_is_wiki_type($node->type);
  foreach ($node->og_groups as $gid) {
    $grants[] = array(
      'realm' => 'og_subscriber',
      'gid' => $gid,
      'grant_view' => 1,
      'grant_update' => $is_wiki,
      'grant_delete' => $is_wiki,
      'priority' => 0,
    );
  }
}

/**
 * @} End of "addtogroup hooks".
 */
