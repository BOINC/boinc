<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General preferences form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The structure of the general preferences form
 */
function boincwork_generalprefs_form(&$form_state, $bilge, $venue, $initialize_if_empty = false) {
  require_boinc(array('user', 'prefs'));
  
  global $user;
  $drupaluser = user_load($user->uid);
  $prefs = null;
  
  // Load preferences from BOINC account
  $boincuser = BoincUser::lookup_id($drupaluser->boincuser_id);
  $main_prefs = prefs_parse_global($boincuser->global_prefs);
  
  if (!$venue OR $venue == 'default') $prefs = $main_prefs;
  elseif (isset($main_prefs->$venue)) $prefs = $main_prefs->$venue;
  elseif (!$initialize_if_empty) return null;
  
  // Define form defaults
  $disk_space_config = get_disk_space_config();
  $default = array(
    'processor' => array(
      'run_on_batteries' => (isset($prefs->run_on_batteries) AND !$prefs->run_on_batteries) ? 0 : 1,
      'run_if_user_active' => (isset($prefs->run_if_user_active) AND $prefs->run_if_user_active) ? 1 : 0,
      'run_gpu_if_user_active' => (isset($prefs->run_gpu_if_user_active) AND !$prefs->run_gpu_if_user_active) ? 0 : 1,
      'idle_time_to_run' => isset($prefs->idle_time_to_run) ? $prefs->idle_time_to_run : 3,
      'suspend_if_no_recent_input' => isset($prefs->suspend_if_no_recent_input) ? $prefs->suspend_if_no_recent_input : 0,
      'suspend_cpu_usage' => isset($prefs->suspend_cpu_usage) ? $prefs->suspend_cpu_usage : 0,
      'start_hour' => isset($prefs->start_hour) ? $prefs->start_hour : 0,
      'end_hour' => isset($prefs->end_hour) ? $prefs->end_hour : 0,
      'leave_apps_in_memory' => (isset($prefs->leave_apps_in_memory) AND $prefs->leave_apps_in_memory) ? 1 : 0,
      'cpu_scheduling_period_minutes' => isset($prefs->cpu_scheduling_period_minutes) ? $prefs->cpu_scheduling_period_minutes : 60,
      'max_cpus' => isset($prefs->max_cpus) ? $prefs->max_cpus : 16,
      'max_ncpus_pct' => isset($prefs->max_ncpus_pct) ? $prefs->max_ncpus_pct : 100,
      'cpu_usage_limit' => isset($prefs->cpu_usage_limit) ? $prefs->cpu_usage_limit : 100
    ),
    'storage' => array(
      'disk_max_used_gb' => isset($prefs->disk_max_used_gb) ? $prefs->disk_max_used_gb : $disk_space_config->disk_max_used_gb,
      'disk_min_free_gb' => isset($prefs->disk_min_free_gb) ? $prefs->disk_min_free_gb : $disk_space_config->disk_min_free_gb,
      'disk_max_used_pct' => isset($prefs->disk_max_used_pct) ? $prefs->disk_max_used_pct : $disk_space_config->disk_max_used_pct,
      'disk_interval' => isset($prefs->disk_interval) ? $prefs->disk_interval : 60,
      'vm_max_used_pct' => isset($prefs->vm_max_used_pct) ? $prefs->vm_max_used_pct : 75,
      'ram_max_used_busy_pct' => isset($prefs->ram_max_used_busy_pct) ? $prefs->ram_max_used_busy_pct : 50,
      'ram_max_used_idle_pct' => isset($prefs->ram_max_used_idle_pct) ? $prefs->ram_max_used_idle_pct : 90
    ),
    'network' => array(
      'work_buf_min_days' => isset($prefs->work_buf_min_days) ? $prefs->work_buf_min_days : 0,
      'work_buf_additional_days' => isset($prefs->work_buf_additional_days) ? $prefs->work_buf_additional_days : 0.25,
      'confirm_before_connecting' => (isset($prefs->confirm_before_connecting) AND $prefs->confirm_before_connecting) ? 1 : 0,
      'hangup_if_dialed' => (isset($prefs->hangup_if_dialed) AND $prefs->hangup_if_dialed) ? 1 : 0,
      'max_bytes_sec_down' => isset($prefs->max_bytes_sec_down) ? $prefs->max_bytes_sec_down : 0,
      'max_bytes_sec_up' => isset($prefs->max_bytes_sec_up) ? $prefs->max_bytes_sec_up : 0,
      'net_start_hour' => isset($prefs->net_start_hour) ? $prefs->net_start_hour : 0,
      'net_end_hour' => isset($prefs->net_end_hour) ? $prefs->net_end_hour : 0,
      'daily_xfer_limit_mb' => isset($prefs->daily_xfer_limit_mb) ? $prefs->daily_xfer_limit_mb : 0,
      'daily_xfer_period_days' => isset($prefs->daily_xfer_period_days) ? $prefs->daily_xfer_period_days : 0,
      'dont_verify_images' => (isset($prefs->dont_verify_images) AND $prefs->dont_verify_images) ? 1 : 0
    )
  );
  
  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => t('yes'), 0 => t('no'))
  );
  $form['hour_options'] = array(
    '#type' => 'value',
    '#value' => array(t('0:00'), t('1:00'), t('2:00'), t('3:00'), t('4:00'), 
      t('5:00'), t('6:00'), t('7:00'), t('8:00'), t('9:00'), t('10:00'), t('11:00'), 
      t('12:00'), t('13:00'), t('14:00'), t('15:00'), t('16:00'), t('17:00'),
      t('18:00'), t('19:00'), t('20:00'), t('21:00'), t('22:00'), t('23:00'))
  );
  
  // Hidden elements
  $form['venue'] = array(
    '#type' => 'hidden',
    '#value' => $venue
  );
  $form['modified'] = array(
    '#type' => 'hidden',
    '#value' => (isset($prefs->mod_time) ? $prefs->mod_time : null)
  );
  
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li></ul>'
  );
  
  // Processing preferences
  $form['processor'] = array(
    '#title' => t('Processor usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['processor']['run_on_batteries'] = array(
    '#title' => t('Run while computer is on battery power?'),
    '#type' => 'radios',
    '#description' => t('Only applies to portable computers'),
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['processor']['run_on_batteries']
  );
  $form['processor']['run_if_user_active'] = array(
    '#title' => t('Run while computer is in use?'),
    '#type' => 'radios',
    '#description' => ' ',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['processor']['run_if_user_active']
  );
  $form['processor']['run_gpu_if_user_active'] = array(
    '#title' => t('Run GPU work while computer is in use?'),
    '#type' => 'radios',
    '#description' => t('Enforced by version 6.6.21+'),
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['processor']['run_gpu_if_user_active']
  );
  $form['processor']['idle_time_to_run'] = array(
    '#title' => t('&apos;In use&apos; means mouse/keyboard activity in last'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['processor']['idle_time_to_run'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['processor']['suspend_if_no_recent_input'] = array(
    '#title' => t('Suspend work if no mouse/keyboard activity in last'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['processor']['suspend_if_no_recent_input'],
    '#size' => 1,
    '#description' => t('Needed to enter low-power mode on some computers')
  );
  $form['processor']['suspend_cpu_usage'] = array(
    '#title' => t('Suspend work if CPU usage is above'),
    '#type' => 'textfield',
    '#field_suffix' => t('%'),
    '#default_value' => $default['processor']['suspend_cpu_usage'],
    '#size' => 1,
    '#description' => t('0 means no restriction. Enforced by version 6.10.30+')
  );
  $form['processor']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . t('Do work only between the hours of:') . '</label></div>'
  );
  $form['processor']['start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['processor']['start_hour']
  );
  $form['processor']['hour_delimiter'] = array(
    '#value' => '<span>' . t('and') . '</span>'
  );
  $form['processor']['end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['processor']['end_hour']
  );
  $form['processor']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . t('No restriction if equal') . '</div></div>'
  );
  $form['processor']['leave_apps_in_memory'] = array(
    '#title' => t('Leave tasks in memory while suspended?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['processor']['leave_apps_in_memory'],
    '#description' => t('Suspended tasks will consume swap space if &apos;yes&apos;')
  );
  $form['processor']['cpu_scheduling_period_minutes'] = array(
    '#title' => t('Switch between tasks every'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['processor']['cpu_scheduling_period_minutes'],
    '#size' => 1,
    '#description' => t('Recommended: 60 minutes')
  );
  $form['processor']['max_cpus'] = array(
    '#title' => t('On multiprocessors, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('processors'),
    '#default_value' => $default['processor']['max_cpus'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['processor']['max_ncpus_pct'] = array(
    '#title' => t('On multiprocessors, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of the processors'),
    '#default_value' => $default['processor']['max_ncpus_pct'],
    '#size' => 1,
    '#description' => t('Enforced by version 6.1+')
  );
  $form['processor']['cpu_usage_limit'] = array(
    '#title' => t('Use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of the CPU time'),
    '#default_value' => $default['processor']['cpu_usage_limit'],
    '#size' => 1,
    '#description' => t('Can be used to reduce CPU heat')
  );
  
  // Disk and memory preferences
  $form['storage'] = array(
    '#title' => t('Disk and memory usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['storage']['disk_max_used_gb'] = array(
    '#title' => t('Disk: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('GB'),
    '#default_value' => $default['storage']['disk_max_used_gb'],
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['storage']['disk_min_free_gb'] = array(
    '#title' => t('Disk: leave free at least'),
    '#type' => 'textfield',
    '#field_suffix' => t('GB'),
    '#default_value' => $default['storage']['disk_min_free_gb'],
    '#size' => 1,
    '#description' => t('Values smaller than 0.001 are ignored')
  );
  $form['storage']['disk_max_used_pct'] = array(
    '#title' => t('Disk: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['storage']['disk_max_used_pct'],
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['storage']['disk_interval'] = array(
    '#title' => t('Tasks checkpoint to disk at most every'),
    '#type' => 'textfield',
    '#field_suffix' => t('seconds'),
    '#default_value' => $default['storage']['disk_interval'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['storage']['vm_max_used_pct'] = array(
    '#title' => t('Swap space: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['storage']['vm_max_used_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['storage']['ram_max_used_busy_pct'] = array(
    '#title' => t('Memory: when computer is in use, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['storage']['ram_max_used_busy_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['storage']['ram_max_used_idle_pct'] = array(
    '#title' => t('Memory: when computer is not in use, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['storage']['ram_max_used_idle_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  
  // Network preferences
  $form['network'] = array(
    '#title' => t('Network usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['network']['work_buf_min_days'] = array(
    '#title' => t('Computer is connected to the Internet about every'),
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['network']['work_buf_min_days'],
    '#size' => 1,
    '#description' => t('Leave blank or 0 if always connected. @project will try to maintain at least this much work.', array('@project' => PROJECT))
  ); 
  $form['network']['work_buf_additional_days'] = array(
    '#title' => t('Maintain enough work for an additional'),
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['network']['work_buf_additional_days'],
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['network']['confirm_before_connecting'] = array(
    '#title' => t('Confirm before connecting to Internet?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['network']['confirm_before_connecting'],
    '#description' => t('Matters only if you have a modem, ISDN, or VPN connection')
  ); 
  $form['network']['hangup_if_dialed'] = array(
    '#title' => t('Disconnect when done?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['network']['hangup_if_dialed'],
    '#description' => t('Matters only if you have a modem, ISDN, or VPN connection')
  );
  $form['network']['max_bytes_sec_down'] = array(
    '#title' => t('Maximum download rate'),
    '#type' => 'textfield',
    '#field_suffix' => t('Kbytes/sec'),
    '#default_value' => $default['network']['max_bytes_sec_down']/1000,
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['network']['max_bytes_sec_up'] = array(
    '#title' => t('Maximum upload rate'),
    '#type' => 'textfield',
    '#field_suffix' => t('Kbytes/sec'),
    '#default_value' => $default['network']['max_bytes_sec_up']/1000,
    '#size' => 1,
    '#description' => ' '
  );
  $form['network']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . t('Use network only between the hours of:') . '</label></div>'
  );
  $form['network']['net_start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['network']['net_start_hour']
  );
  $form['network']['hour_delimiter'] = array(
    '#value' => '<span>' . t('and') . '</span>'
  );
  $form['network']['net_end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['network']['net_end_hour']
  );
  $form['network']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . t('No restriction if equal') . '</div></div>'
  ); 
  $form['network']['daily_xfer_limit_mb'] = array(
    '#title' => t('Transfer at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('Mbytes'),
    '#default_value' => $default['network']['daily_xfer_limit_mb'],
    '#size' => 1
  ); 
  $form['network']['daily_xfer_period_days'] = array(
    '#field_prefix' => 'every',
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['network']['daily_xfer_period_days'],
    '#size' => 1,
    '#description' => t('Enforced by version 6.10.46+')
  ); 
  $form['network']['dont_verify_images'] = array(
    '#title' => t('Skip image file verification?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['network']['dont_verify_images'],
    '#description' => t('Check this ONLY if your Internet provider modifies image files (UMTS does this, for example). Skipping verification reduces the security of BOINC.')
  ); 
  
  return $form;
}

/**
  * Validate the general preferences form.
  */
function boincwork_generalprefs_form_validate($form, &$form_state) {
  require_boinc('util');
  
  // Verify all non-boolean user input values and notify form API of failures
  
  // Processing preferences
  if (!verify_numeric($form_state['values']['idle_time_to_run'], 1, 9999)) form_set_error('idle_time_to_run', t('Invalid setting for') . " \"{$form['processor']['idle_time_to_run']['#title']} [x] {$form['processor']['idle_time_to_run']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['suspend_if_no_recent_input'], 0, 9999)) form_set_error('suspend_if_no_recent_input', t('Invalid setting for') . " \"{$form['processor']['suspend_if_no_recent_input']['#title']} [x] {$form['processor']['suspend_if_no_recent_input']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['suspend_cpu_usage'], 0, 100)) form_set_error('suspend_cpu_usage', t('Invalid setting for') . " \"{$form['processor']['suspend_cpu_usage']['#title']} [x] {$form['processor']['suspend_cpu_usage']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['start_hour'], 0, 23)) form_set_error('start_hour', t('Invalid setting for') . " \"{$form['processor']['start_hour']['#title']} [x] {$form['processor']['start_hour']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['end_hour'], 0, 23)) form_set_error('end_hour', t('Invalid setting for') . " \"{$form['processor']['end_hour']['#title']} [x] {$form['processor']['end_hour']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['cpu_scheduling_period_minutes'], 1, 9999)) form_set_error('cpu_scheduling_period_minutes', t('Invalid setting for') . " \"{$form['processor']['cpu_scheduling_period_minutes']['#title']} [x] {$form['processor']['cpu_scheduling_period_minutes']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['max_cpus'], 0, 9999)) form_set_error('max_cpus', t('Invalid setting for') . " \"{$form['processor']['max_cpus']['#title']} [x] {$form['processor']['max_cpus']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['max_ncpus_pct'], 0, 100)) form_set_error('max_ncpus_pct', t('Invalid setting for') . " \"{$form['processor']['max_ncpus_pct']['#title']} [x] {$form['processor']['max_ncpus_pct']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['cpu_usage_limit'], 0, 100)) form_set_error('cpu_usage_limit', t('Invalid setting for') . " \"{$form['processor']['cpu_usage_limit']['#title']} [x] {$form['processor']['cpu_usage_limit']['#field_suffix']}\"");

  // Storage preferences
  if (!verify_numeric($form_state['values']['disk_max_used_gb'], 0, 9999999)) form_set_error('disk_max_used_gb', t('Invalid setting for') . " \"{$form['storage']['disk_max_used_gb']['#title']} [x] {$form['storage']['disk_max_used_gb']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['disk_min_free_gb'], 0.001, 9999999)) form_set_error('disk_min_free_gb', t('Invalid setting for') . " \"{$form['storage']['disk_min_free_gb']['#title']} [x] {$form['storage']['disk_min_free_gb']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['disk_max_used_pct'], 0, 100)) form_set_error('disk_max_used_pct', t('Invalid setting for') . " \"{$form['storage']['disk_max_used_pct']['#title']} [x] {$form['storage']['disk_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['disk_interval'], 0, 9999999)) form_set_error('disk_interval', t('Invalid setting for') . " \"{$form['storage']['disk_interval']['#title']} [x] {$form['storage']['disk_interval']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['vm_max_used_pct'], 0, 100)) form_set_error('vm_max_used_pct', t('Invalid setting for') . " \"{$form['storage']['vm_max_used_pct']['#title']} [x] {$form['storage']['vm_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['ram_max_used_busy_pct'], 0, 100)) form_set_error('ram_max_used_busy_pct', t('Invalid setting for') . " \"{$form['storage']['ram_max_used_busy_pct']['#title']} [x] {$form['storage']['ram_max_used_busy_pct']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['ram_max_used_idle_pct'], 0, 100)) form_set_error('ram_max_used_idle_pct', t('Invalid setting for') . " \"{$form['storage']['ram_max_used_idle_pct']['#title']} [x] {$form['storage']['ram_max_used_idle_pct']['#field_suffix']}\"");

  // Network preferences
  if (!verify_numeric($form_state['values']['work_buf_min_days'], 0, 10)) form_set_error('work_buf_min_days', t('Invalid setting for') . " \"{$form['network']['work_buf_min_days']['#title']} [x] {$form['network']['work_buf_min_days']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['work_buf_additional_days'], 0, 10)) form_set_error('work_buf_additional_days', t('Invalid setting for') . " \"{$form['network']['work_buf_additional_days']['#title']} [x] {$form['network']['work_buf_additional_days']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['max_bytes_sec_down'], 0, 9999.999)) form_set_error('max_bytes_sec_down', t('Invalid setting for') . " \"{$form['network']['max_bytes_sec_down']['#title']} [x] {$form['network']['max_bytes_sec_down']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['max_bytes_sec_up'], 0, 9999.999)) form_set_error('max_bytes_sec_up', t('Invalid setting for') . " \"{$form['network']['max_bytes_sec_up']['#title']} [x] {$form['network']['max_bytes_sec_up']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['net_start_hour'], 0, 23)) form_set_error('net_start_hour', t('Invalid setting for') . " \"{$form['network']['net_start_hour']['#title']} [x] {$form['network']['net_start_hour']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['net_end_hour'], 0, 23)) form_set_error('net_end_hour', t('Invalid setting for') . " \"{$form['network']['net_end_hour']['#title']} [x] {$form['network']['net_end_hour']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['daily_xfer_limit_mb'], 0, 9999999)) form_set_error('daily_xfer_limit_mb', t('Invalid setting for') . " \"{$form['network']['daily_xfer_limit_mb']['#title']} [x] {$form['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
  if (!verify_numeric($form_state['values']['daily_xfer_period_days'], 0, 9999999)) form_set_error('daily_xfer_period_days', t('Invalid setting for') . " \"{$form['network']['daily_xfer_limit_mb']['#title']} [x] {$form['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
}

/**
  * Handle post-validation submission of general preferences form.
  */
function boincwork_generalprefs_form_submit($form, &$form_state) {
  require_boinc(array('user', 'prefs'));
  
  global $user;
  $drupaluser = user_load($user->uid);
  
  $venue = $form_state['values']['venue'];
  
  // Load preferences from BOINC account
  $boincuser = BoincUser::lookup_id($drupaluser->boincuser_id);
  $main_prefs = prefs_parse_global($boincuser->global_prefs);
  $venue = (isset($main_prefs->$venue)) ? $venue : null;
  $prefs = ($venue) ? $main_prefs->$venue : $main_prefs;
  
  // Processing preferences
  $prefs->run_on_batteries = ($form_state['values']['run_on_batteries']) ? true : false;
  $prefs->run_if_user_active = ($form_state['values']['run_if_user_active']) ? true : false;
  $prefs->run_gpu_if_user_active = ($form_state['values']['run_gpu_if_user_active']) ? true: false;
  $prefs->idle_time_to_run = $form_state['values']['idle_time_to_run'];
  $prefs->suspend_if_no_recent_input = $form_state['values']['suspend_if_no_recent_input'];
  $prefs->suspend_cpu_usage = $form_state['values']['suspend_cpu_usage'];
  $prefs->start_hour = $form_state['values']['start_hour'];
  $prefs->end_hour = $form_state['values']['end_hour'];
  $prefs->leave_apps_in_memory = ($form_state['values']['leave_apps_in_memory']) ? true : false;
  $prefs->cpu_scheduling_period_minutes = $form_state['values']['cpu_scheduling_period_minutes'];
  $prefs->max_cpus = $form_state['values']['max_cpus'];
  $prefs->max_ncpus_pct = $form_state['values']['max_ncpus_pct'];
  $prefs->cpu_usage_limit = $form_state['values']['cpu_usage_limit'];
  
  // Storage preferences
  $prefs->disk_max_used_gb = $form_state['values']['disk_max_used_gb'];
  $prefs->disk_min_free_gb = $form_state['values']['disk_min_free_gb'];
  $prefs->disk_max_used_pct = $form_state['values']['disk_max_used_pct'];
  $prefs->disk_interval = $form_state['values']['disk_interval'];
  $prefs->vm_max_used_pct = $form_state['values']['vm_max_used_pct'];
  $prefs->ram_max_used_busy_pct = $form_state['values']['ram_max_used_busy_pct'];
  $prefs->ram_max_used_idle_pct = $form_state['values']['ram_max_used_idle_pct'];
  
  // Network preferences
  $prefs->work_buf_min_days = $form_state['values']['work_buf_min_days'];
  $prefs->work_buf_additional_days = $form_state['values']['work_buf_additional_days'];
  $prefs->confirm_before_connecting = ($form_state['values']['confirm_before_connecting']) ? true : false;
  $prefs->hangup_if_dialed = ($form_state['values']['hangup_if_dialed']) ? true : false;
  $prefs->max_bytes_sec_down = $form_state['values']['max_bytes_sec_down']*1000;
  $prefs->max_bytes_sec_up = $form_state['values']['max_bytes_sec_up']*1000;
  $prefs->net_start_hour = $form_state['values']['net_start_hour'];
  $prefs->net_end_hour = $form_state['values']['net_end_hour'];
  $prefs->daily_xfer_limit_mb = $form_state['values']['daily_xfer_limit_mb'];
  $prefs->daily_xfer_period_days = $form_state['values']['daily_xfer_period_days'];
  $prefs->dont_verify_images = ($form_state['values']['dont_verify_images']) ? true : false;
  
  if ($venue) $main_prefs->$venue = $prefs;
  else $main_prefs = $prefs;
  
  // Update BOINC DB (using BOINC prefs.inc)
  $prefs_xml = mysql_real_escape_string(global_prefs_make_xml($main_prefs));
  $result = mysql_query("UPDATE user SET global_prefs = '{$prefs_xml}' WHERE id = '{$boincuser->id}'");
  
  if (!$result) {
    watchdog('boincwork', 'Error updating global prefs for user @id: @message', array('@id' => $drupaluser->id, '@message' => mysql_error()), WATCHDOG_ERROR);
    drupal_set_message(t('Your changes could not be saved. Please contact support!'), 'error');
  }
  else {
    drupal_set_message(t('Your preferences have been updated.
      Client-related preferences will take effect when your computer 
      communicates with @project or you issue the "Update"
      command from the BOINC client.', array('@project' => PROJECT)));
  }
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Project preferences form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */ 

/**
 * The structure of the project preferences form
 */
function boincwork_projectprefs_form(&$form_state, $bilge, $venue, $initialize_if_empty = false) {
  require_boinc(array('user', 'prefs', 'util'));
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_project_prefs($venue);
  
  // Define form defaults
  $default = array(
    'resource' => array(
      'resource_share' => isset($prefs['resource_share']) ? $prefs['resource_share'] : 100,
      'use_cpu' => (isset($prefs['no_cpu']) AND $prefs['no_cpu'] == false) ? 1 : 0,
      'use_cuda' => (isset($prefs['no_cuda']) AND $prefs['no_cuda'] == false) ? 1 : 0,
      'use_ati' => (isset($prefs['no_ati']) AND $prefs['no_ati'] == false) ? 1 : 0
    ),
    'default_venue' => array(
      'default_venue' => isset($boincuser->venue) ? $boincuser->venue : '',
    )
  );
  
  // Get availability of special BOINC preferences
  global $app_types, $project_has_beta;
  
  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => t('yes'), 0 => t('no'))
  );
  
  // Hidden elements
  $form['venue'] = array(
    '#type' => 'hidden',
    '#value' => $venue
  );
  $form['modified'] = array(
    '#type' => 'hidden',
    '#value' => (isset($prefs['mod_time']) ? $prefs['mod_time'] : null)
  );
  
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li></ul>'
  );
  
  // Common project preferences  
  $form['resource'] = array(
    '#title' => t('Resource settings'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['resource']['resource_share'] = array(
    '#title' => t('Resource share'),
    '#type' => 'textfield',
    '#default_value' => $default['resource']['resource_share'],
    '#size' => 5,
    '#description' => t("Determines the proportion of your computer's resources allocated to this project. Example: if you participate in two BOINC projects with resource shares of 100 and 200, the first will get 1/3 of your resources and the second will get 2/3.")
  );
  if ($app_types->count > 1) {
    if ($app_types->cpu) {
      $form['resource']['use_cpu'] = array(
        '#title' => t('Use CPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#default_value' => $default['resource']['use_cpu'],
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
    if ($app_types->cuda) {
      $form['resource']['use_cuda'] = array(
        '#title' => t('Use NVIDIA GPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#default_value' => $default['resource']['use_cuda'],
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
    if ($app_types->ati) {
      $form['resource']['use_ati'] = array(
        '#title' => t('Use ATI GPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#default_value' => $default['resource']['use_ati'],
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
  }
  
  $form['default_venue'] = array(
    '#title' => t('Venue settings'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['venue_options'] = array(
    '#type' => 'value',
    '#value' => array('' => '---', 'home' => t('Home'), 'work' => t('Work'), 'school' => t('School'))
  );
  $form['default_venue']['default_venue'] = array(
    '#title' => t('Default computer location'),
    '#type' => 'select',
    '#options' => $form['venue_options']['#value'],
    '#default_value' => $default['default_venue']['default_venue'],
    '#description' => null
  );
  
  if ($project_has_beta) {
    $form['beta'] = array(
      '#title' => t('Beta settings'),
      '#type' => 'fieldset',
      '#description' => null,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );
    $form['beta']['allow_beta_work'] = array(
      '#title' => t('Run test applications?'),
      '#type' => 'radios',
      '#options' => $form['boolean_options']['#value'],
      '#default_value' => ($prefs['allow_beta_work']) ? 1 : 0,
      '#description' => t('This helps us develop applications, but may cause jobs to fail on your computer')
    );
  }
  
  return $form;
}

/**
  * Validate the project preferences form.
  */
function boincwork_projectprefs_form_validate($form, &$form_state) {
  require_boinc('util');
  
  // Verify all non-boolean user input values and notify form API of failures
  if (!verify_numeric($form_state['values']['resource_share'], 0)) form_set_error('resource_share', t('Invalid setting for') . " \"{$form['resource']['resource_share']['#title']}\"");
  if (!isset($form['venue_options']['#value'][$form_state['values']['default_venue']])) form_set_error('default_venue', t('Invalid setting for') . " \"{$form['venue']['default_venue']['#title']}\"");
}

/**
  * Handle post-validation submission of project preferences form.
  */
function boincwork_projectprefs_form_submit($form, &$form_state) {
  require_boinc(array('user', 'prefs'));
  
  global $site_name, $app_types, $app_array;
  
  $venue = $form_state['values']['venue'];
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_project_prefs($venue);
  
  // Resource preferences
  $prefs['resource_share'] = $form_state['values']['resource_share'];
  if ($app_types->count > 1) {
    if ($app_types->cpu) $prefs['no_cpu'] = ($form_state['values']['use_cpu']) ? false : true;
    if ($app_types->cuda) $prefs['no_cuda'] = ($form_state['values']['use_cuda']) ? false : true;
    if ($app_types->ati) $prefs['no_ati'] = ($form_state['values']['use_ati']) ? false : true;
  }
  
  // Beta preferences
  $prefs['allow_beta_work'] = ($form_state['values']['allow_beta_work']) ? true : false;
  
  // Save preferences back to the BOINC account
  boincwork_save_project_prefs($prefs, $venue);
  
  if (!$venue) {
    $boincuser->venue = $form_state['values']['default_venue'];
    venue_update($boincuser);
  }
  
  drupal_set_message(t('Your preferences have been updated.
      Client-related preferences will take effect when your computer 
      communicates with @project or you issue the "Update"
      command from the BOINC client.', array('@project' => $site_name)));
}

/**
 * The structure of the privacy preferences form
 */
function boincwork_privacyprefs_form(&$form_state, $bilge, $initialize_if_empty = false) {
  require_boinc(array('user', 'prefs', 'util'));
  
  global $user;
  $drupaluser = user_load($user->uid);
  $boincuser = BoincUser::lookup_id($drupaluser->boincuser_id);
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_project_prefs();
  
  if (!$prefs AND !$initialize_if_empty) return null;
  
  // Define form defaults
  $default = array(
    'privacy' => array(
      'send_email' => ($boincuser->send_email) ? 1 : 0,
      'show_hosts' => ($boincuser->show_hosts) ? 1 : 0
    )
  );
  
  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => t('yes'), 0 => t('no'))
  );
  
  $form['modified'] = array(
    '#type' => 'hidden',
    '#value' => (isset($prefs['mod_time']) ? $prefs['mod_time'] : null)
  );
  
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li></ul>'
  );
  
  $form['privacy'] = array(
    '#title' => t('Privacy settings'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['privacy']['send_email'] = array(
    '#title' => t('Is it OK for @project and your team (if any) to email you?', array('@project' => variable_get('site_name', 'BOINC'))),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['privacy']['send_email']
  );
  $form['privacy']['show_hosts'] = array(
    '#title' => t('Should @project show your computers on its web site?', array('@project' => variable_get('site_name', 'BOINC'))),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#default_value' => $default['privacy']['show_hosts']
  );
  
  return $form;
}

/**
  * Validate the privacy preferences form.
  */
function boincwork_privacyprefs_form_validate($form, &$form_state) {
  require_boinc('util');
  
  // Verify all non-boolean user input values and notify form API of failures
  // ... currently there are no non-boolean values!
}

/**
  * Handle post-validation submission of privacy preferences form.
  */
function boincwork_privacyprefs_form_submit($form, &$form_state) {
  require_boinc(array('user', 'prefs'));
  
  global $user;
  $drupaluser = user_load($user->uid);
  
  // Load BOINC account
  $boincuser = BoincUser::lookup_id($drupaluser->boincuser_id);
  
  // Privacy preferences
  $boincuser->send_email = ($form_state['values']['send_email']) ? true : false;
  $boincuser->show_hosts = ($form_state['values']['show_hosts']) ? true : false;
  
  //project_prefs_update($boincuser, $main_prefs);
  
  db_set_active('boinc');
  db_query("UPDATE user SET send_email = '{$boincuser->send_email}', show_hosts = '{$boincuser->show_hosts}' WHERE id = '{$boincuser->id}'");
  db_set_active('default');
  
  drupal_set_message(t('Your privacy preferences have been updated.'));
}
