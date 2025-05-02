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
    // transform old way to store the preset into new way
    if (isset($prefs['@attributes']['preset'])) {
        $prefs['preset'] = $prefs['@attributes']['preset'];
        unset($prefs['@attributes']['preset']);
    }
    // Set $prefs_preset if preset tag is present in database.
    if (!$prefs_preset) {
        if (isset($prefs['preset'])) {
            $prefs_preset = $prefs['preset']['@value'];
        }
    }// if !$prefs_preset
  }
  // Extract mod_time tag if present, because it will be erased with
  // boincwork_get_preset_prefs() below.
  $mod_time = null;
  if (isset($prefs['mod_time']['@value'])) {
      $mod_time = $prefs['mod_time']['@value'];
  }

  if (isset($form_state['storage']['wip'])) {
      switch ($prefs_preset) {
      case 'standard':
      case 'maximum':
      case 'green':
      case 'minimum':
          $prefs = boincwork_get_preset_prefs($prefs_preset);
          break;
      case 'custom':
      default:
          // Just keeps prefs as they are
          unset($prefs['preset']);
          break;
      }// switch
  } else {
      $form_state['storage']['wip'] = TRUE;
      if ( !in_array($prefs_preset, array('standard','maximum','green','minimum','custom')) ) {
          if ($established) {
              $prefs_preset = 'custom';
          } else {
              $prefs_preset = 'standard';
              $prefs = boincwork_get_preset_prefs($prefs_preset);
          }// if $established
      }// if $prefs_preset
  }// if WIP

  // This set of preferences is used in the form if no preferences
  // have been set above, in variable $prefs.
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
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes'))
  );
  $form['hour_options'] = array(
    '#type' => 'value',
    '#value' => array('0:00','1:00','2:00','3:00','4:00',
     '5:00','6:00','7:00','8:00','9:00','10:00','11:00',
     '12:00','13:00','14:00','15:00','16:00','17:00',
     '18:00','19:00','20:00','21:00','22:00','23:00')
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
    '#value' => $mod_time
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
      '#title' => bts('Presets', array(), NULL, 'boinc:account-preferences-preset:-1:for a user to choose a computing or project preference preset.'),
    '#type' => 'radios',
    '#description' => ' ',
    '#options' => array(
      'standard' => bts('Standard', array(), NULL, 'boinc:account-preferences-preset'),
      'maximum' => bts('Maximum', array(), NULL, 'boinc:account-preferences-preset'),
      'green' => bts('Green', array(), NULL, 'boinc:account-preferences-preset'),
      'minimum' => bts('Minimum', array(), NULL, 'boinc:account-preferences-preset'),
      'custom' => bts('Custom', array(), NULL, 'boinc:account-preferences-preset')
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
    '#value' => bts('Update preset', array(), NULL, 'boinc:account-preferences-preset'),
    '#submit' => array('ahah_helper_generic_submit'),
    // The 'no-js' class only displays this button if javascript is disabled
    '#attributes' => array('class' => 'no-js'),
  );

  // Advanced preferences
  $form['prefs']['advanced'] = array(
    '#title' => bts('Advanced settings', array(), NULL, 'boinc:account-preferences-option'),
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
    '#title' => bts('Processor usage', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['processor']['run_on_batteries'] = array(
    '#title' => bts('Suspend when computer is on battery?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#description' => bts('Suspends computing on portables when running on battery power.', array(), NULL, 'boinc:account-preferences-computing'),
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => ($default['run_on_batteries']) ? 0 : 1 // intentional inversion of setting
  );
  $form['prefs']['advanced']['processor']['run_if_user_active'] = array(
    '#title' => bts('Suspend when computer is in use?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#description' => bts("Suspends computing and file transfers when you're using the computer.", array(), NULL, 'boinc:account-preferences-computing'),
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => ($default['run_if_user_active']) ? 0 : 1 // intentional inversion of setting
  );
  $form['prefs']['advanced']['processor']['run_gpu_if_user_active'] = array(
    '#title' => bts('Suspend GPU computing when computer is in use?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#description' => bts("Suspends GPU computing when you're using the computer.", array(), NULL, 'boinc:account-preferences-computing'),
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => ($default['run_gpu_if_user_active']) ? 0 : 1 // intentional inversion of setting
  );
  $form['prefs']['advanced']['processor']['idle_time_to_run'] = array(
      '#title' => bts('"In use" means mouse/keyboard input in last', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('minutes', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['idle_time_to_run'],
    '#size' => 1,
    '#description' => bts('This determines when the computer is considered "in use".', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['processor']['suspend_if_no_recent_input'] = array(
    '#title' => bts('Suspend when no mouse/keyboard input in last', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('minutes', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['suspend_if_no_recent_input'],
    '#size' => 1,
    '#description' => bts('This allows some computers to enter low-power mode when not in use.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['processor']['suspend_cpu_usage'] = array(
    '#title' => bts('Suspend when non-BOINC CPU usage is above', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => '%',
    '#default_value' => $default['suspend_cpu_usage'],
    '#size' => 1,
    '#description' => bts('Suspend computing when your computer is busy running other programs.', array(), NULL, 'boinc:account-preferences-computing'),
  );
  $form['prefs']['advanced']['processor']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . bts('Compute only between:', array(), NULL, 'boinc:account-preferences-computing') . '</label></div>'
  );
  $form['prefs']['advanced']['processor']['start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['start_hour']
  );
  $form['prefs']['advanced']['processor']['hour_delimiter'] = array(
    '#value' => '<span>' . bts('and', array(), NULL, 'boinc:account-preference') . '</span>'
  );
  $form['prefs']['advanced']['processor']['end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['end_hour']
  );
  $form['prefs']['advanced']['processor']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . bts('Compute only during a particular period each day.', array(), NULL, 'boinc:account-preferences-computing') . '</div></div>'
  );
  $form['prefs']['advanced']['processor']['leave_apps_in_memory'] = array(
    '#title' => bts('Leave non-GPU tasks in memory while suspended?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['leave_apps_in_memory'],
    '#description' => bts('If "Yes", suspended tasks stay in memory, and resume with no work lost. If "No", suspended tasks are removed from memory, and resume from their last checkpoint.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes'] = array(
    '#title' => bts('Switch between tasks every', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('minutes', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['cpu_scheduling_period_minutes'],
    '#size' => 1,
    '#description' => bts('If you run several projects, BOINC may switch between them this often.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['processor']['max_ncpus_pct'] = array(
    '#title' => bts('Use at most', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of the processors', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['max_ncpus_pct'],
    '#size' => 1,
    '#description' => bts('Keep some CPUs free for other applications. Example: 75% means use 6 cores on an 8-core CPU.', array(), NULL, 'boinc:account-preferences-computing'),
  );
  $form['prefs']['advanced']['processor']['cpu_usage_limit'] = array(
    '#title' => bts('Use at most', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of the CPU time', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['cpu_usage_limit'],
    '#size' => 1,
    '#description' => bts('Suspend/resume computing every few seconds to reduce CPU temperature and energy usage. Example: 75% means compute for 3 seconds, wait for 1 second, and repeat.', array(), NULL, 'boinc:account-preferences-computing')
  );

  // Disk and memory preferences
  $form['prefs']['advanced']['storage'] = array(
    '#title' => bts('Disk and memory usage', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['storage']['disk_max_used_gb'] = array(
    '#title' => bts('Disk: use no more than', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => 'GB',
    '#default_value' => $default['disk_max_used_gb'],
    '#size' => 1,
    '#description' => bts('Limit the total amount of disk space used by BOINC.', array(), NULL, 'boinc:account-preferences-computing'),
  );
  $form['prefs']['advanced']['storage']['disk_min_free_gb'] = array(
    '#title' => bts('Disk: leave at least', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => 'GB free',
    '#default_value' => $default['disk_min_free_gb'],
    '#size' => 1,
    '#description' => bts('Limit disk usage to leave this much free space on the volume where BOINC stores data.', array(), NULL, 'boinc:account-preferences-computing'),
  );
  $form['prefs']['advanced']['storage']['disk_max_used_pct'] = array(
    '#title' => bts('Disk: use no more than', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of total', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['disk_max_used_pct'],
    '#size' => 1,
    '#description' => bts('Limit the percentage of disk space used by BOINC on the volume where it stores data.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['storage']['disk_interval'] = array(
    '#title' => bts('Request tasks to checkpoint at most every', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('seconds', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['disk_interval'],
    '#size' => 1,
    '#description' => bts('This controls how often tasks save their state to disk, so that later they can be continued from that point.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['storage']['vm_max_used_pct'] = array(
    '#title' => bts('Page/swap file: use at most', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of total', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['vm_max_used_pct'],
    '#size' => 1,
    '#description' => bts('Limit the swap space (page file) used by BOINC.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['storage']['ram_max_used_busy_pct'] = array(
    '#title' => bts('Memory: when computer is in use, use at most', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of total', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['ram_max_used_busy_pct'],
    '#size' => 1,
    '#description' => bts("Limit the memory used by BOINC when you're using the computer.", array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['storage']['ram_max_used_idle_pct'] = array(
    '#title' => bts('Memory: when computer is not in use, use at most', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('% of total', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['ram_max_used_idle_pct'],
    '#size' => 1,
    '#description' => bts("Limit the memory used by BOINC when you're not using the computer.", array(), NULL, 'boinc:account-preferences-computing')
  );

  // Network preferences
  $form['prefs']['advanced']['network'] = array(
    '#title' => bts('Network usage', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE
  );
  $form['prefs']['advanced']['network']['work_buf_min_days'] = array(
    '#title' => bts('Store at least', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('days of work', array(), NULL, 'boinc:account-preferences-computing'),
    '#default_value' => $default['work_buf_min_days'],
    '#size' => 1,
    '#description' => bts('Store at least enough tasks to keep the computer busy for this long.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['work_buf_additional_days'] = array(
    '#title' => bts('Store up to an additional', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => bts('days', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['work_buf_additional_days'],
    '#size' => 1,
    '#description' => bts('Store additional tasks above the minimum level.  Determines how much work is requested when contacting a project.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['confirm_before_connecting'] = array(
    '#title' => bts('Confirm before connecting to Internet?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['confirm_before_connecting'],
    '#description' => bts('Useful only if you have a modem, ISDN or VPN connection.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['hangup_if_dialed'] = array(
    '#title' => bts('Disconnect when done?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['hangup_if_dialed'],
    '#description' => bts('Useful only if you have a modem, ISDN or VPN connection.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['max_bytes_sec_down'] = array(
    '#title' => bts('Limit download rate to', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => 'Kbytes/sec',
    '#default_value' => $default['max_bytes_sec_down']/1000,
    '#size' => 1,
    '#description' => bts('Limit the download rate of file transfers.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['max_bytes_sec_up'] = array(
    '#title' => bts('Limit upload rate to', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => 'Kbytes/sec',
    '#default_value' => $default['max_bytes_sec_up']/1000,
    '#size' => 1,
    '#description' => bts('Limit the upload rate of file transfers.', array(), NULL, 'boinc:account-preferences-computing')
  );
  $form['prefs']['advanced']['network']['hour_label'] = array(
    '#value' => '<div class="form-item"><label>' . bts('Transfer files only between', array(), NULL, 'boinc:account-preferences-computing') . '</label></div>'
  );
  $form['prefs']['advanced']['network']['net_start_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['net_start_hour']
  );
  $form['prefs']['advanced']['network']['hour_delimiter'] = array(
    '#value' => '<span>' . bts('and', array(), NULL, 'boinc:account-preference') . '</span>'
  );
  $form['prefs']['advanced']['network']['net_end_hour'] = array(
    '#type' => 'select',
    '#options' => $form['hour_options']['#value'],
    '#default_value' => $default['net_end_hour']
  );
  $form['prefs']['advanced']['network']['hour_description'] = array(
    '#value' => '<div class="form-item slim"><div class="description">' . bts('Transfer files only during a particular period each day.', array(), NULL, 'boinc:account-preferences-computing') . '</div></div>'
  );
  $form['prefs']['advanced']['network']['daily_xfer_limit_mb'] = array(
    '#title' => bts('Limit usage to', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'textfield',
    '#field_suffix' => 'Mbytes',
    '#default_value' => $default['daily_xfer_limit_mb'],
    '#size' => 1
  );
  $form['prefs']['advanced']['network']['daily_xfer_period_days'] = array(
    '#field_prefix' => 'every',
    '#type' => 'textfield',
    '#field_suffix' => bts('days', array(), NULL, 'boinc:unit-of-time'),
    '#default_value' => $default['daily_xfer_period_days'],
    '#size' => 1,
    '#description' => bts('Example: BOINC should transfer at most 2000 MB of data every 30 days.', array(), NULL, 'boinc:account-preferences-computing'),
  );
  $form['prefs']['advanced']['network']['dont_verify_images'] = array(
    '#title' => bts('Skip data verification for image files?', array(), NULL, 'boinc:account-preferences-computing'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['dont_verify_images'],
    '#description' => bts('Only select "Yes" if your Internet provider modifies image files. Skipping verification reduces the security of BOINC.', array(), NULL, 'boinc:account-preferences-computing')
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
    '#value' => bts('Save changes', array(), NULL, 'boinc:form-save'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), drupal_get_path_alias("account/prefs/computing/edit")) . '</li>'
  );
  if ($venue AND $venue != 'generic') {
    global $base_path;
    $form['prefs']['form control tabs']['#value'] .= '<li class="tab">' .
      l(bts('Clear', array(), NULL, 'boinc:form-clear'), "account/prefs/computing/clear/{$venue}",
        array(
          'query' => 'destination=' . urlencode(drupal_get_path_alias('account/prefs/computing/combined')),
          'attributes' => array(
            'onclick' => 'return confirm(\'' . bts('This will remove all of your settings from the @name preference set. Are you sure?',
              array('@name' => $venue), NULL, 'boinc:account-preferences') . '\')'
          )
        )
      ) . '</li>';
  }
  $form['prefs']['view control'] = array(
    '#value' => '<li class="first alt tab">' . l('(' . bts('Show comparison view', array(), NULL, 'boinc:account-preferences') . ')', 'account/prefs/computing/combined') . '</li>'
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
  if (!verify_numeric($values['processor']['idle_time_to_run'], 1, 9999)) form_set_error('idle_time_to_run', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['idle_time_to_run']['#title']} [x] {$form['prefs']['advanced']['processor']['idle_time_to_run']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['suspend_if_no_recent_input'], 0, 9999)) form_set_error('suspend_if_no_recent_input', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['suspend_if_no_recent_input']['#title']} [x] {$form['prefs']['advanced']['processor']['suspend_if_no_recent_input']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['suspend_cpu_usage'], 0, 100)) form_set_error('suspend_cpu_usage', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['suspend_cpu_usage']['#title']} [x] {$form['prefs']['advanced']['processor']['suspend_cpu_usage']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['start_hour'], 0, 23)) form_set_error('start_hour', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['start_hour']['#title']} [x] {$form['prefs']['advanced']['processor']['start_hour']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['end_hour'], 0, 23)) form_set_error('end_hour', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['end_hour']['#title']} [x] {$form['prefs']['advanced']['processor']['end_hour']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['cpu_scheduling_period_minutes'], 1, 9999)) form_set_error('cpu_scheduling_period_minutes', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes']['#title']} [x] {$form['prefs']['advanced']['processor']['cpu_scheduling_period_minutes']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['max_ncpus_pct'], 0, 100)) form_set_error('max_ncpus_pct', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['max_ncpus_pct']['#title']} [x] {$form['prefs']['advanced']['processor']['max_ncpus_pct']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['processor']['cpu_usage_limit'], 0, 100)) form_set_error('cpu_usage_limit', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['processor']['cpu_usage_limit']['#title']} [x] {$form['prefs']['advanced']['processor']['cpu_usage_limit']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));

  // Storage preferences
  if (!verify_numeric($values['storage']['disk_max_used_gb'], 0, 9999999)) form_set_error('disk_max_used_gb', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['disk_max_used_gb']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_max_used_gb']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['disk_min_free_gb'], 0.001, 9999999)) form_set_error('disk_min_free_gb', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['disk_min_free_gb']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_min_free_gb']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['disk_max_used_pct'], 0, 100)) form_set_error('disk_max_used_pct', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['disk_max_used_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_max_used_pct']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['disk_interval'], 0, 9999999)) form_set_error('disk_interval', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['disk_interval']['#title']} [x] {$form['prefs']['advanced']['storage']['disk_interval']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['vm_max_used_pct'], 0, 100)) form_set_error('vm_max_used_pct', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['vm_max_used_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['vm_max_used_pct']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['ram_max_used_busy_pct'], 0, 100)) form_set_error('ram_max_used_busy_pct', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['ram_max_used_busy_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['ram_max_used_busy_pct']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['storage']['ram_max_used_idle_pct'], 0, 100)) form_set_error('ram_max_used_idle_pct', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['storage']['ram_max_used_idle_pct']['#title']} [x] {$form['prefs']['advanced']['storage']['ram_max_used_idle_pct']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));

  // Network preferences
  if (!verify_numeric($values['network']['work_buf_min_days'], 0, 10)) form_set_error('work_buf_min_days', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['work_buf_min_days']['#title']} [x] {$form['prefs']['advanced']['network']['work_buf_min_days']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['work_buf_additional_days'], 0, 10)) form_set_error('work_buf_additional_days', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['work_buf_additional_days']['#title']} [x] {$form['prefs']['advanced']['network']['work_buf_additional_days']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['max_bytes_sec_down'], 0, 9999.999)) form_set_error('max_bytes_sec_down', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['max_bytes_sec_down']['#title']} [x] {$form['prefs']['advanced']['network']['max_bytes_sec_down']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['max_bytes_sec_up'], 0, 9999.999)) form_set_error('max_bytes_sec_up', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['max_bytes_sec_up']['#title']} [x] {$form['prefs']['advanced']['network']['max_bytes_sec_up']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['net_start_hour'], 0, 23)) form_set_error('net_start_hour', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['net_start_hour']['#title']} [x] {$form['prefs']['advanced']['network']['net_start_hour']['#field_suffix']}"), NULL, 'boinc:account-prefrences-computing'));
  if (!verify_numeric($values['network']['net_end_hour'], 0, 23)) form_set_error('net_end_hour', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['net_end_hour']['#title']} [x] {$form['prefs']['advanced']['network']['net_end_hour']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['daily_xfer_limit_mb'], 0, 9999999)) form_set_error('daily_xfer_limit_mb', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#title']} [x] {$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
  if (!verify_numeric($values['network']['daily_xfer_period_days'], 0, 9999999)) form_set_error('daily_xfer_period_days', bts('Invalid setting for "%preference"', array('%preference' => "{$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#title']} [x] {$form['prefs']['advanced']['network']['daily_xfer_limit_mb']['#field_suffix']}"), NULL, 'boinc:account-preferences-computing'));
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
  $prefs['run_on_batteries'] = ($values['processor']['run_on_batteries']) ? 0 : 1;
  $prefs['run_if_user_active'] = ($values['processor']['run_if_user_active']) ? 0 : 1;
  $prefs['run_gpu_if_user_active'] = ($values['processor']['run_gpu_if_user_active']) ? 0 : 1;
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

  // transform old way to store the preset into new way
  // ideally this should already have happened in boincwork_generalprefs_form()
  if (isset($prefs['@attributes']['preset'])) {
    $prefs['preset'] = $prefs['@attributes']['preset'];
    unset($prefs['@attributes']['preset']);
  }
  // Save the preset selection (or lack thereof)
  if (!$preset OR $preset == 'custom') {
    $prefs['preset'] = 'custom';
  }
  else {
    $prefs['preset'] = $preset;
  }

  // If this is a new preference set, be sure to unset the "cleared" attribute
  if (isset($prefs['@attributes']['cleared'])) {
    unset($prefs['@attributes']['cleared']);
  }

  // Update database
  $result = boincwork_save_prefs($prefs, 'general', $venue);

  if (!$result) {
    watchdog('boincwork', 'Error updating global prefs for user @id: @message', array('@id' => $account->id, '@message' => mysqli_error()), WATCHDOG_ERROR);
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
  db_set_active('boinc_ro');
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
  db_set_active('boinc_ro');
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

  if ($new_host->rpc_seqno == $old_host->id) {
    rules_invoke_event('boincwork_circular_merge_error', $old_host->id, $new_host->id, variable_get('boinc_admin_mailing_list_subject_tag', ''));
    watchdog('boincwork',
      'Circular merge attempted, target host rpc_seqno is equal to old host\'s id: old host id=%old_host, target host id=%new_host',
      array(
        '%old_host' => $old_host->id,
        '%new_host' => $new_host->id,
      ),
      WATCHDOG_WARNING
    );
    $message = 'Could not merge due to a circular merge error. The site administrators have been contacted about this issue, and will investigate further.';
    return FALSE;
  }

  if ($new_host->userid==0) {
    rules_invoke_event('boincwork_zombie_merge_error', $old_host->id, $new_host->id, variable_get('boinc_admin_mailing_list_subject_tag', ''));
    watchdog('boincwork',
      'Zombie merge attempted, target host has userid=0: old host id=%old_host, target host id=%new_host',
      array(
        '%old_host' => $old_host->id,
        '%new_host' => $new_host->id,
      ),
      WATCHDOG_WARNING
    );
    $message = 'Could not merge because the target host has userid=0. The site administrators have been contacted about this issue, and will investigate further.';
    return FALSE;
  }

  // Move credit from the old host to the new host
  db_set_active('boinc_rw');
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
      $message = bts('Could not update credit', array(), NULL, 'boinc:account-host-merge');
    }
    return FALSE;
  }

  // Move results from the old host to the new host
  db_set_active('boinc_rw');
  $results_updated = db_query("
    UPDATE {result}
    SET hostid = '%d'
    WHERE hostid = '%d'",
    $new_host->id, $old_host->id
  );
  db_set_active('default');
  if (!$results_updated) {
    if ($message !== NULL) {
      $message = bts('Could not update results', array(), NULL, 'boinc:account-host-merge');
    }
    return FALSE;
  }

  // Retire the old host
  db_set_active('boinc_rw');
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
      $message = bts('Could not retire old computer', array(), NULL, 'boinc:account-host-merge');
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
    '#value' => '<p>' . bts('Sometimes BOINC assigns separate identities to'
      . ' the same computer by mistake. You can correct this by merging old'
      . ' identities with the newest one.', array(), NULL, 'boinc:account-host-merge') . '</p>'
      . '<p>'
      . bts('Check the computers that are the same as @name'
      . ' (created on @date at @time with computer ID @id)',
        array(
          '@name' => $current_host->domain_name,
          '@date' => date('j M Y', $current_host->create_time),
          '@time' => date('G:i:s T', $current_host->create_time),
          '@id' => $current_host->id,
        ),
        NULL, 'boinc:account-host-merge') . '</p>',
  );

  $options = array();
  foreach ($hosts as $host) {
    $options[$host->id] = array(
      $host->domain_name,
      date('j M Y G:i:s T', $host->create_time),
      $host->id,
    );
  }

  $form['merge'] = array(
    '#title' => '',
    '#type' => 'tableselect',
    '#header' => array(bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'), bts('Created', array(), NULL, 'boinc:host-details'), bts('Computer ID', array(), NULL, 'boinc:host-list')),
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
    '#value' => bts('Merge', array(), NULL, 'boinc:form-merge'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), "host/{$host_id}") . '</li>'
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
    $conjunction = bts('and', array(), NULL, 'boinc:account-preference');
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
    if (count($merged) == 1) {
      drupal_set_message(bts(
        'Computer @old_id has been merged successfully into @id.',
        array(
          '@old_id' => $list,
          '@id' => $current_host_id
        ),
        NULL, 'boinc:account-host-merge'));
    }
    else {
      drupal_set_message(bts(
        'Computers @old_ids have been merged successfully into @id.',
        array(
          '@old_ids' => $list,
          '@id' => $current_host_id
        ),
        NULL, 'boinc:account-host-merge'));
    }
  }

  if ($errors) {
    // Report any hosts that failed to merge
    foreach ($errors as $id => $error) {
      drupal_set_message(
        bts('Computer @old_id failed to merge: @message',
          array(
            '@old_id' => $id,
            '@message' => $error,
          ),
          NULL, 'boinc:account-host-merge'),
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
  require_boinc(array('app_types'));
  $app_types = get_app_types();

  // Load any existing preferences from BOINC account
  $prefs = boincwork_load_prefs('project', $venue);

  // Take note if this is not an established preference set on the account
  if (isset($prefs['@attributes']['cleared'])) {
    $established = FALSE;
  }

  // Extract modified tag if present
  $modified = NULL;
  if (isset($prefs['modified']['@value'])) {
    $modified = $prefs['modified']['@value'];
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
    'no_apple_gpu' => 0,
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
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes'))
  );

  // Identify preference sets that are established to distinguish what has been
  // saved to the database from what is just showing default values
  $form['#established'] = $established;

  // Top level form options
  $form['#tree'] = TRUE;

  // Hidden elements
  $form['modified'] = array(
    '#type' => 'hidden',
    '#value' => $modified,
  );
  $form['venue'] = array(
    '#type' => 'hidden',
    '#value' => $venue,
  );

  $form['separator_top'] = array(
    '#value' => '<div class="separator"></div>'
  );

  // Common project preferences
  $form['resource'] = array(
    '#title' => bts('Resource settings', array(), NULL, 'boinc:account-preferences-project'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['resource']['resource_share'] = array(
    '#title' => bts('Resource share', array(), NULL, 'boinc:account-preferences-project'),
    '#type' => 'textfield',
    '#default_value' => $default['resource_share'],
    '#size' => 5,
    '#description' => bts("Determines the proportion of your computer's resources allocated to this project. Example: if you participate in two BOINC projects with resource shares of 100 and 200, the first will get 1/3 of your resources and the second will get 2/3.", array(), NULL, 'boinc:account-preferences-project')
  );
  if ($app_types->count > 1) {
    if ($app_types->cpu) {
      $form['resource']['no_cpu'] = array(
        '#title' => bts('Use CPU', array(), NULL, 'boinc:account-preferences-project'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_cpu'] ? 0 : 1,
        '#description' => bts('Request CPU-only tasks from this project.', array(), NULL, 'boinc:account-preferences-project')
      );
    }
    if ($app_types->cuda) {
      $form['resource']['no_cuda'] = array(
        '#title' => bts('Use NVIDIA GPU', array(), NULL, 'boinc:account-preferences-project'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_cuda'] ? 0 : 1,
        '#description' => bts('Request NVIDIA GPU tasks from this project.', array(), NULL, 'boinc:account-preferences-project')
      );
    }
    if ($app_types->ati) {
      $form['resource']['no_ati'] = array(
        '#title' => bts('Use AMD GPU', array(), NULL, 'boinc:account-preferences-project'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_ati'] ? 0 : 1,
        '#description' => bts('Request AMD GPU tasks from this project.', array(), NULL, 'boinc:account-preferences-project')
      );
    }
    if ($app_types->intel_gpu) {
      $form['resource']['no_intel_gpu'] = array(
        '#title' => bts('Use INTEL GPU', array(), NULL, 'boinc:account-preferences-project'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_intel_gpu'] ? 0 : 1,
        '#description' => bts('Request Intel GPU tasks from this project.', array(), NULL, 'boinc:account-preferences-project')
      );
    }
    if ($app_types->apple_gpu) {
      $form['resource']['no_apple_gpu'] = array(
        '#title' => bts('Use Apple GPU', array(), NULL, 'boinc:account-preferences-project'),
        '#type' => 'radios',
        '#options' => $form['boolean_options']['#value'],
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $default['no_apple_gpu'] ? 0 : 1,
        '#description' => bts('Request Apple GPU tasks from this project.', array(), NULL, 'boinc:account-preferences-project')
      );
    }
  }

  if (variable_get('boinc_prefs_options_beta', FALSE)) {
    $form['beta'] = array(
      '#title' => bts('Beta settings', array(), NULL, 'boinc:account-preferences-project'),
      '#type' => 'fieldset',
      '#description' => null,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );
    $form['beta']['allow_beta_work'] = array(
      '#title' => bts('Run test applications?', array(), NULL, 'boinc:account-preferences-project'),
      '#type' => 'radios',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy'),
      '#default_value' => ($default['allow_beta_work']) ? 1 : 0,
      '#description' => bts('This helps us develop applications, but may cause jobs to fail on your computer', array(), NULL, 'boinc:account-preferences-project')
    );
  }

  // Add project specific prefs to the form
  boincwork_add_project_specific_prefs($form, $prefs);

  // Set whether to use this preference set by default for new computers
  $form['default_set'] = array(
    '#title' => bts('Default set', array(), NULL, 'boinc:account-preferences-project'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['default_set']['default_venue'] = array(
    '#title' => bts('Set used for new computers', array(), NULL, 'boinc:account-preferences-project'),
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
    '#value' => bts('Save changes', array(), NULL, 'boinc:form-save'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), $_GET['q']) . '</li>'
  );
  if ($venue AND $venue != 'generic') {
    global $base_path;
    $form['prefs']['form control tabs']['#value'] .= '<li class="tab">' .
      l(bts('Clear', array(), NULL, 'boinc:form-clear'), "account/prefs/project/clear/{$venue}",
        array(
          'query' => 'destination=' . urlencode(drupal_get_path_alias('account/prefs/project/combined')),
          'attributes' => array(
            'onclick' => 'return confirm(\'' . bts('This will remove all of your settings from the @name preference set. Are you sure?',
              array('@name' => $venue), NULL, 'boinc:account-preferences') . '\')'
          )
        )
      ) . '</li>';
  }
  $form['prefs']['view control'] = array(
      '#value' => '<li class="first alt tab">' . l('(' . bts('Show comparison view', array(), NULL, 'boinc:account-preferences') . ')', 'account/prefs/project/combined') . '</li>'
  );
  $form['prefs']['form control tabs suffix'] = array(
    '#value' => '</ul>'
  );

  return $form;
}

/**
 * Add project specific preferences to the project preferences form
 */
function boincwork_add_project_specific_prefs(&$form, $prefs) {
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
          bts('At least one application must be selected', array(), NULL, 'boinc:account-preferences-project')
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
  global $user;
  global $site_name;

  require_boinc(array('app_types'));
  $app_types = get_app_types();

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
    if ($app_types->apple_gpu) $prefs['no_apple_gpu'] = ($edit['resource']['no_apple_gpu']) ? 0 : 1;
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
    watchdog('boincwork', 'Error updating project prefs for user @id: @message', array('@id' => $user->id, '@message' => mysqli_error()), WATCHDOG_ERROR);
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
  $profile->language = '';
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
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes'))
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
    '#title' => bts('General settings', array(), NULL, 'boinc:account-preferences-community'),
    '#weight' => 0,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  // Add the BOINC user name (non-unique, user editable)
  $form['general']['boincuser_name'] = array(
    '#type' => 'textfield',
    '#title' => bts('Name', array(), NULL, 'boinc:user-or-team-name'),
    '#default_value' => $account->boincuser_name,
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#required' => TRUE,
    '#description' => '',
    '#size' => 40
  );
  // Time zone
  if (variable_get('configurable_timezones', 1)) {
    $zones = _system_zonelist();
    $form['general']['timezone'] = array(
      '#type' => 'select',
      '#title' => bts('Time zone', array(), NULL, 'boinc:account-preferences-community'),
      '#default_value' => ($account->timezone !== NULL) ? $account->timezone : variable_get('date_default_timezone', 0),
      '#options' => $zones,
      '#description' => '',
    );
  }

  // Notification options
  $form['notifications'] = array(
    '#type' => 'fieldset',
    '#title' => bts('Notification settings', array(), NULL, 'boinc:account-preferences-community'),
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
      '#title' => bts('Receive email notification for private messages?', array(), NULL, 'boinc:account-preferences-community'),
      '#description' => ' ',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy')
    )
  );
  $form['notifications']['friend_notification'] = array(
    '#type' => 'radios',
    '#title' => bts('Receive email notification for friend requests?', array(), NULL, 'boinc:account-preferences-community'),
    '#description' => ' ',
    '#options' => array(0 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), -1 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
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
      '#title' => bts('Language settings', array(), NULL, 'boinc:account-preferences-community'),
      '#weight' => 10,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    // Get language negotiation settings.
    $mode = variable_get('language_negotiation', LANGUAGE_NEGOTIATION_NONE);
    $user_preferred_language = user_preferred_language($account);
    $form['locale']['language'] = array(
      '#type' => 'select',
      '#title' => bts('Language', array(), NULL, 'boinc:account-preferences-community'),
      '#default_value' => check_plain($user_preferred_language->language),
      '#options' => $names,
      '#description' => ($mode == LANGUAGE_NEGOTIATION_PATH) ? bts("This account's default language for e-mails and preferred language for site presentation.", array(), NULL, 'boinc:account-preferences-community') : bts("This account's default language for e-mails.", array(), NULL, 'boinc:account-preferences-community'),
    );
  }

  // Avatar options
  $form['gravatar'] = array(
    '#type' => 'item',
    '#value' => bts('If you have a <a href="@gravatar-check">valid Gravatar</a> associated with your e-mail address, it will be used for your user picture.', array('@gravatar-check' => 'http://en.gravatar.com/site/check/' . $account->mail), NULL, 'boinc:account-preferences-community'),
    '#description' => bts('Your Gravatar will not be shown if you upload a user picture.', array(), NULL, 'boinc:account-preferences-community'),
  );
  if (user_access('disable own gravatar', $account)) {
    $form['gravatar'] = array(
      '#type' => 'checkbox',
      '#title' => bts('If you have a <a href="@gravatar-check">valid Gravatar</a> associated with your e-mail address, use it for your user picture.', array('@gravatar-check' => 'http://en.gravatar.com/site/check/' . $account->mail), NULL, 'boinc:account-preferences-community'),
      '#description' => bts('Gravatar will not be shown if an avatar is uploaded.', array(), NULL, 'boinc:account-preferences-community'),
      '#default_value' => isset($account->gravatar) ? $account->gravatar : 0,
      '#disabled' => !empty($account->picture),
    );
  }
  $form['gravatar']['#weight'] = 15;
  $form['gravatar']['#prefix'] = '<fieldset class="collapsible"><legend><a href="#">' . bts('Avatar settings', array(), NULL, 'boinc:account-preferences-community') . '</a></legend>';
  // Upload an avatar (pulled from profile_node_form):
  if (!empty($profile_form['field_image'])) {
    $form['field_image'] = $profile_form['field_image'];
  }
  else {
    $form['field_image'] = array(
      '#value' => '<div class="form-item">'
        . '<label class="placeholder">'
        . bts('This is not available until your profile is set up.', array(), NULL, 'boinc:account-preferences-community')
        . '</label>'
        . l(bts('Create a profile', array(), NULL, 'boinc:account-preferences-community'), 'account/profile/edit', array('attributes' => array('class' => 'form-link')))
        . '</div>',
    );
  }
  $form['field_image'][0]['#title'] = bts('Upload an avatar', array(), NULL, 'boinc:account-preferences-community');
  $form['field_image']['#weight'] = 20;
  $form['field_image']['#suffix'] = '</fieldset>';

  // Forum options
  $form['forums'] = array(
    '#type' => 'fieldset',
    '#title' => bts('Forum settings', array(), NULL, 'boinc:account-preferences-community'),
    '#weight' => 25,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['forums']['comments_per_page'] = array(
    '#type' => 'select',
    '#title' => bts('In discussion topics, show at most @comments_per_page', array('@comments_per_page' => ''), NULL, 'boinc:account-preferences-community'),
    '#options' => array(10 => 10, 20 => 20, 30 => 30, 50 => 50, 100 => 100),
    '#default_value' => $default['comments_per_page']
  );
  // Can't have a typical Drupal form suffix on a select box?
  $form['forums']['comments_per_page_suffix'] = array(
    '#value' => '<span>' . bts('comments per page', array(), NULL, 'boinc:account-preferences-community') . '</span>'
  );
  $form['forums']['comments_order'] = array(
    '#type' => 'select',
    '#title' => bts('Sort comments in discussions', array(), NULL, 'boinc:account-preferences-community'),
    '#options' => array(1 => bts('Newest post first', array(), NULL, 'boinc:account-preferences-community'), 2 => bts('Oldest post first', array(), NULL, 'boinc:account-preferences-community')),
    '#default_value' => $default['comments_order']
  );
  // Signature (pulled from user_edit_form):
  if (variable_get('user_signatures', 0) && module_exists('comment')) {
    $form['forums']['signature'] = array(
      '#type' => 'textarea',
      '#title' => bts('Signature', array(), NULL, 'boinc:account-preferences-community'),
      '#description' => bts('Your signature will be publicly displayed at the end of your comments.', array(), NULL, 'boinc:account-preferences-community'),
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
      '#title' => bts('Hide signatures in forums', array(), NULL, 'boinc:account-preferences-community'),
      '#description' => ' ',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy'),
      '#default_value' => isset($account->hide_signatures) ? $account->hide_signatures : 0,
    );
  }

  //Bottom separator
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
    '#value' => bts('Save changes', array(), NULL, 'boinc:form-save'),
    '#suffix' => '</li>',
    '#weight' => 1002,
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), $_GET['q']) . '</li>',
    '#weight' => 1003,
  );
  $form['form control tabs suffix'] = array(
    '#value' => '</ul>',
    '#weight' => 1004,
  );
  return $form;
}

/**
  * Handle validation submission of community preferences form.
  */
function communityprefs_form_validate($form, &$form_state) {
  // require_boinc();
  global $user;
  $account = user_load($user->uid);
  $boinc_user = BoincUser::lookup_id($account->boincuser_id);
  $edit = $form_state['values'];

  if ($edit['boincuser_name'] != $boinc_user->name) {
    $blacklist1 = preg_split('/\r\n|\r|\n/', variable_get('boinc_weboptions_blacklisted_usernames', "admin\nadministrator\nmoderator"));
    $blacklist2 = array();
    if (is_array($blacklist1)) {
      $blacklist2 = array_map('strtolower', $blacklist1);
    }
    if (in_array(strtolower($edit['boincuser_name']), $blacklist2)) {
      form_set_error('boincuser_name',
        bts('You may not use username @blname, as that name is not allowed. Please choose another name.',
            array('@blname' => $edit['boincuser_name']),
            NULL, 'boinc:account-preferences-community'));
      return false;
    }
  }

  return true;
}

/**
  * Handle post-validation submission of community preferences form.
  */
function communityprefs_form_submit($form, &$form_state) {
  require_boinc('boinc_db');
  global $user;
  $account = user_load($user->uid);
  $boinc_user = BoincUser::lookup_id($account->boincuser_id);
  $edit = $form_state['values'];
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

  // Avatar settings - only set if profile_node exists.
  if ($profile_node) {
    if (!$edit['field_image']) $edit['field_image'] = array();
    $profile_node->field_image = $edit['field_image'];
    node_save($profile_node);
    // Flush this from the node cache or changes won't show up immediately!
    $profile_node = node_load($profile_node->nid, NULL, TRUE);
  }

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

  drupal_set_message(bts('Your community preferences have been updated.', array(), NULL, 'boinc:account-preferences-community'));

  // Form will not redirect if storage is set; not good if language changes
  unset($form_state['storage']);
}

/**
 * The structure of the privacy preferences form
 */
function boincwork_privacyprefs_form(&$form_state) {
  require_boinc(array('user', 'prefs', 'util', 'consent'));

  global $user;
  $account = user_load($user->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);

  // Load preferences from BOINC account
  $prefs = boincwork_load_prefs('project');

  //if (!$prefs AND !$initialize_if_empty) return null;

  $privacy_consent_types = boincwork_load_privacyconsenttypes();

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
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes'))
  );

  $form['privacy'] = array(
    '#title' => bts('Privacy settings', array(), NULL, 'boinc:account-preferences-privacy'),
    '#type' => 'fieldset',
    '#description' => null,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['privacy']['send_email'] = array(
    '#title' => bts('Is it OK for @project and your team (if any) to email you?', array('@project' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:account-preferences-privacy'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['privacy']['send_email']
  );
  $form['privacy']['show_hosts'] = array(
    '#title' => bts('Should @project show your computers on its website?', array('@project' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:account-preferences-privacy'),
    '#description' => bts('At times, you may be asked to enable this option in order to receive help from the forums. Advanced users may need to be able to inspect your computers\' information in order to help diagnose any problems.', array(), NULL, 'boinc:account-preferences-privacy'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['privacy']['show_hosts']
  );

  // Loop over privacy consent types and create form question for each
  // option that deals with privacy.
  foreach ($privacy_consent_types as $ct) {

    $currstate = (check_user_consent($boincuser, $ct['shortname'])) ? 1 : 0 ;
    // Set name to 'privacyconsent_SHORTNAME', which can be parsed
    // later in the submit function.
    $form['privacy']['privacyconsent_'.$ct['shortname']] = array(
      '#title' => bts($ct['description'], array(), NULL, 'boinc:account-preferences-privacy'),
      '#type' => 'radios',
      '#options' => $form['boolean_options']['#value'],
      '#attributes' => array('class' => 'fancy'),
      '#default_value' => $currstate,
    );

    // Add a description with link to the question 'Do you consent to
    // exporting your data...'.
    $mypatt = '/Do you consent to exporting your data/';
    if (preg_match($mypatt, $ct['description']) ) {
      $form['privacy']['privacyconsent_'.$ct['shortname']]['#description'] = bts('See our !privacy_policy_link for the current list of statistics exports.',
      array(
        '!privacy_policy_link' => l(
          bts('privacy policy', array(), NULL, 'boinc:account-preferences-privacy'),
          '/privacy'
        )
      ),
      NULL, 'boinc:account-preferences-privacy');
    }
  }

  // Ignore and block users
  if (module_exists('ignore_user')) {
    $form['ignoreblock'] = array(
      '#title' => bts('Ignore Users', array(), NULL, 'boinc:account-preferences-privacy'),
      '#type' => 'fieldset',
      '#description' => bts('<p>You may ignore users in the forums and block users from sending you private messages.<p>', array(), NULL, 'boinc:ignore-user-help'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );

    // Table for ignored users
    $form['ignoreblock']['current_ignore_section'] = array(
      '#type' => 'item',
      '#value' => bts('Current users on your Ignore List', array(), NULL, 'boinc:ignore-user-list'),
      '#prefix' => '<h4>',
      '#suffix' => '</h4>',
      '#weight' => -20,
    );

    $ignored_users = _ignore_user_ignored_users();
    foreach ($ignored_users as $ignored_user) {
      $form['ignoreblock']['username'][$ignored_user['iuid']] = array(
        '#value' => $ignored_user['username'],
      );
      $form['ignoreblock']['delete'][$ignored_user['iuid']] = array(
        '#value' => l(
          bts('delete', array(), NULL, 'boinc:ignore-user-delete-button'),
          'account/prefs/privacy/ignore_user/remove/'. $ignored_user['iuid'],
          array()
        ),
      );
    }
    $form['ignoreblock']['pager'] = array('#value' => theme('pager', NULL, 10, 0));

    // Sub-form to add user to ignore list
    $form['ignoreblock']['add_ignore_user_section'] = array(
      '#type' => 'item',
      '#value' => bts('Add user to Ignore List', array(), NULL, 'boinc:ignore-user-add'),
      '#prefix' => '<h4>',
      '#suffix' => '</h4>',
      '#weight' => 10,
    );

    $form['ignoreblock']['addusername_toignorelist'] = array(
      '#type' => 'textfield',
      '#title' => bts('Username', array(), NULL, 'boinc:ignore-user-searchbox'),
      '#description' => bts('To lookup a username start typing in the search box. A list of usernames will appear as you type. The number appearing in the suffix is the BOINC id. You can find a user\'s BOINC id on their user profile page.', array(), NULL, 'boinc:ignore-user-searchbox-help'),
      '#weight' => 11,
      '#size' => 50,
      '#autocomplete_path' => 'boincuser/autocomplete',
    );

    $form['ignoreblock']['addusername_submit'] = array(
      '#type' => 'submit',
      '#value' => bts('Ignore user', array(), NULL, 'boinc:ignore-user-add'),
      '#submit' => array('_boincwork_ignore_list_form_submit'),
      '#weight' => 12,
      '#attributes' => array('class' => 'add_ignore_user'),
    );
  }// endif module_exists

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
    '#value' => bts('Save changes', array(), NULL, 'boinc:form-save'),
    '#validate' => array('boincwork_privacyprefs_form_validate'),
    '#submit' => array('boincwork_privacyprefs_form_submit'),
    '#suffix' => '</li>'
  );
  $form['prefs']['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), $_GET['q']) . '</li>'
  );
  $form['prefs']['form control tabs suffix'] = array(
    '#value' => '</ul>'
  );

  return $form;
}

/**
 * Theme the form where user's can manage their ignore list
 */
function theme_boincwork_privacyprefs_form($form) {

  $output = '';
  $output .= drupal_render($form['privacy']);

  $header = array(
    bts('Username', array(), NULL, 'boinc:ignore-user-list'),
    bts('Operations', array(), NULL, 'boinc:ignore-user-list')
  );

  $rows = array();
  if (isset($form['ignoreblock']['username']) && is_array($form['ignoreblock']['username'])) {
    foreach (element_children($form['ignoreblock']['username']) as $key) {
      $row = array();
      $row[] = drupal_render($form['ignoreblock']['username'][$key]);
      $row[] = drupal_render($form['ignoreblock']['delete'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(
      array(
        'data' => bts('You have not added any users to your Ignore List.', array(), NULL, 'boinc:ignore-user-list'),
        'colspan' => '2',
      )
    );
  }

  $attr = array('class' => 'ignore_user');
  $form['ignoreblock']['current_list']['ignored_users']['#value'] = theme('table', $header, $rows, $attr);
  $output .= drupal_render($form['current_list']);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
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
  require_boinc(array('user', 'prefs', 'consent'));

  global $user;
  $account = user_load($user->uid);

  // Load BOINC account
  $boincuser = BoincUser::lookup_id($account->boincuser_id);

  // Privacy preferences
  $boincuser->send_email = ($form_state['values']['send_email']) ? true : false;
  $boincuser->show_hosts = ($form_state['values']['show_hosts']) ? true : false;

  // Privacy consent options, extract the 'privacyconsent_SHORTNAME'
  // from values array, and loop over them; each is checked with
  // check_consent_type(). Also check the current state of the option
  // in the database. If the form value is a new state, then set it.
  $result = preg_grep("/^privacyconsent/", array_keys($form_state['values']));
  $privacyconsent_prefs = array_intersect_key($form_state['values'], array_flip($result));
  foreach ($privacyconsent_prefs as $name => $newstate) {
    $subname = explode('_', $name)[1];
    $currstate = (check_user_consent($boincuser, $subname)) ? 1 : 0 ;
    list($checkct, $ctid) = check_consent_type($subname);
    if ($checkct && ($currstate != $newstate)) {
      consent_to_a_policy($boincuser, $ctid, $newstate, 0, 'Webform', time());
    }
  }

  //project_prefs_update($boincuser, $main_prefs);

  db_set_active('boinc_rw');
  db_query("UPDATE user SET send_email = '{$boincuser->send_email}', show_hosts = '{$boincuser->show_hosts}' WHERE id = '{$boincuser->id}'");
  db_set_active('default');

  drupal_set_message(t('Your privacy preferences have been updated.'));
}

/**
 * Additional submit handler for privacy form, button to add user to
 * ignore list.
 */
function _boincwork_ignore_list_form_submit($form, $form_state) {
  boincwork_ignore_user_add_user_username($form_state['values']['addusername_toignorelist']);
  drupal_set_message(
    bts('@username has been added to your ignore list. See your !privacy_preferences for more details.',
      array(
        '@username' => $form_state['values']['addusername_toignorelist'],
        '!privacy_preferences' => l(bts('privacy preferences', array(), NULL, 'boinc:ignore-user-add'), 'account/prefs/privacy'),
      ),
      NULL, 'boinc:ignore-user-add'),
    'status');
}

/**
 * Structure of the select application form in the task table.
 */
function boincwork_selectapp_form(&$form_state, $apps, $current_app) {

  $form['selectapp'] = array(
    '#type' => 'select',
    '#attributes' => array(
      'class' => 'task-app-filter',
      'onchange' => 'this.form.submit();',
    ),
    '#default_value' => $current_app,
    '#options' => $apps,
    '#post_render' => array('_boincwork_selectapp_form_callback'),
  );

  // Class task-app-filter-submit for this form is used in
  // theming. CSS sets 'display:none' if javascript is present. Thus
  // only non-js users/browsers will see the Apply Filter button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => bts('Apply Filter', array(), NULL, 'boinc:form-save'),
    '#attributes' => array('class' => 'js-hide',),
  );

  return $form;
}

/**
 * Submit function for select appliacation form.
 */
function boincwork_selectapp_form_submit($form, &$form_state) {
  $myargs = arg();
  array_pop($myargs);
  $newpath = implode('/', $myargs ) . '/' . $form['selectapp']['#value'];
  $form_state['redirect'] = $newpath;
}

/**
 * Select application helper callback.
 *
 * Manually disables the option with value -1, which acts as the
 * 'title' for the Application drop down box.
 */
function _boincwork_selectapp_form_callback($theContent, $theElement) {
  $disabled = '<option value="-1" disabled hidden';
  $newContent = preg_replace('/<option value="-1"/', $disabled, $theContent);
  return $newContent;
}
