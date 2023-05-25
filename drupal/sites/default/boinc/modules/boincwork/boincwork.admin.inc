<?php
// $Id$

/**
 * @file
 * Administration page callbacks for the boincwork module.
 */

/**
 * Allow configuration of general options for preference pages
 */
function boincwork_admin_prefs_options_form(&$form_state) {
  $form = array();
  $form['beta'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow users to opt in to test beta applications'),
    '#default_value' => variable_get('boinc_prefs_options_beta', 0),
    '#required' => TRUE
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );
  // Add the official mechanical things and return
  //drupal_prepare_form('boincwork_admin_prefs_upload_form', $form, $form_state);
  return $form;
}

/**
  * Handle validation of preference general settings form.
  */
function boincwork_admin_prefs_options_form_validate($form, &$form_state) {
}

/**
  * Handle submission of preference general settings form.
  */
function boincwork_admin_prefs_options_form_submit($form, &$form_state) {
  variable_set('boinc_prefs_options_beta', $form_state['values']['beta']);
  drupal_set_message('Preference options have been updated.');
}

/**
 *
 */
function boincwork_admin_prefs_upload_form(&$form_state) {
  $form = array();
  $form['prefs_xml'] = array(
    '#type' => 'textarea',
    '#title' => t('Project specific preferences XML'),
    '#default_value' => variable_get('boinc_project_specific_prefs_config', "<project_specific_preferences>\n\n</project_specific_preferences>"),
    '#required' => TRUE
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );
  // Add the official mechanical things and return
  //drupal_prepare_form('boincwork_admin_prefs_upload_form', $form, $form_state);
  return $form;
}

/**
  * Handle validation of preference upload form.
  */
function boincwork_admin_prefs_upload_form_validate($form, &$form_state) {

  $xsd = './' . drupal_get_path('module', 'boincwork') . '/includes/projectprefs.xsd';
  libxml_use_internal_errors(true);
  $xml = new DomDocument();
  $xml->loadXML($form_state['values']['prefs_xml'], LIBXML_NOBLANKS);
  if (!$xml->schemaValidate($xsd)) {
    $errors = libxml_get_errors();
    $lines = explode("\r", $form_state['values']['prefs_xml']);
    drupal_set_message("{$errors[0]->message} at line {$errors[0]->line}" .
      ': <br/>' . htmlentities($lines[$errors[0]->line - 1]), 'error');
    form_set_error('upload', t('XML file failed validation'));
  }
}

/**
  * Handle submission of preference upload form.
  */
