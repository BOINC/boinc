<?php

/**
 * @file
 *
 * Plugin to provide an argument handler for a user id which supports the me alias.
 */

/**
 * Implementation of CTools hook_ctools_arguments().
 *
 * See http://drupal.org/project/ctools for module and hook information.
 */
function me_me_uid_ctools_arguments() {
  $args['me_uid'] = array(
    'title' => t("User ID (Me Supported)"),
    // keyword to use for %substitution
    'keyword' => 'user',
    'description' => t('Creates a user context from a user ID argument or the %me alias.', array('%me' => _me_get_me_alias(TRUE))),
    'context' => 'me_argument_uid_context',
    'placeholder form' => array(
      '#type' => 'textfield',
      '#description' => t('Enter the user ID of a user for this argument or the %me alias.', array('%me' => _me_get_me_alias(TRUE))),
    ),
  );
  return $args;
}

/**
 * Discover if this argument gives us the user we crave.
 */
function me_argument_uid_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return ctools_context_create_empty('user');
  }

  // We can accept either a user object or a pure uid.
  if (is_object($arg)) {
    return ctools_context_create('user', $arg);
  }

  // Convert the argument if needed.
  $arg = _me_check_arg($arg);

  if (!is_numeric($arg)) {
    return NULL;
  }

  $account = user_load(array('uid' => $arg));
  if (!$account) {
    return NULL;
  }

  return ctools_context_create('user', $account);
}

