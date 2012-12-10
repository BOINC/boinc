<?php

/**
 * Variable management strongarm form.
 */
function strongarm_admin_form($form_state) {
  global $conf;
  $vars = strongarm_vars_load(TRUE, TRUE);
  $form = array('#theme' => 'strongarm_admin_form',);
  foreach ($vars as $name => $variable) {
    if ($variable->export_type & EXPORT_IN_CODE) {
      $default = ctools_get_default_object('variable', $name);

      // If variable value does not match global $conf, this value has been
      // hardcoded (e.g. in settings.php) and has been allowed to pass
      // through. It cannot be reverted.
      $hardcoded = FALSE;
      $restorable = FALSE;
      if (isset($conf[$name]) && $conf[$name] !== $variable->value) {
        $storage = t('Hardcoded');
        $hardcoded = TRUE;
      }
      elseif (!empty($variable->in_code_only)) {
        $storage = t('In code');
        $restorable = TRUE;
      }
      elseif ($variable->value != $default->value) {
        $storage = t('Overridden');
        $restorable = TRUE;
      }
      else {
        $storage = t('Saved to DB');
      }

      $value = $hardcoded ? $conf[$name] : $variable->value;

      // If the variable is in the database and differs from its code value,
      // allow administrator to revert its value.
      if ($restorable) {
        $form['revert']['#tree'] = TRUE;
        $form['revert'][$name]['revert'] = array('#type' => 'checkbox');
        $form['revert'][$name]['value'] = array('#type' => 'value', '#value' => $default->value);
      }

      $form['name'][$name] = array(
        '#type' => 'markup',
        '#value' => $name,
      );
      $form['storage'][$name] = array(
        '#type' => 'markup',
        '#value' => $storage,
      );
      $form['value'][$name] = array(
        '#type' => 'markup',
        '#value' => check_plain(_strongarm_readable($value)),
      );
    }
  }
  if (!empty($form['revert'])) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Restore values to DB'),
      '#submit' => array('strongarm_admin_revert_submit'),
    );
  }
  return $form;
}

/**
 * Revert form submit handler.
 */
function strongarm_admin_revert_submit(&$form, &$form_state) {
  if (!empty($form_state['values']['revert'])) {
    foreach ($form_state['values']['revert'] as $name => $revert) {
      if ($revert['revert']) {
        variable_set($name, $revert['value']);
      }
    }
    strongarm_flush_caches();
  }
}

/**
 * Display variables in a nicer way.
 */
function _strongarm_readable($var) {
  if (is_string($var) || is_numeric($var)) {
    return truncate_utf8($var, 30, TRUE, TRUE);
  }
  else if (is_bool($var)) {
    return $var ? 'TRUE' : 'FALSE';
  }
  else if (is_array($var)) {
    $test = $detected = array();
    $test['keys'] = array_keys($var);
    $test['values'] = array_values($var);

    foreach ($test as $type => $values) {
      $numeric = TRUE;
      $sequential = 0;
      $boolean = TRUE;
      foreach ($values as $v) {
        $numeric = is_numeric($v) && $numeric;
        $sequential = is_numeric($v) && ($sequential == $v) && $sequential !== FALSE ? $sequential + 1 : FALSE;
        $boolean = $boolean && ($v === 0 || $v === 1 || $v === '1' || $v === '0' || $v === TRUE || $v === FALSE);
      }
      $detected[$type]['numeric'] = $numeric;
      $detected[$type]['sequential'] = $sequential !== FALSE;
      $detected[$type]['boolean'] = $boolean;
    }

    // List of things
    if (!empty($var) && $detected['keys']['numeric'] && $detected['keys']['sequential']) {
      return truncate_utf8(implode(', ', $var), 30, TRUE, TRUE);
    }
    return '-';
  }
}

/**
 * Theme function for the strongarm admin form.
 */
function theme_strongarm_admin_form($form) {
  drupal_add_js('misc/tableselect.js');
  $rows = $headers = array();
  $headers[] = theme('table_select_header_cell');
  $headers[] = t('Variable');
  $headers[] = t('Storage');
  $headers[] = t('Value');
  foreach (element_children($form['name']) as $name) {
    $row = array();
    $row[] = isset($form['revert'][$name]) ? drupal_render($form['revert'][$name]) : '';
    $row[] = drupal_render($form['name'][$name]);
    $row[] = drupal_render($form['storage'][$name]);
    $row[] = drupal_render($form['value'][$name]);
    $rows[] = $row;
  }
  $output = theme('table', $headers, $rows);
  $output .= drupal_render($form);
  return $output;
}
