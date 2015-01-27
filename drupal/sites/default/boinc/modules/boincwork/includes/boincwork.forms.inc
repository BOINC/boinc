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
function boincwork_generalprefs_form(&$form_state, $venue, $prefs_preset = null, $advanced = FALSE) {
  $form = array();
  $prefs = null;
  $established = TRUE;
  
  // Enable AHAH form support for dynamically updating content based on preset
  ahah_helper_register($form, $form_state);
  
  if (!$prefs_preset) {
    if (isset($form_state['storage']['prefs']['preset'])) {
      $prefs_preset = $form_state['storage']['prefs']['preset'];
    }
    
    // Load preferences from BOINC account
    $prefs = boincwork_load_prefs('general', $venue);
    
    // Take note if this is not an established preference set on the account
    if (isset($prefs['@attributes']['cleared'])) {
      $established = FALSE;
    }
    
    // Determine if a preset is selected or if these are custom settings
    if (!isset($prefs['@attributes']['preset'])) $prefs['@attributes']['preset'] = 'custom';
    if (!$prefs_preset) $prefs_preset = $prefs['@attributes']['preset'];
  }
  
  // Define form defaults
  switch($prefs_preset) {
  case 'standard':
  case 'maximum':
  case 'green':
  case 'minimum':
    $prefs = boincwork_get_preset_prefs($prefs_preset);
    break;
    
  case 'custom':
  default:
    // Just keeps prefs as they are
    $prefs_preset = 'custom';
  }
  
  require_boinc(array('db', 'prefs'));
  $disk_space_config = get_disk_space_config();
  
  $default = array(
    'preset' => $prefs_preset,
    // Processing...
    'run_on_batteries' => 0,
    'run_if_user_active' => 0,
    'run_gpu_if_user_active' => 1,
    'idle_time_to_run' => 3,
    'suspend_if_no_recent_input' => 0,
    'suspend_cpu_usage' => 0,
    'start_hour' => 0,
    'end_hour' => 0,
    'leave_apps_in_memory' => 0,
    'cpu_scheduling_period_minutes' => 60,
    'max_cpus' => 16,
    'max_ncpus_pct' => 100,
    'cpu_usage_limit' => 100,
    // Storage...
    'disk_max_used_gb' => $disk_space_config->disk_max_used_gb,
    'disk_min_free_gb' => $disk_space_config->disk_min_free_gb,
    'disk_max_used_pct' => $disk_space_config->disk_max_used_pct,
    'disk_interval' => 60,
    'vm_max_used_pct' => 75,
    'ram_max_used_busy_pct' => 50,
    'ram_max_used_idle_pct' => 90,
    // Network...
    'work_buf_min_days' => 0,
    'work_buf_additional_days' => 0.25,
    'confirm_before_connecting' => 0,
    'hangup_if_dialed' => 0,
    'max_bytes_sec_down' => 0,
    'max_bytes_sec_up' => 0,
    'net_start_hour' => 0,
    'net_end_hour' => 0,
    'daily_xfer_limit_mb' => 0,
    'daily_xfer_period_days' => 0,
    'dont_verify_images' => 0
  );
  foreach ($default as $name => $value) {
    if (isset($prefs[$name])) {
      if (is_array($prefs[$name])) {
        if (isset($prefs[$name]['@value'])) {
          $default[$name] = $prefs[$name]['@value'];
        }
      }
      else {
        $default[$name] = $prefs[$name];
      }
    }
  }
  
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
  
  // Identify preference sets that are established to distinguish what has been
  // saved to the database from what is just showing default values
  $form['#established'] = $established;
   
  // Set up the preference container for AHAH
  $form['prefs'] = array(
    '#title' => '',
    '#type' => 'fieldset',
    '#prefix' => '<div id="prefs-wrapper">', // This is our wrapper div.
    '#attributes' => array('class' => 'ahah-container'),
    '#suffix' => '</div>',
    '#tree'   => TRUE
  );
  //$form['prefs']['debug'] = array('#value' => '<pre>' . print_r($form_state, true) . '</pre>');
  
  // Hidden elements
  $form['prefs']['modified'] = array(
    '#type' => 'hidden',
    '#value' => (isset($prefs['mod_time']['@value']) ? $prefs['mod_time']['@value'] : null)
  );
  $form['prefs']['venue'] = array(
    '#type' => 'hidden',
    '#value' => $venue
  );
  
  $form['prefs']['separator_top'] = array(
    '#value' => '<div class="separator"></div>'
  );
  
  // Simplified selectors
  $form['prefs']['preset'] = array(
    '#title' => t('Presets'),
    '#type' => 'radios',
    '#description' => ' ',
    '#options' => array(
      'standard' => t('Standard'),
      'maximum' => t('Maximum'),
      'green' => t('Green'),
      'minimum' => t('Minimum'),
      'custom' => t('Custom')
    ),
    '#prefix' => '<div class="simple-form-controls">',
    '#suffix' => '</div>',
    '#default_value' => $default['preset'],
    '#ahah' => array(
      'event' => 'change',
      'path' => ahah_helper_path(array('prefs')),
      'wrapper' => 'prefs-wrapper'
    )
  );
  $form['prefs']['select preset'] = array(
    '#type'  => 'submit',
    '#value' => t('Update preset'),
    '#submit' => array('ahah_helper_generic_submit'),
    // The 'no-js' class only displays this button if javascript is disabled
    '#attributes' => array('class' => 'no-js'),
  );
  
  // Advanced preferences
  $form['prefs']['advanced'] = array(
    '#title' => t('Advanced settings'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => !$advanced,
    '#attributes' => array('class' => 'advanced-settings'),
  );
  
  // Processing preferences
  
  $form['prefs']['advanced']['anchor'] = array(
    '#value' => '<a name="advanced"></a>'
  );
  
  $form['prefs']['advanced']['separator_top'] = array(
    '#value' => '<div class="separator"></div>'
  );
  
  $form['prefs']['advanced']['processor'] = array(
    '#title' => t('Processor usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['processor']['run_on_batteries'] = array(
    '#title' => t('Run while computer is on battery power?'),
    '#type' => 'radios',
    '#description' => t('Only applies to portable computers'),
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['run_on_batteries']
  );
  $form['prefs']['advanced']['processor']['run_if_user_active'] = array(
    '#title' => t('Run while computer is in use?'),
    '#type' => 'radios',
    '#description' => ' ',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['run_if_user_active']
  );
  $form['prefs']['advanced']['processor']['run_gpu_if_user_active'] = array(
    '#title' => t('Run GPU work while computer is in use?'),
    '#type' => 'radios',
    '#description' => t('Enforced by version 6.6.21+'),
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['run_gpu_if_user_active']
  );
  $form['prefs']['advanced']['processor']['idle_time_to_run'] = array(
    '#title' => t('&apos;In use&apos; means mouse/keyboard activity in last'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['idle_time_to_run'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['prefs']['advanced']['processor']['suspend_if_no_recent_input'] = array(
    '#title' => t('Suspend work if no mouse/keyboard activity in last'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['suspend_if_no_recent_input'],
    '#size' => 1,
    '#description' => t('Needed to enter low-power mode on some computers')
  );
  $form['prefs']['advanced']['processor']['suspend_cpu_usage'] = array(
    '#title' => t('Suspend work if CPU usage is above'),
    '#type' => 'textfield',
    '#field_suffix' => t('%'),
    '#default_value' => $default['suspend_cpu_usage'],
    '#size' => 1,
    '#description' => t('0 means no restriction. Enforced by version 6.10.30+')
  );
  $form['prefs']['advanced']['processor']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . t('Do work only between the hours of:') . '</label></div>'
  );
  $form['prefs']['advanced']['processor']['start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['start_hour']
  );
  $form['prefs']['advanced']['processor']['hour_delimiter'] = array(
    '#value' => '<span>' . t('and') . '</span>'
  );
  $form['prefs']['advanced']['processor']['end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['end_hour']
  );
  $form['prefs']['advanced']['processor']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . t('No restriction if equal') . '</div></div>'
  );
  $form['prefs']['advanced']['processor']['leave_apps_in_memory'] = array(
    '#title' => t('Leave tasks in memory while suspended?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['leave_apps_in_memory'],
    '#description' => t('Suspended tasks will consume swap space if &apos;yes&apos;')
  );
  $form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes'] = array(
    '#title' => t('Switch between tasks every'),
    '#type' => 'textfield',
    '#field_suffix' => t('minutes'),
    '#default_value' => $default['cpu_scheduling_period_minutes'],
    '#size' => 1,
    '#description' => t('Recommended: 60 minutes')
  );
  $form['prefs']['advanced']['processor']['max_cpus'] = array(
    '#title' => t('On multiprocessors, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('processors'),
    '#default_value' => $default['max_cpus'],
    '#size' => 1,
    '#description' => t('Set to 0 for no limit'),
  );
  $form['prefs']['advanced']['processor']['max_ncpus_pct'] = array(
    '#title' => t('On multiprocessors, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of the processors'),
    '#default_value' => $default['max_ncpus_pct'],
    '#size' => 1,
    '#description' => t('Enforced by version 6.1+')
  );
  $form['prefs']['advanced']['processor']['cpu_usage_limit'] = array(
    '#title' => t('Use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of the CPU time'),
    '#default_value' => $default['cpu_usage_limit'],
    '#size' => 1,
    '#description' => t('Can be used to reduce CPU heat')
  );
  
  // Disk and memory preferences
  $form['prefs']['advanced']['storage'] = array(
    '#title' => t('Disk and memory usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['storage']['disk_max_used_gb'] = array(
    '#title' => t('Disk: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('GB'),
    '#default_value' => $default['disk_max_used_gb'],
    '#size' => 1,
    '#description' => t('Set to 0 for no limit'),
  ); 
  $form['prefs']['advanced']['storage']['disk_min_free_gb'] = array(
    '#title' => t('Disk: leave free at least'),
    '#type' => 'textfield',
    '#field_suffix' => t('GB'),
    '#default_value' => $default['disk_min_free_gb'],
    '#size' => 1,
    '#description' => t('Values smaller than 0.001 are ignored')
  );
  $form['prefs']['advanced']['storage']['disk_max_used_pct'] = array(
    '#title' => t('Disk: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['disk_max_used_pct'],
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['prefs']['advanced']['storage']['disk_interval'] = array(
    '#title' => t('Tasks checkpoint to disk at most every'),
    '#type' => 'textfield',
    '#field_suffix' => t('seconds'),
    '#default_value' => $default['disk_interval'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['prefs']['advanced']['storage']['vm_max_used_pct'] = array(
    '#title' => t('Swap space: use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['vm_max_used_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['prefs']['advanced']['storage']['ram_max_used_busy_pct'] = array(
    '#title' => t('Memory: when computer is in use, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['ram_max_used_busy_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  $form['prefs']['advanced']['storage']['ram_max_used_idle_pct'] = array(
    '#title' => t('Memory: when computer is not in use, use at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('% of total'),
    '#default_value' => $default['ram_max_used_idle_pct'],
    '#size' => 1,
    '#description' => ' '
  );
  
  // Network preferences
  $form['prefs']['advanced']['network'] = array(
    '#title' => t('Network usage'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['network']['work_buf_min_days'] = array(
    '#title' => t('Computer is connected to the Internet about every'),
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['work_buf_min_days'],
    '#size' => 1,
    '#description' => t('Leave blank or 0 if always connected. @project will try to maintain at least this much work.', array('@project' => PROJECT))
  ); 
  $form['prefs']['advanced']['network']['work_buf_additional_days'] = array(
    '#title' => t('Maintain enough work for an additional'),
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['work_buf_additional_days'],
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['prefs']['advanced']['network']['confirm_before_connecting'] = array(
    '#title' => t('Confirm before connecting to Internet?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['confirm_before_connecting'],
    '#description' => t('Matters only if you have a modem, ISDN, or VPN connection')
  ); 
  $form['prefs']['advanced']['network']['hangup_if_dialed'] = array(
    '#title' => t('Disconnect when done?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['hangup_if_dialed'],
    '#description' => t('Matters only if you have a modem, ISDN, or VPN connection')
  );
  $form['prefs']['advanced']['network']['max_bytes_sec_down'] = array(
    '#title' => t('Maximum download rate'),
    '#type' => 'textfield',
    '#field_suffix' => t('Kbytes/sec'),
    '#default_value' => $default['max_bytes_sec_down']/1000,
    '#size' => 1,
    '#description' => ' '
  ); 
  $form['prefs']['advanced']['network']['max_bytes_sec_up'] = array(
    '#title' => t('Maximum upload rate'),
    '#type' => 'textfield',
    '#field_suffix' => t('Kbytes/sec'),
    '#default_value' => $default['max_bytes_sec_up']/1000,
    '#size' => 1,
    '#description' => ' '
  );
  $form['prefs']['advanced']['network']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . t('Use network only between the hours of:') . '</label></div>'
  );
  $form['prefs']['advanced']['network']['net_start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['net_start_hour']
  );
  $form['prefs']['advanced']['network']['hour_delimiter'] = array(
    '#value' => '<span>' . t('and') . '</span>'
  );
  $form['prefs']['advanced']['network']['net_end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['net_end_hour']
  );
  $form['prefs']['advanced']['network']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . t('No restriction if equal') . '</div></div>'
  ); 
  $form['prefs']['advanced']['network']['daily_xfer_limit_mb'] = array(
    '#title' => t('Transfer at most'),
    '#type' => 'textfield',
    '#field_suffix' => t('Mbytes'),
    '#default_value' => $default['daily_xfer_limit_mb'],
    '#size' => 1
  ); 
  $form['prefs']['advanced']['network']['daily_xfer_period_days'] = array(
    '#field_prefix' => 'every',
    '#type' => 'textfield',
    '#field_suffix' => t('days'),
    '#default_value' => $default['daily_xfer_period_days'],
    '#size' => 1,
    '#description' => t('Enforced by version 6.10.46+')
  ); 
  $form['prefs']['advanced']['network']['dont_verify_images'] = array(
    '#title' => t('Skip image file verification?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['dont_verify_images'],
    '#description' => t('Check this ONLY if your Internet provider modifies image files (UMTS does this, for example). Skipping verification reduces the security of BOINC.')
  );
  
  // The "fancy radios" are made via javascript on document load. In order for
  // these to work with AHAH, we need this crazy setTimeout() call.
  $form['prefs']['fancy-radios'] = array(
    '#value' => '
      <script>
        setTimeout(
          function() {
            fancyRadiosInit();
            customPrefsListener();
          },
          300
        )
      </script>'
  );
  $form['prefs']['view advanced'] = array(
    '#type' => 'hidden',
    '#value' => 1
  );
  
  $form['prefs']['separator_bottom'] = array(
    '#value' => '<div class="separator buttons"></div>'
  );
  
  // Form control
  $form['prefs']['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['prefs']['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li>'
  );
  if ($venue AND $venue != 'generic') {
    global $base_path;
    $form['prefs']['form control tabs']['#value'] .= '<li class="tab">' . 
      l(t('Clear'), "account/prefs/computing/clear/{$venue}",
        array(
          'query' => 'destination=' . urlencode(drupal_get_path_alias('account/prefs/computing/combined')),
          'attributes' => array(
            'onclick' => 'return confirm(\'' . t('This will remove all of your settings from the @name preference set. Are you sure?',
              array('@name' => $venue)) . '\')'
          )
        )
      ) . '</li>';
  }
  $form['prefs']['view control'] = array(
    '#value' => '<li class="first alt tab">' . l('(' . t('Show comparison view') . ')', 'account/prefs/computing/combined') . '</li>'
  );
  $form['prefs']['form control tabs suffix'] = array(
    '#value' => '</ul>'
  );
  $form['#submit'][] = 'boincwork_generalprefs_form_submit';
  
  return $form;
}

/**
  * Validate the general preferences form.
  */
function boincwork_generalprefs_form_validate($form, &$form_state) {
  require_boinc('util');
  $values = $form_state['values']['prefs']['advanced'];
  
  //drupal_set_message('<pre>' . print_r($form_state['values'], true) . '</pre>');
  // Verify all non-boolean user input values and notify form API of failures
  
  // Processing preferences
  if (!verify_numeric($values['processor']['idle_time_to_run'], 1, 9999)) form_set_error('idle_time_to_run', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['idle_time_to_run']['#title']} [x] {$form['prefs']['advanced']['processor']['idle_time_to_run']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['suspend_if_no_recent_input'], 0, 9999)) form_set_error('suspend_if_no_recent_input', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['suspend_if_no_recent_input']['#title']} [x] {$form['prefs']['advanced']['processor']['suspend_if_no_recent_input']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['suspend_cpu_usage'], 0, 100)) form_set_error('suspend_cpu_usage', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['suspend_cpu_usage']['#title']} [x] {$form['prefs']['advanced']['processor']['suspend_cpu_usage']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['start_hour'], 0, 23)) form_set_error('start_hour', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['start_hour']['#title']} [x] {$form['prefs']['advanced']['processor']['start_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['end_hour'], 0, 23)) form_set_error('end_hour', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['end_hour']['#title']} [x] {$form['prefs']['advanced']['processor']['end_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['cpu_scheduling_period_minutes'], 1, 9999)) form_set_error('cpu_scheduling_period_minutes', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes']['#title']} [x] {$form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['max_cpus'], 0, 9999)) form_set_error('max_cpus', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['max_cpus']['#title']} [x] {$form['prefs']['advanced']['processor']['max_cpus']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['max_ncpus_pct'], 0, 100)) form_set_error('max_ncpus_pct', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['max_ncpus_pct']['#title']} [x] {$form['prefs']['advanced']['processor']['max_ncpus_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['processor']['cpu_usage_limit'], 0, 100)) form_set_error('cpu_usage_limit', t('Invalid setting for') . " \"{$form['prefs']['advanced']['processor']['cpu_usage_limit']['#title']} [x] {$form['prefs']['advanced']['processor']['cpu_usage_limit']['#field_suffix']}\"");

  // Storage preferences
  if (!verify_numeric($values['storage']['disk_max_used_gb'], 0, 9999999)) form_set_error('disk_max_used_gb', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['disk_max_used_gb']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_max_used_gb']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_min_free_gb'], 0.001, 9999999)) form_set_error('disk_min_free_gb', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['disk_min_free_gb']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_min_free_gb']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_max_used_pct'], 0, 100)) form_set_error('disk_max_used_pct', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['disk_max_used_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['disk_interval'], 0, 9999999)) form_set_error('disk_interval', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['disk_interval']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_interval']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['vm_max_used_pct'], 0, 100)) form_set_error('vm_max_used_pct', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['vm_max_used_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['vm_max_used_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['ram_max_used_busy_pct'], 0, 100)) form_set_error('ram_max_used_busy_pct', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['ram_max_used_busy_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['ram_max_used_busy_pct']['#field_suffix']}\"");
  if (!verify_numeric($values['storage']['ram_max_used_idle_pct'], 0, 100)) form_set_error('ram_max_used_idle_pct', t('Invalid setting for') . " \"{$form['prefs']['advanced']['storage']['ram_max_used_idle_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['ram_max_used_idle_pct']['#field_suffix']}\"");

  // Network preferences
  if (!verify_numeric($values['network']['work_buf_min_days'], 0, 10)) form_set_error('work_buf_min_days', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['work_buf_min_days']['#title']} [x] {$form['prefs']['advanced']['network']['work_buf_min_days']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['work_buf_additional_days'], 0, 10)) form_set_error('work_buf_additional_days', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['work_buf_additional_days']['#title']} [x] {$form['prefs']['advanced']['network']['work_buf_additional_days']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['max_bytes_sec_down'], 0, 9999.999)) form_set_error('max_bytes_sec_down', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['max_bytes_sec_down']['#title']} [x] {$form['prefs']['advanced']['network']['max_bytes_sec_down']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['max_bytes_sec_up'], 0, 9999.999)) form_set_error('max_bytes_sec_up', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['max_bytes_sec_up']['#title']} [x] {$form['prefs']['advanced']['network']['max_bytes_sec_up']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['net_start_hour'], 0, 23)) form_set_error('net_start_hour', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['net_start_hour']['#title']} [x] {$form['prefs']['advanced']['network']['net_start_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['net_end_hour'], 0, 23)) form_set_error('net_end_hour', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['net_end_hour']['#title']} [x] {$form['prefs']['advanced']['network']['net_end_hour']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['daily_xfer_limit_mb'], 0, 9999999)) form_set_error('daily_xfer_limit_mb', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#title']} [x] {$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
  if (!verify_numeric($values['network']['daily_xfer_period_days'], 0, 9999999)) form_set_error('daily_xfer_period_days', t('Invalid setting for') . " \"{$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#title']} [x] {$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#field_suffix']}\"");
}

/**
  * Handle post-validation submission of general preferences form.
  */
function boincwork_generalprefs_form_submit($form, &$form_state) {
  global $user;
  $account = user_load($user->uid);
  
  $values = $form_state['values']['prefs']['advanced'];
  $venue = $form_state['values']['prefs']['venue'];
  $preset = $form_state['values']['prefs']['preset'];
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_prefs('general', $venue);
  
  // Processing preferences
  $prefs['run_on_batteries'] = ($values['processor']['run_on_batteries']) ? 1 : 0;
  $prefs['run_if_user_active'] = ($values['processor']['run_if_user_active']) ? 1 : 0;
  $prefs['run_gpu_if_user_active'] = ($values['processor']['run_gpu_if_user_active']) ? 1: 0;
  $prefs['idle_time_to_run'] = $values['processor']['idle_time_to_run'];
  $prefs['suspend_if_no_recent_input'] = $values['processor']['suspend_if_no_recent_input'];
  $prefs['suspend_cpu_usage'] = $values['processor']['suspend_cpu_usage'];
  $prefs['start_hour'] = $values['processor']['start_hour'];
  $prefs['end_hour'] = $values['processor']['end_hour'];
  $prefs['leave_apps_in_memory'] = ($values['processor']['leave_apps_in_memory']) ? 1 : 0;
  $prefs['cpu_scheduling_period_minutes'] = $values['processor']['cpu_scheduling_period_minutes'];
  $prefs['max_cpus'] = $values['processor']['max_cpus'];
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
  
  // Save the preset selection (or lack thereof)
  if (!$preset OR $preset == 'custom') {
    if (isset($prefs['@attributes']['preset'])) {
      unset($prefs['@attributes']['preset']);
    }
  }
  else {
    $prefs['@attributes']['preset'] = $preset;
  }
  
  // If this is a new preference set, be sure to unset the "cleared" attribute
  if (isset($prefs['@attributes']['cleared'])) {
    unset($prefs['@attributes']['cleared']);
  }
  
  // Update database
  $result = boincwork_save_prefs($prefs, 'general', $venue);
  
  if (!$result) {
    watchdog('boincwork', 'Error updating global prefs for user @id: @message', array('@id' => $account->id, '@message' => mysql_error()), WATCHDOG_ERROR);
    drupal_set_message(t('Your changes could not be saved. Please contact support!'), 'error');
  }
  elseif (!drupal_get_messages('status', FALSE)) {
    // Show this message if the set wasn't created automatically (in which case
    // there is a message tailored to that) {
    drupal_set_message(t('Your preferences have been updated.
      Client-related preferences will take effect when your computer 
      communicates with @project or you issue the "Update"
      command from the BOINC client.', array('@project' => PROJECT)));
  }
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Merge host form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */ 

/**
 * Find compatible hosts for merging
 */
function boincwork_host_get_compatible_hosts($host_id) {
  require_boinc('host');
  global $user;
  $account = user_load($user->uid);
  $compatible_hosts = array();
  $host_count = 0;
  db_set_active('boinc');
  $current_host = db_fetch_object(db_query("
    SELECT id, domain_name, create_time, total_credit, rpc_time, os_name,
      p_vendor, p_model
    FROM {host}
    WHERE userid = '%d' AND id = '%d'",
    $account->boincuser_id, $host_id
  ));
  db_set_active('default');
  $current_host->task_count = boincwork_host_get_task_count($current_host->id);
  $current_host->is_new = !$current_host->total_credit AND !$current_host->task_count;
  // Get the list of all other hosts owned by this user for comparison
  db_set_active('boinc');
  $all_other_hosts = db_query("
    SELECT id, domain_name, create_time, total_credit, rpc_time, os_name,
      p_vendor, p_model
    FROM {host}
    WHERE userid = '%d' AND id <> '%d'",
    $account->boincuser_id, $host_id
  );
  db_set_active('default');
  // Compare all hosts to see if any are plausible duplicates
  while ($other_host = db_fetch_object($all_other_hosts)) {
    // First, disqualify if hosts were active at the same time
    if (!$current_host->is_new) {
      $other_host->task_count = boincwork_host_get_task_count($other_host->id);
      $other_host->is_new = !$other_host->total_credit AND !$other_host->task_count;
      if (!$other_host->is_new) {
        // If both hosts being compared are not new, see if times overlap
        if (!times_disjoint($current_host, $other_host)) {
          // Hosts were active at the same time; can't be a duplicate
          continue;
        }
      }
    }
    // Next, disqualify if hosts have different OS platforms
    if (!os_compatible($current_host, $other_host)) {
      // Hosts have different OS platforms; not really a duplicate
      continue;
    }
    // Finally, disqualify if hosts have different CPUs
    if (!cpus_compatible($current_host, $other_host)) {
      // CPUs don't match; not a duplicate
      continue;
    }
    // If not disqualified, this host is available for merging
    $hosts[] = $other_host;
    $host_count++;
    if ($host_count == 500) {
      // This is enough!
      break;
    }
  }
  return $hosts;
}

/**
 * Perform the database updates to merge the old host into the new host
 */
function boincwork_host_merge($old_host, $new_host, &$message = NULL) {
  // Decay the average credit of the two hosts
  require_boinc('credit');
  $now = time();
  update_average($now, 0, 0, $old_host->expavg_credit, $old_host->expavg_time);
  update_average($now, 0, 0, $new_host->expavg_credit, $new_host->expavg_time);
  
  // Update the database:
  // - add credit from old host to new host
  // - change results to refer to the new host
  // - put old host in "zombie" state (userid=0, rpc_seqno=[new_host_id])
  
  $total_credit = $old_host->total_credit + $new_host->total_credit;
  $recent_credit = $old_host->expavg_credit + $new_host->expavg_credit;
  
  // Move credit from the old host to the new host
  db_set_active('boinc');
  $credit_updated = db_query("
    UPDATE {host}
    SET
      total_credit = '%d',
      expavg_credit = '%d',
      expavg_time = '%d'
    WHERE id = '%d'",
    $total_credit, $recent_credit, $now, $new_host->id
  );
  db_set_active('default');
  if (!$credit_updated) {
    if ($message !== NULL) {
      $message = t('Could not update credit');
    }
    return FALSE;
  }
  
  // Move results from the old host to the new host
  db_set_active('boinc');
  $results_updated = db_query("
    UPDATE {result}
    SET hostid = '%d'
    WHERE hostid = '%d'",
    $new_host->id, $old_host->id
  );
  db_set_active('default');
  if (!$results_updated) {
    if ($message !== NULL) {
      $message = t('Could not update results');
    }
    return FALSE;
  }
  
  // Retire the old host
  db_set_active('boinc');
  $old_host_retired = db_query("
    UPDATE {host}
    SET
      total_credit = '0',
      expavg_credit = '0',
      userid = '0',
      rpc_seqno = '%d'
    WHERE id = '%d'",
    $new_host->id, $old_host->id
  );
  db_set_active('default');
  if (!$old_host_retired) {
    if ($message !== NULL) {
      $message = t('Could not retire old computer');
    }
    return FALSE;
  }
  
  return TRUE;
}

/**
 * Merge host form
 */
function boincwork_host_merge_form(&$form_state, $host_id) {

  if (!boincwork_host_user_is_owner($host_id)) {
    drupal_goto("host/{$host_id}");
  }
  
  $form = array();
  $form_state['storage']['current_host_id'] = $host_id;
  $current_host = boincwork_host_get_info($host_id);
  
  // Get hosts that could be merged with this one
  $hosts = boincwork_host_get_compatible_hosts($host_id);
  
  if (!$hosts) {
    drupal_set_message(t('There are no computers eligible for merging with this
      one'), 'warning'
    );
    drupal_goto("host/{$host_id}");
  }
  
  $form['overview'] = array(
    '#value' => '<p>' . t('Sometimes BOINC assigns separate identities to the
      same computer by mistake. You can correct this by merging old identities
      with the newest one.') . '</p>' .
      '<p>' . t('Check the computers that are the same as @name
      (created on @date at @time with computer ID @id)',
        array(
          '@name' => $current_host->domain_name,
          '@date' => date('j M Y', $current_host->create_time),
          '@time' => date('H:i:s T', $current_host->create_time),
          '@id' => $current_host->id,
        )
      ) . '</p>',
  );
  
  $options = array();
  foreach ($hosts as $host) {
    $options[$host->id] = array(
      $host->domain_name,
      date('j M Y H:i:s T', $host->create_time),
      $host->id,
    );
  }
  
  $form['merge'] = array(
    '#title' => '',
    '#type' => 'tableselect',
    '#header' => array(t('Name'), t('Created'), t('Computer ID')),
    '#options' => $options,
  );
  
  $form['prefs']['separator_bottom'] = array(
  //  '#value' => '<div class="separator buttons"></div>'
  );
  
  // Form control
  $form['prefs']['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['prefs']['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Merge'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), "host/{$host_id}") . '</li>'
  );
  
  return $form;
}

/**
 * Validate the merge host form
 */
function boincwork_host_merge_form_validate($form, &$form_state) {
}

/**
 * Handle submission of the merge host form
 */
function boincwork_host_merge_form_submit($form, &$form_state) {
  $merged = array();
  $errors = array();
  $current_host_id = $form_state['storage']['current_host_id'];
  $current_host = boincwork_host_get_info($current_host_id);
  $selected_hosts = array_filter($form_state['values']['merge']);
  
  foreach ($selected_hosts as $host_id) {
    // Attempt to merge each host, noting the results
    $message = '';
    $old_host = boincwork_host_get_info($host_id);
    if (boincwork_host_merge($old_host, $current_host, $message)) {
      $merged[$old_host->id] = $old_host->id;
      $current_host = boincwork_host_get_info($current_host_id);
    }
    else {
      $errors[$old_host->id] = $message;
    }
  }
  
  if ($merged) {
    // Generate a natural language list of IDs that were merged
    $oxford_comma = ',';
    $conjunction = t('and');
    $list = array_keys($merged);
    $last = array_pop($list);
    if ($list) {
      if (count($merged) == 2) {
        $oxford_comma = '';
      }
      $list = implode(', ', $list) . $oxford_comma . ' ' . $conjunction . ' ' . $last;
    }
    else {
      $list = $last;
    }
    drupal_set_message(format_plural(count($merged),
      'Computer @old_id has been merged successfully into @id.',
      'Computers @old_ids have been merged successfully into @id.',
      array(
        '@old_id' => $list,
        '@old_ids' => $list,
        '@id' => $current_host_id
      )
    ));
  }
  
  if ($errors) {
    // Report any hosts that failed to merge
    foreach ($errors as $id => $error) {
      drupal_set_message(
        t('Computer @old_id failed to merge: @message',
          array(
            '@old_id' => $id,
            '@message' => $error,
          )
        ),
        'warning'
      );
    }
  }
  
  drupal_goto("host/{$current_host_id}");
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Project preferences form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */ 

/**
 * The structure of the project preferences form
 */
function boincwork_projectprefs_form(&$form_state, $venue) {
  
  global $user;
  $account = user_load($user->uid);
  
  $established = TRUE;
  
  // Get availability of special BOINC preferences
  require_boinc(array('prefs'));
  global $app_array, $app_types;
  
  // Load any existing preferences from BOINC account
  $prefs = boincwork_load_prefs('project', $venue);
  
  // Take note if this is not an established preference set on the account
  if (isset($prefs['@attributes']['cleared'])) {
    $established = FALSE;
  }
  
  $venue_is_default = FALSE;
  if ($account->boincuser_default_pref_set) {
    if ($account->boincuser_default_pref_set == $venue) {
      $venue_is_default = TRUE;
    }
  }
  elseif (!$venue OR $venue == 'generic') {
    $venue_is_default = TRUE;
  }
  else {
    $venue_is_default = FALSE;
  }
  
  // Define form defaults
  $default = array(
    'resource_share' => 100,
    'no_cpu' => 0,
    'no_cuda' => 0,
    'no_ati' => 0,
    'no_intel_gpu' => 0,
    'default_venue' => $venue_is_default,
    'allow_beta_work' => $prefs['allow_beta_work'],
  );
  foreach ($default as $name => $value) {
    if (isset($prefs[$name])) {
      if (is_array($prefs[$name])) {
        if (isset($prefs[$name]['@value'])) {
          $default[$name] = $prefs[$name]['@value'];
        }
      }
      else {
        $default[$name] = $prefs[$name];
      }
    }
  }
  
  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => t('yes'), 0 => t('no'))
  );
  
  // Identify preference sets that are established to distinguish what has been
  // saved to the database from what is just showing default values
  $form['#established'] = $established;
  
  // Top level form options
  $form['#tree'] = TRUE;
  
  // Hidden elements
  $form['venue'] = array(
    '#type' => 'hidden',
    '#value' => $venue
  );
  
  $form['separator_top'] = array(
    '#value' => '<div class="separator"></div>'
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
    '#default_value' => $default['resource_share'],
    '#size' => 5,
    '#description' => t("Determines the proportion of your computer's resources allocated to this project. Example: if you participate in two BOINC projects with resource shares of 100 and 200, the first will get 1/3 of your resources and the second will get 2/3.")
  );
  if ($app_types->count > 1) {
    if ($app_types->cpu) {
      $form['resource']['no_cpu'] = array(
        '#title' => t('Use CPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_cpu'] ? 0 : 1,
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
    if ($app_types->cuda) {
      $form['resource']['no_cuda'] = array(
        '#title' => t('Use NVIDIA GPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_cuda'] ? 0 : 1,
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
    if ($app_types->ati) {
      $form['resource']['no_ati'] = array(
        '#title' => t('Use ATI GPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_ati'] ? 0 : 1,
        '#description' => t('Enforced by version @number', array('@number' => '6.10+'))
      );
    }
    if ($app_types->intel_gpu) {
      $form['resource']['no_intel_gpu'] = array(
        '#title' => t('Use INTEL GPU'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_intel_gpu'] ? 0 : 1,
        '#description' => t('Enforced by version @number', array('@number' => '7.0.27+'))
      );
    }
  }
  
  if (variable_get('boinc_prefs_options_beta', FALSE)) {
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
      '#attributes' => array('class' => 'fancy'),
      '#default_value' => ($default['allow_beta_work']) ? 1 : 0,
      '#description' => t('This helps us develop applications, but may cause jobs to fail on your computer')
    );
  }
  
  // Load project specific preferences from XML config
  $xml = boincwork_get_project_specific_config();
  
  // Respect the order of the top level elements
  $ordered_array = array();
  $unordered_array = array();
  foreach ($xml['project_specific_preferences'] as $type => $element) {
    if (is_array($element) AND is_numeric(key($element))) {
      foreach ($element as $ordered_element) {
        if (isset($ordered_element['@position'])) {
          $ordered_array[$ordered_element['@position']] = array($type => $ordered_element);
        }
        else {
          $unordered_array[] = array($type => $ordered_element);
        }
      }
    }
    elseif (isset($element['@position'])) {
      $ordered_array[$element['@position']] = array($type => $element);
    }
    else {
      $unordered_array[] = array($type => $element);
    }
  }
  ksort($ordered_array);
  $primed_array = array_merge($ordered_array, $unordered_array);
  $xml = array('project_specific_preferences' => $primed_array);
    
  foreach ($xml['project_specific_preferences'] as $wrapped_element) {
    $type = key($wrapped_element);
    $element= reset($wrapped_element);
    boincwork_generate_prefs_element($form, $type, $element, $prefs['project_specific']);
  }
  
  // Set whether to use this preference set by default for new computers
  $form['default_set'] = array(
    '#title' => t('Default set'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['default_set']['default_venue'] = array(
    '#title' => t('Set used for new computers'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['default_venue'] ? 1 : 0,
    '#description' => ''
  );
  
  $form['prefs']['separator_bottom'] = array(
    '#value' => '<div class="separator buttons"></div>'
  );
  
  // Form control
  $form['prefs']['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['prefs']['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li>'
  );
  if ($venue AND $venue != 'generic') {
    global $base_path;
    $form['prefs']['form control tabs']['#value'] .= '<li class="tab">' . 
      l(t('Clear'), "account/prefs/project/clear/{$venue}",
        array(
          'query' => 'destination=' . urlencode(drupal_get_path_alias('account/prefs/project/combined')),
          'attributes' => array(
            'onclick' => 'return confirm(\'' . t('This will remove all of your settings from the @name preference set. Are you sure?',
              array('@name' => $venue)) . '\')'
          )
        )
      ) . '</li>';
  }
  $form['prefs']['view control'] = array(
    '#value' => '<li class="first alt tab">' . l('(' . t('Show comparison view') . ')', 'account/prefs/project/combined') . '</li>'
  );
  $form['prefs']['form control tabs suffix'] = array(
    '#value' => '</ul>'
  );
  
  return $form;
}

/**
  * Validate the project preferences form.
  */
function boincwork_projectprefs_form_validate($form, &$form_state) {
  
  // Verify all text user input values and notify form API of failures
  $validation_rules = array(
    'resource' => array(
      'resource_share' => array(
        'datatype' => 'integer',
        'min' => 0
      ),
    ),
  );
  
  // Add validation rules for project specific settings
  $validation_rules += boincwork_get_project_specific_config_validation_rules();
  
  // Perform validation
  boincwork_validate_form($validation_rules, $form_state['values']);
  
  // Check for app validation
  if (isset($validation_rules['apps'])) {
    if (isset($validation_rules['apps']['minimum selected'])
        AND $validation_rules['apps']['minimum selected'] > 0) {
      $apps_selected = 0;
      foreach ($validation_rules['apps']['list'] as $app) {
        if ($form_state['values']['applications'][$app]) $apps_selected++;
      }
      if ($apps_selected < $validation_rules['apps']['minimum selected']) {
        form_set_error(
          'applications',
          t('At least one application must be selected')
        );
      }
      if ($apps_selected == count($validation_rules['apps']['list'])) {
        foreach ($validation_rules['apps']['list'] as $app) {
          unset($form_state['values']['applications'][$app]);
        }
        $form_state['storage']['all apps selected'] = TRUE;
      }
    }
  }
}

/**
  * Handle post-validation submission of project preferences form.
  */
function boincwork_projectprefs_form_submit($form, &$form_state) {
  require_boinc(array('prefs'));
  
  global $user;
  global $site_name, $app_types, $app_array;
  
  $account = user_load($user->uid);
  $edit = $form_state['values'];
  $venue = $edit['venue'];
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_prefs('project', $venue);
  
  // Resource preferences
  $prefs['resource_share'] = $edit['resource']['resource_share'];
  if ($app_types->count > 1) {
    if ($app_types->cpu) $prefs['no_cpu'] = ($edit['resource']['no_cpu']) ? 0 : 1;
    if ($app_types->cuda) $prefs['no_cuda'] = ($edit['resource']['no_cuda']) ? 0 : 1;
    if ($app_types->ati) $prefs['no_ati'] = ($edit['resource']['no_ati']) ? 0 : 1;
    if ($app_types->intel_gpu) $prefs['no_intel_gpu'] = ($edit['resource']['no_intel_gpu']) ? 0 : 1;
  }
  
  // Beta preferences
  if (variable_get('boinc_prefs_options_beta', FALSE)) {
    $prefs['allow_beta_work'] = ($edit['beta']['allow_beta_work']) ? 1 : 0;
  }
  
  // Load project specific preferences from XML config
  $xml = boincwork_get_project_specific_config();
  $updated_prefs = array(
    'project_specific' => boincwork_format_project_specific_prefs_data($edit)
  );
  $prefs = $updated_prefs + $prefs;
  
  // Don't specify apps if all are selected
  if (isset($form_state['storage']['all apps selected'])) {
    unset($prefs['project_specific']['app_id']);
    unset($form_state['storage']['all apps selected']);
  }
  
  // If this is a new preference set, be sure to unset the "cleared" attribute
  if (isset($prefs['@attributes']['cleared'])) {
    unset($prefs['@attributes']['cleared']);
  }
  
  // Save preferences back to the BOINC account
  $result = boincwork_save_prefs($prefs, 'project', $venue);
  
  // Update the user's default preference set
  if ($edit['default_set']['default_venue']) {
    boincwork_set_default_venue($venue);
  }
  elseif ($venue == $account->boincuser_default_pref_set) {
    // User has cleared out the default venue setting
    boincwork_set_default_venue();
  }
  
  if (!$result) {
    watchdog('boincwork', 'Error updating project prefs for user @id: @message', array('@id' => $user->id, '@message' => mysql_error()), WATCHDOG_ERROR);
    drupal_set_message(t('Your changes could not be saved. Please contact support!'), 'error');
  }
  elseif (!drupal_get_messages('status', FALSE)) {
    // Show this message if the set wasn't created automatically (in which case
    // there is a message tailored to that)
    drupal_set_message(t('Your preferences have been updated.
        Client-related preferences will take effect when your computer 
        communicates with @project or you issue the "Update"
        command from the BOINC client.', 
        array('@project' => $site_name)));
  }
}

/**
 * The structure of the community preferences form
 */
function communityprefs_form(&$form_state) {
  global $user;
  $account = user_load($user->uid);
  $form = array();
  
  // Pull in some elements from the profile form
  $profile_form_state = array();
  $profile = new stdClass();
  $profile->type = 'profile';
  $profile->language = NULL;
  if ($profile_nid = content_profile_profile_exists($profile, $account->uid)) {
    $profile_node = node_load($profile_nid);
    $form_state['storage']['profile_node'] = $profile_node;
    module_load_include('inc', 'node', 'node.pages');    
    $profile_form = drupal_retrieve_form('profile_node_form', $profile_form_state, $profile_node);
    drupal_prepare_form('profile_node_form', $profile_form, $profile_form_state);
  }
  
  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => t('yes'), 0 => t('no'))
  );
  
  $default = array(
    'pm_send_notification' => '', // This is set already in pm_email_notify_user
    'friend_notification' => isset($account->friend_notification) ? $account->friend_notification : 0,
    'comments_per_page' => (isset($account->comments_per_page) AND $account->comments_per_page) ? $account->comments_per_page : variable_get('comment_default_per_page_forum', 50),
    'comments_order' => (isset($account->sort) AND $account->sort) ? $account->sort : variable_get('comment_default_order_forum', COMMENT_ORDER_OLDEST_FIRST),
  );
  
  // General options
  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#weight' => 0,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  // Add the BOINC user name (non-unique, user editable)
  $form['general']['boincuser_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $account->boincuser_name,
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#required' => TRUE,
    '#description' => t(''),
    '#size' => 40
  );
  // Time zone
  if (variable_get('configurable_timezones', 1)) {
    $zones = _system_zonelist();
    $form['general']['timezone'] = array(
      '#type' => 'select',
      '#title' => t('Time zone'),
      '#default_value' => ($account->timezone !== NULL) ? $account->timezone : variable_get('date_default_timezone', 0),
      '#options' => $zones,
      '#description' => '',
    );
  }
  
  // Notification options
  $form['notifications'] = array(
    '#type' => 'fieldset',
    '#title' => t('Notification settings'),
    '#weight' => 5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  // Pull in private message notification handling and tweak the form
  $pm_notify = pm_email_notify_user('form', $edit, $account, 'account');
  $form['notifications']['pm_send_notifications'] = array_replace(
    $pm_notify['enable_pm_mail']['pm_send_notifications'],
    array(
      '#type' => 'radios',
      '#title' => t('Receive email notification for private messages?'),
      '#description' => ' ',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy')
    )
  );
  $form['notifications']['friend_notification'] = array(
    '#type' => 'radios',
    '#title' => t('Receive email notification for friend requests?'),
    '#description' => ' ',
    '#options' => array(0 => t('yes'), -1 => t('no')),
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['friend_notification']
  );
  
  // Internationalization options
  if (module_exists('internationalization')) {
    $languages = language_list('enabled');
    $languages = $languages[1];
    $names = array();
    foreach ($languages as $langcode => $item) {
      $name = t($item->name);
      $names[check_plain($langcode)] = check_plain($name . ($item->native != $name ? ' ('. $item->native .')' : ''));
    }
    $form['locale'] = array(
      '#type' => 'fieldset',
      '#title' => t('Language settings'),
      '#weight' => 10,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    // Get language negotiation settings.
    $mode = variable_get('language_negotiation', LANGUAGE_NEGOTIATION_NONE);
    $user_preferred_language = user_preferred_language($account);
    $form['locale']['language'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => check_plain($user_preferred_language->language),
      '#options' => $names,
      '#description' => ($mode == LANGUAGE_NEGOTIATION_PATH) ? t("This account's default language for e-mails and preferred language for site presentation.") : t("This account's default language for e-mails."),
    );
  }
  
  // Avatar options
  $form['gravatar'] = array(
    '#type' => 'item',
    '#value' => t('If you have a <a href="@gravatar-check">valid gravatar</a> associated with your e-mail address, it will be used for your user picture.', array('@gravatar-check' => 'http://en.gravatar.com/site/check/' . $account->mail)),
    '#description' => t('Your Gravatar will not be shown if you upload a user picture.'),
  );
  if (user_access('disable own gravatar', $account)) {
    $form['gravatar'] = array(
      '#type' => 'checkbox',
      '#title' => t('If you have a <a href="@gravatar-check">valid Gravatar</a> associated with your e-mail address, use it for your user picture.', array('@gravatar-check' => 'http://en.gravatar.com/site/check/' . $account->mail)),
      '#description' => t('Gravatar will not be shown if an avatar is uploaded.'),
      '#default_value' => isset($account->gravatar) ? $account->gravatar : 0,
      '#disabled' => !empty($account->picture),
    );
  }
  $form['gravatar']['#weight'] = 15;
  $form['gravatar']['#prefix'] = '<fieldset class="collapsible"><legend><a href="#">' . t('Avatar settings') . '</a></legend>';
  // Upload an avatar (pulled from profile_node_form):
  $form['field_image'] = array(
    '#value' => '<div class="form-item">'
      . '<label class="placeholder">'
      . t('This is not available until your profile is set up.')
      . '</label>'
      . l(t('Create a profile'), 'account/profile/edit', array('attributes' => array('class' => 'form-link')))
      . '</div>',
  );
  if ($profile_form_state) {
    $form['field_image'] = $profile_form_state['storage']['avatar']; //$profile_form['field_image'];
  }
  $form['field_image'][0]['#title'] = t('Upload an avatar');
  $form['field_image']['#weight'] = 20;
  $form['field_image']['#suffix'] = '</fieldset>';
  
  // Forum options
  $form['forums'] = array(
    '#type' => 'fieldset',
    '#title' => t('Forum settings'),
    '#weight' => 25,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['forums']['comments_per_page'] = array(
    '#type' => 'select',
    '#title' => t('In discussion topics, show at most @comments_per_page', array('@comments_per_page' => '')),
    '#options' => array(10 => 10, 20 => 20, 30 => 30, 50 => 50, 100 => 100),
    '#default_value' => $default['comments_per_page']
  );
  // Can't have a typical Drupal form suffix on a select box?
  $form['forums']['comments_per_page_suffix'] = array(
    '#value' => '<span>' . t('comments per page') . '</span>'
  );
  $form['forums']['comments_order'] = array(
    '#type' => 'select',
    '#title' => t('Sort comments in discussions'),
    '#options' => array(1 => t('Newest post first'), 2 => t('Oldest post first')),
    '#default_value' => $default['comments_order']
  );
  // Signature (pulled from user_edit_form):
  if (variable_get('user_signatures', 0) && module_exists('comment')) {
    $form['forums']['signature'] = array(
      '#type' => 'textarea',
      '#title' => t('Signature'),
      '#description' => t('Your signature will be publicly displayed at the end of your comments.'),
      '#default_value' => $account->signature
      );
    // Prevent a "validation error" message when the user attempts to save with a default value they
    // do not have access to.
    if (!filter_access($account->signature_format) && empty($_POST)) {
      drupal_set_message(t("The signature input format has been set to a format you don't have access to. It will be changed to a format you have access to when you save this page."));
      $edit['signature_format'] = FILTER_FORMAT_DEFAULT;
    }
    $form['forums']['signature_format'] = filter_form($account->signature_format, NULL, array('signature_format'));
    // Optionally hide signatures from comments
    $form['forums']['hide_signatures'] = array(
      '#type' => 'radios',
      '#title' => t('Hide signatures in forums'),
      '#description' => ' ',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy'),
      '#default_value' => isset($account->hide_signatures) ? $account->hide_signatures : 0,
    );
  }
  
  $form['separator_bottom'] = array(
    '#value' => '<div class="separator buttons"></div>',
    '#weight' => 999,
  );
  
  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">',
    '#weight' => 1001,
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>',
    '#weight' => 1002,
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li>',
    '#weight' => 1003,
  );
  $form['form control tabs suffix'] = array(
    '#value' => '</ul>',
    '#weight' => 1004,
  );
  
  return $form;
}

/**
  * Handle post-validation submission of community preferences form.
  */
function communityprefs_form_submit($form, &$form_state) {
  require_boinc('boinc_db');
  global $user;
  $account = user_load($user->uid);
  $boinc_user = BoincUser::lookup_id($account->boincuser_id);
  $edit= $form_state['values'];
  $profile_node = $form_state['storage']['profile_node'];
  
  // Display name
  if ($edit['boincuser_name'] != $boinc_user->name) {
    $boincuser_name = $edit['boincuser_name'];
    $result = $boinc_user->update(
        "name='{$boincuser_name}'"
    );
  }
  
  // Private message settings
  pm_email_notify_user('submit', $edit, $user);
  
  // Avatar settings
  if (!$edit['field_image']) $edit['field_image'] = array();
  $profile_node->field_image = $edit['field_image'];
  node_save($profile_node);
  // Flush this from the node cache or changes won't show up immediately!
  $profile_node = node_load($profile_node->nid, NULL, TRUE);
  
  // All other settings
  $settings = array(
    'signature' => $edit['signature'],
    'signature_format' => $edit['signature_format'],
    'timezone' => $edit['timezone'],
    'friend_notification' => $edit['friend_notification'],
    'comments_per_page' => $edit['comments_per_page'],
    'hide_signatures' => $edit['hide_signatures'],
    'sort' => $edit['comments_order'],
    'gravatar' => $edit['gravatar'],
  );
  if (module_exists('internationalization')) {
    $settings['language'] = $edit['language'];
    global $language;
    if ($user->language != $edit['language']) {
      global $base_url;
      if ($edit['language'] != language_default('language')) {
        $form_state['redirect'] = $base_url . '/' . $edit['language'] . '/' . $_GET['q'];
      }
      else {
        $form_state['redirect'] = $base_url . '/' . $_GET['q'];
      }
    }
  }
  user_save($user, $settings);
  
  drupal_set_message(t('Your community preferences have been updated.'));
}

/**
 * The structure of the privacy preferences form
 */
function boincwork_privacyprefs_form(&$form_state) {
  require_boinc(array('user', 'prefs', 'util'));
  
  global $user;
  $account = user_load($user->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  
  // Load preferences from BOINC account
  $prefs = boincwork_load_prefs('project');
  
  //if (!$prefs AND !$initialize_if_empty) return null;
  
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
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['privacy']['send_email']
  );
  $form['privacy']['show_hosts'] = array(
    '#title' => t('Should @project show your computers on its web site?', array('@project' => variable_get('site_name', 'BOINC'))),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['privacy']['show_hosts']
  );
  
  $form['prefs']['separator_bottom'] = array(
    '#value' => '<div class="separator buttons"></div>'
  );
  
  // Form control
  $form['prefs']['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['prefs']['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li>'
  );
  $form['prefs']['form control tabs suffix'] = array(
    '#value' => '</ul>'
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
  $account = user_load($user->uid);
  
  // Load BOINC account
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  
  // Privacy preferences
  $boincuser->send_email = ($form_state['values']['send_email']) ? true : false;
  $boincuser->show_hosts = ($form_state['values']['show_hosts']) ? true : false;
  
  //project_prefs_update($boincuser, $main_prefs);
  
  db_set_active('boinc');
  db_query("UPDATE user SET send_email = '{$boincuser->send_email}', show_hosts = '{$boincuser->show_hosts}' WHERE id = '{$boincuser->id}'");
  db_set_active('default');
  
  drupal_set_message(t('Your privacy preferences have been updated.'));
}
