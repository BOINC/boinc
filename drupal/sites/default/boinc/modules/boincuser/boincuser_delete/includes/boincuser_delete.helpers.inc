<?php
// $Id$

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General utility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * _boincuser_delete_disableuser
 *
 * This is the function that actually takes a uid and blocks the
 * user. It also deletes any associated user profile.
 */
function _boincuser_delete_disableuser($uid) {
  $update = array();
  // Set status to 1: block/disable the account
  $account = user_load($uid);
  $update['status'] = 0;
  user_save($account, $update);

  // delete the user profile: code is a copy from content_profile.module
  // Retrieve all profile nodes (in any language) for this user by issueing an SQL query.
  if ($types = content_profile_get_types()) {
    $condition = array_fill(0, count($types), "type = '%s'");
    $arguments = array_merge(array_keys($types), array($account->uid));

    $result = db_query("SELECT * FROM {node} WHERE (". implode(' OR ', $condition) .") AND uid = %d", $arguments);
    while ($node = db_fetch_object($result)) {
      _content_profile_node_delete($node);
    }
  }

  // Reset the delete status and grace period for the user.
  $qrc = db_query("UPDATE {boincuser} SET delete_status=0, grace_period=0 WHERE uid=%d", $uid);

  watchdog(
    'boincuser',
    'Blocked user account @uid, @email',
    array('@uid' => $uid, '@email' => $account->mail),
    WATCHDOG_NOTICE
  );
}

/**
 * _boincuser_delete_deleteuser
 *
 * This is the function that actually takes a uid and deletes the
 * user.
 */
function _boincuser_delete_deleteuser($uid) {
  // Delete the user account.
  user_delete(array(), $uid);
}

/**
 * _boincuser_delete_notify
 *
 * This function notifies the user that the account is to be disabled
 * or deleted soon.
 */
function _boincuser_delete_notify($userinfo) {
  $uid = $userinfo['uid'];
  $delete_status = $userinfo['ds'];

  global $base_url;
  global $base_path;
  $account = user_load($uid);
  $mysubject = '';
  $mymessage = '';
  $site_name = variable_get('site_name', 'Drupal-BOINC');
  $site_url = $base_url . $base_path . "user/login?destination=home";

  switch ($delete_status) {
    case 1: //disable
      $mysubject = "Notification of ${site_name} account to be disabled in 24 hours.";
      $mymessage = ''
        . "{$account->boincuser_name},\n"
        . "\n"
        . "We have received a request to disable your user account at "
        . "${site_name}. If you do nothing, this account will be disabled in 24 hours. "
        . "If you wish to cancel your request, please login to your account "
        . "on the ${site_name} Web site (${site_url})."
        . ""
        . "If you did not initiate this request, please login to the "
        . "${site_name} Web site (${site_url}) to cancel and then contact "
        . "the administrators.\n"
        . "\n"
        . "Thanks, \n"
        . "\n"
        . "{$site_name} support team";
    break;
    case 2: //delete
      $mysubject = "Notification of ${site_name} account to be DELETED in 24 hours.";
      $mymessage = ''
        . "{$account->boincuser_name},\n"
        . "\n"
        . "We have received a request to DELETE your user account at "
        . "${site_name}. If you do nothing, this account will be DELETED in 24 hours. "
        . "If you wish to cancel your request, please login to your account "
        . "on the ${site_name} Web site (${site_url})."
        . ""
        . "If you did not initiate this request, please login to the "
        . "${site_name} Web site (${site_url}) to cancel and then contact "
        . "the administrators.\n"
        . "\n"
        . "Thanks, \n"
        . "\n"
        . "{$site_name} support team";
    break;
  }

  module_load_include('inc', 'rules', 'modules/system.rules');
  $account = user_load(array('uid' => $uid));
  $settings = array(
    'from' => '',
    'subject' => $mysubject,
    'message' => $mymessage,
  );
  rules_action_mail_to_user($account, $settings);

  // Update database to flag that we've notified the user of his/her
  // impending disable or delete action.
  // Reset the delete status and grace period for the user.
  $qrc = db_query("UPDATE {boincuser} SET delete_notified=1 WHERE uid=%d", $uid);
}