function boincwork_admin_prefs_upload_form_submit($form, &$form_state) {
  variable_set('boinc_project_specific_prefs_config', $form_state['values']['prefs_xml']);
  drupal_set_message('The XML has been validated and any changes to
      preferences are now in place.');
}

/**
 *
 */
function boincwork_admin_prefs_presets_page($preset = 'standard') {
  // Configure preference presets
  if (function_exists('jump_quickly')) {
    $path = 'admin/boinc/prefs/presets';
    $preset_options = array(
      "{$path}/standard" => t('Standard'),
      "{$path}/maximum" => t('Maximum'),
      "{$path}/green" => t('Green'),
      "{$path}/minimum" => t('Minimum')
    );
    variable_set('jump_use_js_presets-Array', 1);
    drupal_add_js(drupal_get_path('module', 'jump') . '/jump.js');
    $output .= '<div class="simple-form-controls"><div class="form-item venue">';
    $output .= '<label>Preset:</label>';
    $output .= jump_quickly($preset_options, 'presets');
    $output .= '</div></div>';
  }
  $output .= drupal_get_form('boincwork_admin_prefs_presets_form', $preset);

  $output .= '<div>';
  $output .= bts('Usage: \'Save configuration\' will save the above preferences into the drupal database. These will be the preset computing (global) preferences that will fill in the preferences form once a user loads the Account Preferences page.', array(), NULL, 'boinc:admin-boinc-preference-presets');
  $output .= '<p>';
  $output .= bts('\'Save configuration with disk usage settings from config.xml\' will save the above preferences into the drupal database, but also load disk usage settings from the BOINC project\'s config.xml file. These will overwrite any disk usage settings you have placed above. Caution: A user\'s personal preferences will not change if you load the disk usage settings from config.xml here into the drupal database. They must change their preference settings manually.', array(), NULL, 'boinc:admin-boinc-preference-presets');
  $output .= '</div>';
  return $output;
}

/**
 *
 */
function boincwork_admin_prefs_presets_form(&$form_state, $preset = 'standard') {

  // Check database for preset prefs
  if (!variable_get('boincwork_preset_prefs', null))
    drupal_set_message(bts('No presets found in database, loading a default set of preferences for each preset. These will be saved to the database once you click \'Save configuration\'.', array(), NULL, 'boinc:admin-boinc-preference-presets'), 'status');

  // Load a copy of the general prefs form
  $form = boincwork_generalprefs_form($form_state, NULL, $preset);
  if ($key = array_search('boincwork_generalprefs_form_submit', $form['#submit'])) {
    unset($form['#submit'][$key]);
  }
  $form['#submit'][] = 'boincwork_admin_prefs_presets_form_submit';

  // Just keep the necessary items
  $form['processor'] = $form['prefs']['advanced']['processor'];
  $form['storage'] = $form['prefs']['advanced']['storage'];
  $form['network'] = $form['prefs']['advanced']['network'];
  unset($form['prefs']);

  // Tweak some things so it behaves a little better
  $form['#tree'] = TRUE;
  $form['processor']['#collapsible'] = TRUE;
  $form['storage']['#collapsible'] = TRUE;
  $form['network']['#collapsible'] = TRUE;

  // Add a submit button and a hidden field to pass the preset
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );
  $form['saveuseconfigxml'] = array (
      '#type' => 'submit',
      '#value' => t('Save configuration with disk usage settings from config.xml'),
      '#validate' => array('boincwork_admin_prefs_preset_saveuseconfigxml'),
  );
  $form['preset'] = array(
    '#type' => 'hidden',
    '#value' => $preset
  );

  // Add the official mechanical things and return
  drupal_prepare_form('boincwork_admin_prefs_presets_form', $form, $form_state);
  return $form;
}

/**
  * Validate the preference presets form.
  */
