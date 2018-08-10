<?php
// $Id$

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General utility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Helper functions
 */
function _boincuser_delete_goto_admindelete($form, &$form_state) {
  drupal_goto('/admin/boinc/user_delete/' . $form['#uid']);
}

function _boincuser_delete_cancel($form, &$form_state) {
  drupal_goto('/user/' . $form['#uid'] . '/edit');
}

/**
 * Validation user password function.
 */
function _boincuser_delete_validatepasswd($boinc_user, $current_pass) {
  if (!$current_pass) {
    return form_set_error('current_pass', bts('Authentication is required when requesting account deletion.', array(), NULL, 'boinc:delete-user-account'));
  }

  $given_hash = md5($current_pass . $boinc_user->email_addr);

  if ( (!password_verify($given_hash, $boinc_user->passwd_hash)) and ($given_hash != $boinc_user->passwd_hash) ) {
    return form_set_error('current_pass', bts('Password entered is not valid. Please verify that it is correct.', array(), NULL, 'boinc:delete-user-account'));
  }

  return true;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Custom delete functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Assigns nodes to anonymous user.
 *
 * Copied from node.module, node_user()
 */
function _boincuser_delete_node_reassign($account) {
  db_query('UPDATE {node} SET uid = 0 WHERE uid = %d', $account->uid);
  db_query('UPDATE {node_revisions} SET uid = 0 WHERE uid = %d', $account->uid);
}

/**
 * Assigns comments to anonymous user.
 *
 * Copied from comment.module, comment_user()
 */
function _boincuser_delete_comment_reassign($account) {
  // bug in comment module, remove user name from comments.
  db_query("UPDATE {comments} SET comments.name='' WHERE uid =%d", $account->uid);

  db_query('UPDATE {comments} SET uid = 0 WHERE uid = %d', $account->uid);
  db_query('UPDATE {node_comment_statistics} SET last_comment_uid = 0 WHERE last_comment_uid = %d', $account->uid);
}

/**
 * Deletes private messages
 *
 * Copied from privatemsg module, case 'delete' in privatemsg_user().
 */
function _boincuser_delete_privatemsg_delete($account) {
  // Load all mids of the messages the user wrote.
  $result = db_query("SELECT mid FROM {pm_message} WHERE author = %d", $account->uid);
  $mids = array();
  while ($row = db_fetch_array($result)) {
    $mids[] = $row['mid'];
  }

  // Delete messages the user wrote.
  db_query('DELETE FROM {pm_message} WHERE author = %d', $account->uid);

  if (!empty($mids)) {
    // Delete recipient entries in {pm_index} of the messages the user wrote.
    db_query('DELETE FROM {pm_index} WHERE mid IN (' . db_placeholders($mids) . ')', $mids);
  }

  // Delete recipient entries of that user.
  db_query('DELETE FROM {pm_index} WHERE uid = %d', $account->uid);
}

/**
 * Deletes the friend connections for this user
 *
 * Copied from flag_friend, flag_friend_user()
 */
function _boincuser_delete_friends($account) {
  // remove any friend relationships if an account is removed
    db_query("DELETE FROM {flag_friend} WHERE uid = %d OR friend_uid = %d", $account->uid, $account->uid);
}