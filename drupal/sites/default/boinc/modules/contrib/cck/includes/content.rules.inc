<?php
// $Id$

/**
 * @file
 * Provides basic rules module support.
 */

/**
 * Implementation of hook_rules_action_info().
 */
function content_rules_action_info() {
  $info = array();
  $info['content_rules_action_populate_field'] = array(
    'label' => t('Populate a field'),
    'arguments' => array(
      'node' => array(
        'type' => 'node',
        'label' => t('Content'),
      ),
    ),
    'eval input' => array('code'),
    'help' => t('You should make sure that the used field exists in the given content type.'),
    'module' => 'CCK',
  );
  return $info;
}

/**
 * Action: populate a field.
 */
function content_rules_action_populate_field($node, $settings, $element, &$state) {
  // Get information about the field.
  $field = content_fields($settings['field_name'], $node->type);
  $value = _content_rules_get_field_value($settings, $state);

  if (!empty($field) && is_array($value)) {
    $node->{$settings['field_name']} = $value;
    return array('node' => $node);
  }
}


/**
 * Action "populate a field" configuration form.
 * This is a multistep form!
 */
function content_rules_action_populate_field_form($settings, &$form, &$form_state) {
  $settings += array('field_name' => '', 'code' => '', 'value' => NULL);
  if (empty($settings['field_name'])) {
    $form['settings']['field_name'] = array(
      '#type' => 'select',
      '#title' => t('Field'),
      '#options' => content_rules_get_field_names_by_type(),
      '#default_value' => $settings['field_name'],
      '#description' => t('Select the machine-name of the field.'),
      '#required' => TRUE,
    );
    // Hide some form elements in the first step.
    $form['negate']['#access'] = FALSE;
    $form['input_help']['#access'] = FALSE;
    $form['weight']['#access'] = FALSE;

    // Replace the usual submit handlers with a own handler.
    $form['submit']['#submit'] = array('content_rules_action_populate_field_form_step_submit');
    $form['submit']['#value'] = t('Continue');
  }
  else {
    // Show the fields form here.
    module_load_include('inc', 'content', 'includes/content.node_form');
    $field = content_fields($settings['field_name']);

    $form['#node'] = (object)array('type' => '', $settings['field_name'] => $settings['value']);
    $form['#field_info'][$field['field_name']] = $field;
    // We can't put it into $form['settings'] as this would break AHAH callbacks
    $form += (array) content_field_form($form, $form_state, $field);
    $form[ $settings['field_name'] ]['#weight'] = 4;

    unset($form['#cache']);

      // Advanced: PHP code.
    $form['advanced_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced: Specify the fields value with PHP code'),
      '#collapsible' => TRUE,
      '#collapsed' => empty($settings['code']),
      '#weight' => 5,
    );

    $db_info = content_database_info($field);
    $columns = array_keys($db_info['columns']);
    foreach ($columns as $key => $column) {
      $columns[$key] = t("'@column' => value for @column", array('@column' => $column));
    }
    $sample = t("return array(\n  0 => array(@columns),\n  // You'll usually want to stop here. Provide more values\n  // if you want your 'default value' to be multi-valued:\n  1 => array(@columns),\n  2 => ...\n);", array('@columns' => implode(', ', $columns)));

    $form['advanced_options']['code'] = array(
      '#type' => 'textarea',
      '#title' => t('Code'),
      '#default_value' => $settings['code'],
      '#rows' => 6,
      '#description' => t('Advanced usage only: PHP code that returns the value to set. Should not include &lt;?php ?&gt; delimiters. If this field is filled out, the value returned by this code will override any value specified above. Expected format: <pre>!sample</pre>Using <a href="@link_devel">devel.module\'s</a> \'devel load\' tab on a content page might help you figure out the expected format.', array(
        '!sample' => $sample,
        '@link_devel' => 'http://www.drupal.org/project/devel',
      )),
    );

    // Add this file to be included when the form is built by rules
    // as it's needed by CCKs add more button.
    // See rules_after_build_include_files().
    $form['#includes'][] = './'. drupal_get_path('module', 'node') .'/node.pages.inc';
  }
}

