<?php
// $Id$

/**
 * @file
 * Provides additional rules support for nodereference fields.
 */

/**
 * Implementation of hook_rules_action_info().
 */
function nodereference_rules_action_info() {
  $info = array();
  $info['nodereference_rules_action_load'] = array(
    'label' => t('Load a referenced node'),
    'arguments' => array(
      'node' => array(
        'type' => 'node',
        'label' => t('Content containing the node reference field'),
      ),
    ),
    'new variables' => array(
      'referenced_node' => array(
        'type' => 'node',
        'label' => t('Referenced content'),
      ),
    ),
    'module' => 'CCK',
    'help' => t('Note that if the field has multiple values, only the first content node will be loaded.'),
  );
  return $info;
}

function nodereference_rules_action_load($node, $settings) {
  if ($nid = $node->{$settings['field']}[0]['nid']) {
    return array('referenced_node' => node_load(array('nid' => $nid)));
  }
}

function nodereference_rules_action_load_form($settings, &$form) {
  $settings += array('field' => '');
  $options = content_rules_get_field_names_by_type('nodereference');
  $form['settings']['field'] = array(
    '#type' => 'select',
    '#title' => t('Field'),
    '#default_value' => $settings['field'],
    '#options' => $options,
    '#required' => TRUE,
    '#disabled' => empty($options),
    '#description' => empty($options) ? t('There are no nodereference fields defined.') : '',
  );
}

/**
 * Helps upgrading from the workflow-ng action
 * "workflow_ng_action_load_referenced_node" to the equivalent rules action.
 */
function workflow_ng_action_load_referenced_node_upgrade(&$element) {
  $element['#name'] = 'nodereference_rules_action_load';
}
