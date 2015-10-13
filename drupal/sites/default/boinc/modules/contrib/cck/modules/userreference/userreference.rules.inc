<?php
// $Id$

/**
 * @file
 * Provides additional rules support for userreference fields.
 */

/**
 * Implementation of hook_rules_action_info()
 */
function userreference_rules_action_info() {
  $info = array();
  $info['userreference_rules_action_load'] = array(
    'label' => t('Load a referenced user'),
    'arguments' => array(
      'node' => array(
        'type' => 'node',
        'label' => t('Content containing the user reference field'),
      ),
    ),
    'new variables' => array(
      'referenced_user' => array(
        'type' => 'user',
        'label' => t('Referenced user'),
      ),
    ),
    'module' => 'CCK',
    'help' => t('Note that if the field has multiple values, only the first user will be loaded.'),
  );
  return $info;
}

function userreference_rules_action_load($node, $settings) {
  $uid = $node->{$settings['field']}[0]['uid'];
  if (isset($uid)) {
    $user = user_load(array('uid' => $uid));
    return array('referenced_user' => $user);
  }
}

function userreference_rules_action_load_form($settings, &$form) {
  $settings += array('field' => '');
  $options = content_rules_get_field_names_by_type('userreference');
  $form['settings']['field'] = array(
    '#type' => 'select',
    '#title' => t('Field'),
    '#default_value' => $settings['field'],
    '#options' => $options,
    '#required' => TRUE,
    '#disabled' => empty($options),
    '#description' => empty($options) ? t('There are no userreference fields defined.') : '',
  );
}

/**
 * Helps upgrading from the workflow-ng action.
 * "workflow_ng_action_load_referenced_user" to the equivalent rules action.
 */
function workflow_ng_action_load_referenced_user_upgrade(&$element) {
  $element['#name'] = 'userreference_rules_action_load';
}
