<?php
// $Id: hook_vud.php,v 1.1.2.2 2010/06/20 20:44:17 marvil07 Exp $
/**
 * @file
 * This file contains module hooks for users of Vote Up/down.
 */

/**
 * @name constant for a new widget message code.
 * The suggested name is build as: VUD_<entity>_WIDGET_MESSAGE_<description>.
 */
define('VUD_NEWENTITY_WIDGET_MESSAGE_POSTPONED', 2);

/**
 * Modify the array of know messages.
 *
 * For a real implementation take a look at
 * vud_node_vud_widget_message_codes_alter() and its implementation of
 * hook_nodeapi() on vud_node module.
 *
 * @param $widget_message_codes
 *   The array of know messages passed by reference to modify it.
 */
function hook_vud_widget_message_codes(&$widget_message_codes) {
  // Add a new message code with its description, take a look to the
  // constant definition for more information
  // This is a dummy message to notify voting is posponed. It make sense
  // on a new vud_<entity> module since we only can include new messages
  // while we are doing a real vote work.
  $widget_message_codes[VUD_NEWENTITY_WIDGET_MESSAGE_POSTPONED] = t('The voting on this is postponed, please wait a while. we will be open the voting soon');
}

/**
 * Modify the vote just before it is casted.
 *
 * @param $votes
 *   A votes array that is going to be passed to votingapi_set_votes()
 *   function.
 */
function hook_vud_votes(&$votes) {
  // let's add a new vote at the same time with an own vote tag
  $new_vote = $votes[0];
  $new_vote['tag'] = 'our_custom_tag';
  $votes[] = $new_vote;
}
