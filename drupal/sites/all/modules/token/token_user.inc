<?php

/**
 * @file
 * Implementations of token module hooks for the core user module.
 *
 * The token module requires specific hooks to be added to modules
 * so that those modules can return data about their objects to the
 * token API.  Until and unless token becomes a part of core, the
 * implementations of the token hooks for core modules are provided
 * in the token module itself.
 *
 * @ingroup token
 */

/**
 * Implementation of hook_token_list().
 */
function user_token_list($type = 'all') {
  $tokens = array();

  if ($type == 'user' || $type == 'all') {
    $tokens['user']['user']     = t("The login name of the user account.");
    $tokens['user']['user-raw'] = t("The login name of the user account.");
    $tokens['user']['uid']      = t("The unique ID of the user account.");
    $tokens['user']['mail']     = t("The email address of the user account.");

    $tokens['user'] += token_get_date_token_info(t("User's registration"), 'user-created-');
    $tokens['user'] += token_get_date_token_info(t("User's last login"), 'user-last-login-');
    $tokens['user']['date-in-tz'] = t("The current date in the user's timezone.");

    $tokens['user']['account-url']      = t("The URL of the account profile page.");
    $tokens['user']['account-edit-url'] = t("The URL of the account edit page.");
  }

  return $tokens;
}

/**
 * Implementation of hook_token_values().
 */
function user_token_values($type, $object = NULL, $options = array()) {
  $values = array();

  if ($type == 'user') {
    // @todo Why do we all the current user object to be loaded?
    $account = !empty($object) ? $object : user_load(array('uid' => $GLOBALS['user']->uid));

    // Adjust for the anonymous user name.
    if (!$account->uid && empty($account->name)) {
      $account_name = variable_get('anonymous', 'Anonymous');
    }
    else {
      $account_name = $account->name;
    }

    $values['user']     = check_plain($account_name);
    $values['user-raw'] = $account_name;
    $values['uid']      = $account->uid;
    $values['mail']     = $account->uid ? $account->mail : '';

    if ($account->uid) {
      $values += token_get_date_token_values($account->created, 'user-created-');
      $values += token_get_date_token_values($account->access, 'user-last-login-');
      $values['reg-date'] = $values['user-created-small'];
      $values['reg-since'] = $values['user-created-since'];
      $values['log-date'] = $values['user-last-login-small'];
      $values['log-since'] = $values['user-last-login-since'];
      $values['date-in-tz'] = $account->uid ? format_date(time(), 'small', '', $account->timezone) : '';
    }

    $values['account-url']      = $account->uid ? url("user/$account->uid", array('absolute' => TRUE)) : '';
    $values['account-edit-url'] = $account->uid ? url("user/$account->uid/edit", array('absolute' => TRUE)) : '';
    $values['account-edit']     = $values['account-edit-url'];
  }

  return $values;
}
