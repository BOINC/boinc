<?php
// $Id: content_profile.rules.inc,v 1.1.2.2 2009/03/30 10:20:01 fago Exp $

/**
 * @file
 * Some rules conditions/actions
 */

/**
 * Implementation of hook_rules_condition_info().
 */
function content_profile_rules_condition_info() {
  return array(
    'content_profile_user_has_profile_condition' => array(
      'label' => t('User has content profile'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
      ),
      'module' => 'Content Profile',
    ),
  );
}

function content_profile_user_has_profile_condition($user, $settings) {
  $node = content_profile_load($settings['type'], $user->uid);
  return (bool)$node;
}

function content_profile_user_has_profile_condition_form($settings, &$form) {
  $settings += array('type' => array());
  $form['settings']['type'] = array(
    '#type' => 'select',
    '#title' => t('Content Profile Content Type'),
    '#options' => content_profile_get_types('names'),
    '#default_value' => $settings['type'],
    '#description' => t('Select the Content Profile content type to check for.'),
    '#required' => TRUE,
  );
}

function content_profile_user_has_profile_condition_label($settings, $argument_labels) {
  return t('@user has his @type created', $argument_labels + array('@type' => node_get_types('name', $settings['type'])));
}

/**
 * Implementation of hook_rules_action_info().
 */
function content_profile_rules_action_info() {
  return array(
    'content_profile_action_load' => array(
      'label' => t('Load Content Profile'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User, whose profile should be loaded')),
      ),
      'new variables' => array(
        'profile_node' => array('type' => 'node', 'label' => t('Content Profile')),
      ),
      'module' => 'Content Profile',
    ),
  );
}

/**
 * Loads a Content Profile
 */
function content_profile_action_load($user, $settings) {
  if ($node = content_profile_load($settings['type'], $user->uid)) {
    return array('profile_node' => $node);
  }
}

function content_profile_action_load_form($settings, &$form) {
  $settings += array('type' => array());
  $form['settings']['type'] = array(
    '#type' => 'select',
    '#title' => t('Content Profile Content Type'),
    '#options' => content_profile_get_types('names'),
    '#default_value' => $settings['type'],
    '#description' => t('Select the Content Profile content type to load.'),
    '#required' => TRUE,
  );
}

function content_profile_action_load_label($settings, $argument_labels) {
  return t("Load @user's @type", $argument_labels + array('@type' => node_get_types('name', $settings['type'])));
}


/**
 * Support upgrading from nodeprofile-workflow-ng integration.
 */
function nodeprofile_user_has_profile_condition_upgrade(&$element) {
  $element['#name'] = 'content_profile_user_has_profile_condition';
}
function nodeprofile_action_load_upgrade(&$element) {
  $element['#name'] = 'content_profile_action_load';
}
