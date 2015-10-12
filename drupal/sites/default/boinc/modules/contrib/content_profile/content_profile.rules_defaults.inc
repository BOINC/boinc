<?php
// $Id: content_profile.rules_defaults.inc,v 1.1.2.2 2009/01/26 15:33:00 fago Exp $
/**
 * @file Rules default rules
 */

/**
 * Implementation of hook_rules_defaults().
 * Add a disabled default rule which redirects to the profile creation page, if users have
 * no profile.
 */
function content_profile_rules_defaults() {
  $type = array_shift(array_keys(content_profile_get_types('names')));
  $config =
array (
  'rules' =>
  array (
    'content_profile_rule_1' =>
    array (
      '#type' => 'rule',
      '#set' => 'event_user_login',
      '#label' => 'Redirect to profile creation page, if users have no profile.',
      '#active' => 0,
      '#weight' => '0',
      '#conditions' =>
      array (
        0 =>
        array (
          '#negate' => 1,
          '#weight' => 0,
          '#info' =>
          array (
            'label' => 'Logged in user has his Profile created',
            'arguments' =>
            array (
              'user' =>
              array (
                'type' => 'user',
                'label' => 'User',
              ),
            ),
            'module' => 'Content Profile',
          ),
          '#name' => 'content_profile_user_has_profile_condition',
          '#settings' =>
          array (
            'type' => $type,
            '#argument map' =>
            array (
              'account' => 'user',
            ),
          ),
          '#type' => 'condition',
        ),
      ),
      '#actions' =>
      array (
        0 =>
        array (
          '#type' => 'action',
          '#settings' =>
          array (
            'path' => 'node/add/'. str_replace('_', '-', $type),
            'query' => '',
            'fragment' => '',
            'force' => 1,
            'override' => 1,
          ),
          '#name' => 'rules_action_drupal_goto',
          '#info' =>
          array (
            'label' => 'Redirect to profile creation page',
            'label callback' => false,
            'module' => 'System',
            'eval input' =>
            array (
              0 => 'path',
              1 => 'query',
              2 => 'fragment',
            ),
          ),
          '#weight' => 0,
        ),
        1 =>
        array (
          '#weight' => 0,
          '#info' =>
          array (
            'label' => 'Show a message',
            'label callback' => false,
            'module' => 'System',
            'eval input' =>
            array (
              0 => 'message',
            ),
          ),
          '#name' => 'rules_action_drupal_message',
          '#settings' =>
          array (
            'message' => 'You haven\'t filled out your profile information yet. Please do so now!',
            'error' => 1,
          ),
          '#type' => 'action',
        ),
      ),
    ),
  ),
);

  return $config;
}