function boincwork_admin_prefs_presets_form_validate($form, &$form_state) {
  require_boinc('util');
  $values = $form_state['values'];

  // Verify all non-boolean user input values and notify form API of failures

  // Processing preferences
  if (!verify_numeric($values['processor']['idle_time_to_run'], 1, 9999)) form_set_error('idle_time_to_run', t('Invalid setting for') . " \"{$form['processor']['idle_time_to_run']['#title']} [x] {$form['processor']['idle_time_to_run']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['suspend_if_no_recent_input'], 0, 9999)) form_set_error('suspend_if_no_recent_input', t('Invalid setting for') . " \"{$form['processor']['suspend_if_no_recent_input']['#title']} [x] {$form['processor']['suspend_if_no_recent_input']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['suspend_cpu_usage'], 0, 100)) form_set_error('suspend_cpu_usage', t('Invalid setting for') . " \"{$form['processor']['suspend_cpu_usage']['#title']} [x] {$form['processor']['suspend_cpu_usage']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['start_hour'], 0, 23)) form_set_error('start_hour', t('Invalid setting for') . " \"{$form['processor']['start_hour']['#title']} [x] {$form['processor']['start_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['end_hour'], 0, 23)) form_set_error('end_hour', t('Invalid setting for') . " \"{$form['processor']['end_hour']['#title']} [x] {$form['processor']['end_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['cpu_scheduling_period_minutes'], 1, 9999)) form_set_error('cpu_scheduling_period_minutes', t('Invalid setting for') . " \"{$form['processor']['cpu_scheduling_period_minutes']['#title']} [x] {$form['processor']['cpu_scheduling_period_minutes']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['max_ncpus_pct'], 0, 100)) form_set_error('max_ncpus_pct', t('Invalid setting for') . " \"{$form['processor']['max_ncpus_pct']['#title']} [x] {$form['processor']['max_ncpus_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['cpu_usage_limit'], 0, 100)) form_set_error('cpu_usage_limit', t('Invalid setting for') . " \"{$form['processor']['cpu_usage_limit']['#title']} [x] {$form['processor']['cpu_usage_limit']['#field_suffix']}\"");

  // Storage preferences
  if (!verify_numeric($values['storage']['disk_max_used_gb'], 0, 9999999)) form_set_error('disk_max_used_gb', t('Invalid setting for') . " \"{$form['storage']['disk_max_used_gb']['#title']} [x] {$form['storage']['disk_max_used_gb']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_min_free_gb'], 0.001, 9999999)) form_set_error('disk_min_free_gb', t('Invalid setting for') . " \"{$form['storage']['disk_min_free_gb']['#title']} [x] {$form['storage']['disk_min_free_gb']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_max_used_pct'], 0, 100)) form_set_error('disk_max_used_pct', t('Invalid setting for') . " \"{$form['storage']['disk_max_used_pct']['#title']} [x] {$form['storage']['disk_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_interval'], 0, 9999999)) form_set_error('disk_interval', t('Invalid setting for') . " \"{$form['storage']['disk_interval']['#title']} [x] {$form['storage']['disk_interval']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['vm_max_used_pct'], 0, 100)) form_set_error('vm_max_used_pct', t('Invalid setting for') . " \"{$form['storage']['vm_max_used_pct']['#title']} [x] {$form['storage']['vm_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['ram_max_used_busy_pct'], 0, 100)) form_set_error('ram_max_used_busy_pct', t('Invalid setting for') . " \"{$form['storage']['ram_max_used_busy_pct']['#title']} [x] {$form['storage']['ram_max_used_busy_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['ram_max_used_idle_pct'], 0, 100)) form_set_error('ram_max_used_idle_pct', t('Invalid setting for') . " \"{$form['storage']['ram_max_used_idle_pct']['#title']} [x] {$form['storage']['ram_max_used_idle_pct']['#field_suffix']}\"");

  // Network preferences
  if (!verify_numeric($values['network']['work_buf_min_days'], 0, 10)) form_set_error('work_buf_min_days', t('Invalid setting for') . " \"{$form['network']['work_buf_min_days']['#title']} [x] {$form['network']['work_buf_min_days']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['work_buf_additional_days'], 0, 10)) form_set_error('work_buf_additional_days', t('Invalid setting for') . " \"{$form['network']['work_buf_additional_days']['#title']} [x] {$form['network']['work_buf_additional_days']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['max_bytes_sec_down'], 0, 9999.999)) form_set_error('max_bytes_sec_down', t('Invalid setting for') . " \"{$form['network']['max_bytes_sec_down']['#title']} [x] {$form['network']['max_bytes_sec_down']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['max_bytes_sec_up'], 0, 9999.999)) form_set_error('max_bytes_sec_up', t('Invalid setting for') . " \"{$form['network']['max_bytes_sec_up']['#title']} [x] {$form['network']['max_bytes_sec_up']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['net_start_hour'], 0, 23)) form_set_error('net_start_hour', t('Invalid setting for') . " \"{$form['network']['net_start_hour']['#title']} [x] {$form['network']['net_start_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['net_end_hour'], 0, 23)) form_set_error('net_end_hour', t('Invalid setting for') . " \"{$form['network']['net_end_hour']['#title']} [x] {$form['network']['net_end_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['daily_xfer_limit_mb'], 0, 9999999)) form_set_error('daily_xfer_limit_mb', t('Invalid setting for') . " \"{$form['network']['daily_xfer_limit_mb']['#title']} [x] {$form['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['daily_xfer_period_days'], 0, 9999999)) form_set_error('daily_xfer_period_days', t('Invalid setting for') . " \"{$form['network']['daily_xfer_limit_mb']['#title']} [x] {$form['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
}

/**
  * Handle post-validation submission of preference presets form.
  */
function boincwork_admin_prefs_presets_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $preset = $form_state['values']['preset'];

  // Load baseline settings from configuration
  $prefs = boincwork_get_preset_prefs($preset);

  // Processing preferences
  $prefs['run_on_batteries'] = ($values['processor']['run_on_batteries']) ? 0 : 1;
  $prefs['run_if_user_active'] = ($values['processor']['run_if_user_active']) ? 0 : 1;
  $prefs['run_gpu_if_user_active'] = ($values['processor']['run_gpu_if_user_active']) ? 0: 1;
  $prefs['idle_time_to_run'] = $values['processor']['idle_time_to_run'];
  $prefs['suspend_if_no_recent_input'] = $values['processor']['suspend_if_no_recent_input'];
  $prefs['suspend_cpu_usage'] = $values['processor']['suspend_cpu_usage'];
  $prefs['start_hour'] = $values['processor']['start_hour'];
  $prefs['end_hour'] = $values['processor']['end_hour'];
  $prefs['leave_apps_in_memory'] = ($values['processor']['leave_apps_in_memory']) ? 1 : 0;
  $prefs['cpu_scheduling_period_minutes'] = $values['processor']['cpu_scheduling_period_minutes'];
  $prefs['max_ncpus_pct'] = $values['processor']['max_ncpus_pct'];
  $prefs['cpu_usage_limit'] = $values['processor']['cpu_usage_limit'];

  // Storage preferences
  $prefs['disk_max_used_gb'] = $values['storage']['disk_max_used_gb'];
  $prefs['disk_min_free_gb'] = $values['storage']['disk_min_free_gb'];
  $prefs['disk_max_used_pct'] = $values['storage']['disk_max_used_pct'];
  $prefs['disk_interval'] = $values['storage']['disk_interval'];
  $prefs['vm_max_used_pct'] = $values['storage']['vm_max_used_pct'];
  $prefs['ram_max_used_busy_pct'] = $values['storage']['ram_max_used_busy_pct'];
  $prefs['ram_max_used_idle_pct'] = $values['storage']['ram_max_used_idle_pct'];

  // Network preferences
  $prefs['work_buf_min_days'] = $values['network']['work_buf_min_days'];
  $prefs['work_buf_additional_days'] = $values['network']['work_buf_additional_days'];
  $prefs['confirm_before_connecting'] = ($values['network']['confirm_before_connecting']) ? 1 : 0;
  $prefs['hangup_if_dialed'] = ($values['network']['hangup_if_dialed']) ? 1 : 0;
  $prefs['max_bytes_sec_down'] = $values['network']['max_bytes_sec_down']*1000;
  $prefs['max_bytes_sec_up'] = $values['network']['max_bytes_sec_up']*1000;
  $prefs['net_start_hour'] = $values['network']['net_start_hour'];
  $prefs['net_end_hour'] = $values['network']['net_end_hour'];
  $prefs['daily_xfer_limit_mb'] = $values['network']['daily_xfer_limit_mb'];
  $prefs['daily_xfer_period_days'] = $values['network']['daily_xfer_period_days'];
  $prefs['dont_verify_images'] = ($values['network']['dont_verify_images']) ? 1 : 0;

  //Remove @attributes to match new format (see boincwork.forms.inc
  //function boincwork_generalprefs_form)
  unset($prefs['@attributes']['preset']);

  // Update the configuration
  boincwork_save_preset_prefs($prefs, $preset);
  drupal_set_message(t('The "@name" preset has been updated.',
    array('@name' => $preset)));
}

