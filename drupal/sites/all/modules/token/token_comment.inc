<?php

/**
 * @file
 * Implementations of token module hooks for the core comment module.
 *
 * The token module requires specific hooks to be added to modules
 * so that those modules can return data about their objects to the
 * token API.  Until and unless token becomes a part of core, the
 * implementations of the token hooks for core modules are provided
 * in the token module itself.
 * @ingroup token
 */

/**
 * Implements hook_token_list() on behalf of comment.module.
 */
function comment_token_list($type = 'all') {
  $tokens = array();

  if ($type == 'comment' || $type == 'all') {
    $tokens['comment']['comment-cid']             = t('The unique ID of the comment.');
    $tokens['comment']['comment-nid']             = t('The unique ID of the node the comment was posted to.');
    $tokens['comment']['comment-title']           = t('The title of the comment.');
    $tokens['comment']['comment-title-raw']       = t('The title of the comment.');
    $tokens['comment']['comment-body']            = t('The formatted content of the comment itself.');
    $tokens['comment']['comment-body-raw']        = t('The formatted content of the comment itself.');

    $tokens['comment']['comment-author-uid']      = t('The unique ID of the author of the comment.');
    $tokens['comment']['comment-author-name']     = t('The name left by the comment author.');
    $tokens['comment']['comment-author-name-raw'] = t('The name left by the comment author.');
    $tokens['comment']['comment-author-homepage'] = t('The home page URL left by the comment author.');

    $tokens['comment']['comment-author-mail']     = t('The email address left by the comment author.');
    $tokens['comment']['comment-author-mail-raw'] = t('The email address left by the comment author.');

    $tokens['comment'] += token_get_date_token_info(t('Comment creation'), 'comment-');

    $tokens['comment']['comment-node-title']      = t('The title of the node the comment was posted to.');
    $tokens['comment']['comment-node-title-raw']  = t('The title of the node the comment was posted to.');
  }

  return $tokens;
}

/**
 * Implements hook_token_values() on behalf of comment.module.
 */
function comment_token_values($type, $object = NULL, $options = array()) {
  $values = array();

  if ($type == 'comment' && !empty($object)) {
    // Cast to an object just in case fussy Drupal gave us an array
    $comment = (object) $object;

    $values['comment-cid']             = $comment->cid;
    $values['comment-nid']             = $comment->nid;
    $values['comment-title']           = check_plain($comment->subject);
    $values['comment-body']            = check_markup($comment->comment, $comment->format, FALSE);
    $values['comment-author-name']     = check_plain($comment->name);
    $values['comment-author-uid']      = $comment->uid;
    $values['comment-author-homepage'] = check_url($comment->homepage);

    // Raw counterparts of user supplied data.
    $values['comment-title-raw']       = $comment->subject;
    $values['comment-body-raw']        = $comment->comment;
    $values['comment-author-name-raw'] = $comment->name;

    if (!empty($comment->mail)) {
      $account_mail = $comment->mail;
    }
    elseif (!empty($comment->uid)) {
      $account_mail = db_result(db_query("SELECT mail FROM {users} WHERE uid = %d", $comment->uid));
    }
    else {
      $account_mail = '';
    }
    $values['comment-author-mail']     = check_plain($account_mail);
    $values['comment-author-mail-raw'] = $account_mail;

    // Included in case a consuming module wants to format the body
    $values['comment-body-format']     = $comment->format;

    $values += token_get_date_token_values($comment->timestamp, 'comment-');

    $values['comment-node-title-raw']  = db_result(db_query("SELECT title FROM {node} WHERE nid = %d", $comment->nid));
    $values['comment-node-title']      = check_plain($values['comment-node-title-raw']);
  }

  return $values;
}
