<?php

// $Id: input_formats.admin.inc,v 1.4 2010/07/30 20:28:07 dagmar Exp $

/**
 * @file
 * Admin page callbacks for the input formats module.
 */


function theme_input_formats_filter_admin_overview($form) {
  $rows = array();
  foreach ($form as $name => $element) {
    if (isset($element['roles']) && is_array($element['roles'])) {
      $rows[] = array(
        drupal_render($form['default'][$element['id']['#value']]),
        check_plain($name) . ' <em>(' . drupal_render($element['type']) . ')</em>',
        drupal_render($element['roles']),
        drupal_render($element['configure']),
        drupal_render($element['export']),
        drupal_render($element['delete'])
      );
      unset($form[$name]);
    }
  }
  $header = array(t('Default'), t('Name'), t('Roles'), array('data' => t('Operations'), 'colspan' => 3));
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form);

  return $output;
}

/**
 * Menu callback; confirm revert of an input format.
 */
function input_formats_admin_revert() {
  $format = arg(4);
  $format = db_fetch_object(db_query('SELECT * FROM {filter_formats} WHERE format = %d', $format));

  if ($format) {
    $input_format = input_formats_load_by_format($format->format);
    if ($format->format != variable_get('filter_default_format', 1)) {
      $form['input_format'] = array('#type' => 'value', '#value' => $input_format);
      $form['format'] = array('#type' => 'hidden', '#value' => $format->format);
      $form['name'] = array('#type' => 'hidden', '#value' => $format->name);

      return confirm_form($form, t('Are you sure you want to revert the input format %format?', array('%format' => $format->name)), 'admin/settings/filters', t('All the settings for this Input Format will be deleted and restored from code.'), t('Revert'), t('Cancel'));
    }
    else {
      drupal_set_message(t('The default format cannot be reverted.'));
      drupal_goto('admin/settings/filters');
    }
  }
  else {
    drupal_not_found();
  }
}

/**
 * Process filter revert form submission.
 */
function input_formats_admin_revert_submit($form, &$form_state) {
  $input_format = $form_state['values']['input_format'];

  input_formats_revert($input_format);

  $form_state['redirect'] = 'admin/settings/filters';
  drupal_set_message(t('Settings for Input Format %format have been reverted from code.', array('%format' => $form_state['values']['name'])));
}

/**
 * Export an input format configuration and display it in a form.
 */
function input_formats_admin_export(&$form_state, $format) {
  ctools_include('export');
  input_formats_load_all(TRUE);
  $input_format = input_formats_load_by_format($format->format);

  drupal_set_title(t('Export @format', array('@format' => $format->name)));

  $code = ctools_export_object('filter_formats', $input_format);
  return ctools_export_form($form_state, $code, t('Exported code'));
}


function input_formats_admin_import() {
  $form['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('Optional. Use this machine name instead the provided in the import code.'),
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Optional. Use this name instead the provided in the import code.'),
  );
  $form['import'] = array(
    '#type' => 'textarea',
    '#title' => t('Import code'),
    '#rows' => 10,
    '#description' => t('Paste here the code to import. If Name and Machine Name are provided, they will be used overriding values pasted here.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

function input_formats_admin_import_validate($form, &$form_state) {
  ob_start();
  eval($form_state['values']['import']);
  ob_end_clean();

  if (!isset($input_format)) {
    drupal_set_message(t('An error has occured and the input format could not be imported.'), 'error');
    return;
  }
  if (!empty($form_state['values']['machine_name'])) {
    $input_format->machine = $form_state['values']['machine_name'];
  }
  if (!empty($form_state['values']['name'])) {
    $input_format->name = $form_state['values']['name'];
  }
  if (db_result(db_query("SELECT 1 FROM {exportables} WHERE machine = '%s' AND type = 'input_formats'", $input_format->machine))) {
    return form_error($form['machine_name'], t('Filter format names needs to be unique. A format with the machine name %name already exists.', array('%name' => $input_format->machine)));
  }
  if (db_result(db_query("SELECT 1 FROM {filter_formats} WHERE name = '%s'", $input_format->name))) {
    return form_error($form['name'], t('Filter format names needs to be unique. A format named %name already exists.', array('%name' => $input_format->name)));
  }
}

/**
 * Submit function triggered when a input format is imported.
 */
function input_formats_admin_import_submit($form, $form_state) {
  $code = $form_state['values']['import'];
  @eval($code);

  if (!empty($form_state['values']['name'])) {
    $input_format->name = $form_state['values']['name'];
  }
  if (!empty($form_state['values']['machine_name'])) {
    $input_format->machine = $form_state['values']['machine_name'];
  }

  input_formats_save($input_format);
  $tmp_form = array();
  $tmp_form_state = array(
    'values' => array(
      'id' => '',
      'input_format' => $input_format,
    ),
  );
  input_formats_complete_override($tmp_form, $tmp_form_state);

  drupal_set_message(t('The input format was susccessfully imported'));
}
