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