function content_rules_action_populate_field_form_step_submit($form, &$form_state) {
  $form_state['element']['#settings']['field_name'] = $form_state['values']['settings']['field_name'];
}

/**
 * Validate the chosen value or php code.
 */
function content_rules_action_populate_field_validate($form, &$form_state) {
  if (!isset($form_state['element']['#settings']['field_name'])) {
    //Just validate the last step.
    return;
  }

  if (isset($form_state['values']['code']) && ($php = $form_state['values']['code'])) {
    if (strpos($php, 'return') === FALSE) {
      form_set_error('code', t('You have to return the default value in the expected format.'));
    }
  }
  else {
    // Validate the field.
    $settings = $form_state['element']['#settings'];
    $field = content_fields($settings['field_name']);
    $field_types = _content_field_types();
    $function = $field_types[$field['type']]['module'] .'_field';
    if (function_exists($function)) {
      $form['#node'] = (object)array('type' => '', $settings['field_name'] => $form_state['values'][$settings['field_name']]);
      $items = isset($form['#node']->{$field['field_name']}) ? $form['#node']->{$field['field_name']} : array();

      //Make sure AHAH 'add more' button isn't sent to the fields
      // for processing.
      unset($items[$field['field_name'] .'_add_more']);

      $function('validate', $form['#node'], $field, $items, $form, NULL);
      content_field('validate', $form['#node'], $field, $items, $form, NULL);
    }
  }
}

function content_rules_action_populate_field_submit(&$settings, $form, &$form_state) {
  // Take over field values and filter out private properties added by CCK
  $settings['value'] = array_filter($form_state['values'][$settings['field_name']], 'is_array');

  foreach ($settings['value'] as $key => $data) {
    foreach (array_filter(array_keys($data)) as $col) {
      if ($col[0] == '_') {
        unset($settings['value'][$key][$col]);
      }
    }
    if ($key && count(array_filter($settings['value'][$key])) == 0) {
      // For multi-valued fields don't check for any additional empty values.
      unset($settings['value'][$key]);
    }
  }

  $settings['code'] = $form_state['values']['code'];

  if (function_exists('rules_action_custom_php_submit')) {
    // Support adding variables to the php code, if php module is present.
    rules_action_custom_php_submit($settings, $form, $form_state);
  }

  // Add all values to the input evaluator, so that textfields / textares can
  // make use of it.
  $names = array('code');

  foreach ($settings['value'] as $key => $data) {
    foreach (array_filter($data, 'is_string') as $col => $value) {
      $names[] = "value|$key|$col";
    }
  }
  $form_state['element']['#info']['eval input'] = $names;
}


/**
 * Label callback: Improve the label of the action.
 */
function content_rules_action_populate_field_label($settings, $argument_labels) {
  return t("Populate @node's field '@field'", array('@field' => $settings['field_name']) + $argument_labels);
}

function workflow_ng_action_populate_field_upgrade(&$element) {
  $element['#name'] = 'content_rules_action_populate_field';
  $element['#settings']['code'] = $element['#settings']['default_value_php'];
  $element['#settings'][$element['#settings']['field_name']] = array();
  unset($element['#settings']['default_value_php']);
}


/**
 * Implementation of hook_rules_condition_info().
 */
function content_rules_condition_info() {
  $info = array();
  $info['content_rules_field_has_value'] = array(
    'label' => t('Field has value'),
    'arguments' => array(
      'node' => array('type' => 'node', 'label' => t('Content')),
    ),
    'eval input' => array('code'),
    'help' => t('You should make sure that the used field exists in the given content type. The condition returns TRUE, if the selected field has the given value.'),
    'module' => 'CCK',
  );
  $info['content_rules_field_changed'] = array(
    'label' => t('Field has changed'),
    'arguments' => array(
      'node' => array('type' => 'node', 'label' => t('Content containing changes')),
      'node_unchanged' => array('type' => 'node', 'label' => t('Content not containing changes')),
    ),
    'help' => t('You should make sure that the used field exists in the given content type.'),
    'module' => 'CCK',
  );
  return $info;
}

