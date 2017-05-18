<?php

/**
 * @file
 * Rules integration for the Flag module.
 */

/*******************************************************************************
 * Actions forms.
 ******************************************************************************/

function flag_rules_action_flag_form($settings, &$form, &$form_state) {
  $settings += array('permission_check' => FALSE);
  $form['settings']['permission_check'] = array(
    '#title' => t('Check access permissions against the user on whose behalf to flag'),
    '#type' => 'checkbox',
    '#default_value' => $settings['permission_check'],
  );
}

function flag_rules_action_flag_label($settings, $argument_labels, $element) {
  $flag = rules_get_element_variable($element, 'flag');
  return t('Flag @object, under "@flag-title"', $argument_labels + array('@flag-title' => $flag->get_title()));
}

function flag_rules_action_unflag_form($settings, &$form, &$form_state) {
  $settings += array('permission_check' => FALSE);
  $form['settings']['permission_check'] = array(
    '#title' => t('Check access permissions against the user on whose behalf to unflag'),
    '#type' => 'checkbox',
    '#default_value' => $settings['permission_check'],
  );
}

function flag_rules_action_unflag_label($settings, $argument_labels, $element) {
  $flag = rules_get_element_variable($element, 'flag');
  return t('Unflag @object, under "@flag-title"', $argument_labels + array('@flag-title' => $flag->get_title()));
}

function flag_rules_action_trim_form($settings, &$form, &$form_state) {
  // Move argument to center of form.
  $form['settings']['cutoff_size']['#weight'] = 2;
}

function flag_rules_action_trim_label($settings, $argument_labels, $element) {
  $flag = rules_get_element_variable($element, 'flag');
  return t('Trim "@flag-title" at @size', array('@flag-title' => $flag->get_title(), '@size' => $settings['cutoff_size']));
}

function flag_rules_action_trim_help() {
  return t('The <em>trim</em> action is used to restrict the number of objects that may be flagged. For example, you may wish your "Editor picks" queue (that is, flag) to contain a maximum of 3 nodes. The trim action is best understood when we think of a flag as a <em>queue</em>. The action works by discarding old flaggings; So newly flagged objects push older ones out of the queue.');
}


/*******************************************************************************
 * Conditions forms.
 ******************************************************************************/


function flag_rules_condition_threshold_form($settings, &$form, &$form_state) {

  $form['settings']['operator'] = array(
    '#title' => t('Comparison operator'),
    '#type' => 'select',
    '#options' => array(
      '>'  => t('Greater than'),
      '>=' => t('Greater than or equal'),
      '='  => t('Equal to'),
      '<=' => t('Less than or equal'),
      '<'  => t('Less than'),
    ),
    '#default_value' => isset($settings['operator']) ? $settings['operator'] : '>=',
  );

  // Move argument to bottom of form.
  $form['settings']['number']['#weight'] = 2;

  // Move tokens help to bottom of form.
  $form['settings']['input_help'] = $form['input_help'];
  $form['settings']['input_help']['#weight'] = 3;
  unset($form['input_help']);
}

function flag_rules_condition_threshold_label($settings, $argument_labels, $element) {
  $flag = rules_get_element_variable($element, 'flag');
  return t('Flagging count !op @number, for @object, under "@flag-title"', array('@flag-title' => $flag->get_title(), '!op' => $settings['operator'], '@number' => $settings['number']) + $argument_labels);
}

function flag_rules_condition_flagged_label($settings, $argument_labels, $element) {
  $flag = rules_get_element_variable($element, 'flag');
  return t('Flag "@flag-title" is flagged, for @object', array('@flag-title' => $flag->get_title()) + $argument_labels);
}

/*******************************************************************************
 * Form utilities for the default input form of flags.
 ******************************************************************************/

function theme_flag_rules_radios($element) {

  if (!empty($element['#disabled'])) {
    drupal_set_message($element['#description'], 'error');
  }

  $headers = array(t('Flag:'), t('The flag type'), t('Is the flag global?'));
  $rows = array();
  foreach (element_children($element) as $flag_name) {
    $flag = flag_get_flag($flag_name);
    $rows[] = array(
      '<div class="flag-radio">' . drupal_render($element[$flag_name]) . '</div>',
      $flag->content_type,
      $flag->global ? t('Yes') : t('No'),
    );
  }
  return theme('table', $headers, $rows);
}

function _flag_rules_flags_options($flag_type = NULL) {
  $flags = flag_get_flags();
  $options = array();
  foreach ($flags as $flag) {
    if (!isset($flag_type) || $flag->content_type == $flag_type) {
      $options[$flag->name] = $flag->get_title();
    }
  }
  return $options;
}