/**
 * Save preset preferences configuration
 */
function boincwork_save_preset_prefs($updated_prefs, $preset = 'standard') {

  // Get the full configuration
  $all_presets = boincwork_get_preset_prefs();
  $all_presets = (array) $all_presets['general_preferences'];

  // Check for sane config
  if (isset($all_presets['preset'])) {
    if (!is_numeric(key($all_presets['preset']))) {
      $all_presets['preset'] = array($all_presets['preset']);
    }
    foreach ($all_presets['preset'] as $key => $old_preset) {
      // Find the preset being updated and... update it
      if (isset($old_preset['@attributes']['name']) AND $old_preset['@attributes']['name'] == $preset) {
        $all_presets['preset'][$key] = $updated_prefs + $old_preset;
      }
    }
  }

  // Convert prefs back to XML and save to database
  $all_presets = array('general_preferences' => $all_presets);
  variable_set('boincwork_preset_prefs', save_configuration($all_presets));
}

/**
 * Saves but also loads disk usage settings from config.xml, and
 * overwrites whatever values are is in form.
 */
function boincwork_admin_prefs_preset_saveuseconfigxml($form, &$form_state) {
    require_boinc(array('db', 'prefs'));
    $disk_space_config = get_disk_space_config();

    $form_state['values']['storage']['disk_max_used_gb']  = $disk_space_config->disk_max_used_gb;
    $form_state['values']['storage']['disk_min_free_gb']  = $disk_space_config->disk_min_free_gb;
    $form_state['values']['storage']['disk_max_used_pct'] = $disk_space_config->disk_max_used_pct;

    //Run the form validate function
    boincwork_admin_prefs_presets_form_validate($form, $form_state);
}