/**
 * Condition: Check the value of a field.
 */
function content_rules_field_has_value($node, $settings) {
  // Get information about the field.
  $field = content_fields($settings['field_name'], $node->type);
  $value = _content_rules_get_field_value($settings, $state);

  if (empty($field) || !is_array($value)) {
    return FALSE;
  }

  return _content_rules_field_has_value($node->{$settings['field_name']}, $value);
}

/**
 * Use the same configuration form as the "populate field" action.
 */
function content_rules_field_has_value_form($settings, &$form, &$form_state) {
  content_rules_action_populate_field_form($settings, $form, $form_state);
}
function content_rules_field_has_value_validate($form, &$form_state) {
  content_rules_action_populate_field_validate($form, $form_state);
}
function content_rules_field_has_value_submit(&$settings, $form, &$form_state) {
  content_rules_action_populate_field_submit($settings, $form, $form_state);
}

function content_rules_field_has_value_label($settings, $argument_labels) {
  return t("@node's field '@field' has value", array('@field' => $settings['field_name']) + $argument_labels);
}

/**
 * Condition: Check if the field has changed.
 */
function content_rules_field_changed($node1, $node2, $settings) {
  // Get information about the field.
  $field = content_fields($settings['field_name'], $node1->type);

  return !empty($field) && !_content_rules_field_has_value($node1->{$settings['field_name']}, $node2->{$settings['field_name']});
}

function content_rules_field_changed_form($settings, &$form, &$form_state) {
  $settings += array('field_name' => '');
  $form['settings']['field_name'] = array(
    '#type' => 'select',
    '#title' => t('Field'),
    '#options' => content_rules_get_field_names_by_type(),
    '#default_value' => $settings['field_name'],
    '#description' => t('Select the machine-name of the field to look at.'),
    '#required' => TRUE,
  );
}

function content_rules_field_changed_label($settings, $argument_labels) {
  return t("@node's field '@field' has been changed", array('@field' => $settings['field_name']) + $argument_labels);
}


/**
 * Returns the fields of a given field type only.
 * Suitable for using it with #options.
 */
function content_rules_get_field_names_by_type($type = NULL) {
  $fields = array();
  foreach (content_fields() as $field) {
    if (!isset($type) || $field['type'] == $type) {
      $fields[$field['field_name']] = $field['field_name'];
    }
  }
  asort($fields);
  return $fields;
}

function _content_rules_get_field_value($settings, &$state) {
  if ($settings['code']) {
    if (function_exists('rules_input_evaluator_php_apply')) {
      // Support adding variables to the php code, if php module is present.
      $value = rules_input_evaluator_php_apply($settings['code'], $settings['vars'], $state, FALSE);
    }
    else {
      ob_start();
      $value = eval($settings['code']);
      ob_end_clean();
    }
  }
  else {
    $value = $settings['value'];
  }
  return $value;
}

/**
 * Checks whether both field values match in a robust way.
 *
 * It returns TRUE, only if the number of multiple values matches and
 * each property of the cck field's value is the same in the node.
 *
 * @param $node_value The value present in the node.
 * @param $value The value to check for.
 */
function _content_rules_field_has_value($node_value, $value) {
  if (count($value) != count($node_value)) {
    return FALSE;
  }
  // Loop over multiple fields
  foreach ($value as $delta => $sub_value) {
    // Check if all properties of the value are there in the node value too
    if (is_array($sub_value) && is_array($node_value[$delta])) {
      if (count(array_diff_assoc($sub_value, $node_value[$delta])) != 0) {
        return FALSE;
      }
    }
    elseif ($sub_value !== $node_value[$delta]) {
      return FALSE;
    }
  }
  return TRUE;
}
