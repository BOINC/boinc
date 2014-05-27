<?php
// $Id$

/**
 * Functions that are shared amongst files and dependent modules go
 * here to keep the clutter down in the main module file.
 */ 

/**
 * When a venue is changed in preference forms, don't carry over the preset
 * that was selected
 */
function boincwork_ahah_helper_venue_submit($form, &$form_state) {
  $form_state['storage']['prefs']['preset'] = null;
  ahah_helper_generic_submit($form, $form_state);
}

/**
 * Get a predetermined set of preferences
 */
function boincwork_get_preset_prefs($preset = null) {
  $saved_state = variable_get('boincwork_preset_prefs', null);
  
  // If not configured yet, use these defaults
  if (!$saved_state) {
    $saved_state = '
      <general_preferences>
        <preset name="standard">
          <run_on_batteries>0</run_on_batteries>
          <run_if_user_active>0</run_if_user_active>
          <run_gpu_if_user_active>1</run_gpu_if_user_active>
          <idle_time_to_run>3</idle_time_to_run>
          <suspend_if_no_recent_input>0</suspend_if_no_recent_input>
          <suspend_cpu_usage>25</suspend_cpu_usage>
          <start_hour>0</start_hour>
          <end_hour>0</end_hour>
          <leave_apps_in_memory>1</leave_apps_in_memory>
          <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
          <max_cpus>16</max_cpus>
          <max_ncpus_pct>100</max_ncpus_pct>
          <cpu_usage_limit>80</cpu_usage_limit>
          <disk_max_used_gb>8</disk_max_used_gb>
          <disk_min_free_gb>4</disk_min_free_gb>
          <disk_max_used_pct>10</disk_max_used_pct>
          <disk_interval>60</disk_interval>
          <vm_max_used_pct>0</vm_max_used_pct>
          <ram_max_used_busy_pct>15</ram_max_used_busy_pct>
          <ram_max_used_idle_pct>50</ram_max_used_idle_pct>
          <work_buf_min_days>0.1</work_buf_min_days>
          <work_buf_additional_days>0.25</work_buf_additional_days>
          <confirm_before_connecting>1</confirm_before_connecting>
          <hangup_if_dialed>0</hangup_if_dialed>
          <max_bytes_sec_down>0</max_bytes_sec_down>
          <max_bytes_sec_up>0</max_bytes_sec_up>
          <net_start_hour>0</net_start_hour>
          <net_end_hour>0</net_end_hour>
          <daily_xfer_limit_mb>0</daily_xfer_limit_mb>
          <daily_xfer_period_days>0</daily_xfer_period_days>
          <dont_verify_images>0</dont_verify_images>
        </preset>
        <preset name="maximum">
          <run_on_batteries>1</run_on_batteries>
          <run_if_user_active>1</run_if_user_active>
          <run_gpu_if_user_active>1</run_gpu_if_user_active>
          <idle_time_to_run>3</idle_time_to_run>
          <suspend_if_no_recent_input>0</suspend_if_no_recent_input>
          <suspend_cpu_usage>0</suspend_cpu_usage>
          <start_hour>0</start_hour>
          <end_hour>0</end_hour>
          <leave_apps_in_memory>1</leave_apps_in_memory>
          <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
          <max_cpus>16</max_cpus>
          <max_ncpus_pct>100</max_ncpus_pct>
          <cpu_usage_limit>100</cpu_usage_limit>
          <disk_max_used_gb>100</disk_max_used_gb>
          <disk_min_free_gb>2</disk_min_free_gb>
          <disk_max_used_pct>100</disk_max_used_pct>
          <disk_interval>60</disk_interval>
          <vm_max_used_pct>50</vm_max_used_pct>
          <ram_max_used_busy_pct>80</ram_max_used_busy_pct>
          <ram_max_used_idle_pct>90</ram_max_used_idle_pct>
          <work_buf_min_days>0.1</work_buf_min_days>
          <work_buf_additional_days>0.25</work_buf_additional_days>
          <confirm_before_connecting>1</confirm_before_connecting>
          <hangup_if_dialed>0</hangup_if_dialed>
          <max_bytes_sec_down>0</max_bytes_sec_down>
          <max_bytes_sec_up>0</max_bytes_sec_up>
          <net_start_hour>0</net_start_hour>
          <net_end_hour>0</net_end_hour>
          <daily_xfer_limit_mb>0</daily_xfer_limit_mb>
          <daily_xfer_period_days>0</daily_xfer_period_days>
          <dont_verify_images>0</dont_verify_images>
        </preset>
        <preset name="green">
          <run_on_batteries>0</run_on_batteries>
          <run_if_user_active>1</run_if_user_active>
          <run_gpu_if_user_active>1</run_gpu_if_user_active>
          <idle_time_to_run>3</idle_time_to_run>
          <suspend_if_no_recent_input>1</suspend_if_no_recent_input>
          <suspend_cpu_usage>75</suspend_cpu_usage>
          <start_hour>0</start_hour>
          <end_hour>0</end_hour>
          <leave_apps_in_memory>1</leave_apps_in_memory>
          <cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>
          <max_cpus>16</max_cpus>
          <max_ncpus_pct>100</max_ncpus_pct>
          <cpu_usage_limit>80</cpu_usage_limit>
          <disk_max_used_gb>8</disk_max_used_gb>
          <disk_min_free_gb>4</disk_min_free_gb>
          <disk_max_used_pct>10</disk_max_used_pct>
          <disk_interval>60</disk_interval>
          <vm_max_used_pct>0</vm_max_used_pct>
          <ram_max_used_busy_pct>50</ram_max_used_busy_pct>
          <ram_max_used_idle_pct>75</ram_max_used_idle_pct>
          <work_buf_min_days>0.1</work_buf_min_days>
          <work_buf_additional_days>0.25</work_buf_additional_days>
          <confirm_before_connecting>1</confirm_before_connecting>
          <hangup_if_dialed>0</hangup_if_dialed>
          <max_bytes_sec_down>100000</max_bytes_sec_down>
          <max_bytes_sec_up>10000</max_bytes_sec_up>
          <net_start_hour>0</net_start_hour>
          <net_end_hour>0</net_end_hour>
          <daily_xfer_limit_mb>100</daily_xfer_limit_mb>
          <daily_xfer_period_days>2</daily_xfer_period_days>
          <dont_verify_images>1</dont_verify_images>
        </preset>
        <preset name="minimum">
          <run_on_batteries>0</run_on_batteries>
          <run_if_user_active>0</run_if_user_active>
          <run_gpu_if_user_active>0</run_gpu_if_user_active>
          <idle_time_to_run>5</idle_time_to_run>
          <suspend_if_no_recent_input>1</suspend_if_no_recent_input>
          <suspend_cpu_usage>25</suspend_cpu_usage>
          <start_hour>0</start_hour>
          <end_hour>8</end_hour>
          <leave_apps_in_memory>0</leave_apps_in_memory>
          <cpu_scheduling_period_minutes>120</cpu_scheduling_period_minutes>
          <max_cpus>4</max_cpus>
          <max_ncpus_pct>25</max_ncpus_pct>
          <cpu_usage_limit>40</cpu_usage_limit>
          <disk_max_used_gb>2</disk_max_used_gb>
          <disk_min_free_gb>4</disk_min_free_gb>
          <disk_max_used_pct>5</disk_max_used_pct>
          <disk_interval>120</disk_interval>
          <vm_max_used_pct>0</vm_max_used_pct>
          <ram_max_used_busy_pct>5</ram_max_used_busy_pct>
          <ram_max_used_idle_pct>20</ram_max_used_idle_pct>
          <work_buf_min_days>0.1</work_buf_min_days>
          <work_buf_additional_days>0.25</work_buf_additional_days>
          <confirm_before_connecting>1</confirm_before_connecting>
          <hangup_if_dialed>0</hangup_if_dialed>
          <max_bytes_sec_down>15000</max_bytes_sec_down>
          <max_bytes_sec_up>5000</max_bytes_sec_up>
          <net_start_hour>0</net_start_hour>
          <net_end_hour>8</net_end_hour>
          <daily_xfer_limit_mb>1000</daily_xfer_limit_mb>
          <daily_xfer_period_days>30</daily_xfer_period_days>
          <dont_verify_images>0</dont_verify_images>
        </preset>
      </general_preferences>';
  }
  
  // Convert XML data to array format
  $preset_prefs = load_configuration($saved_state);
  
  if ($preset) {
    // Load preset from configuration
    $preset_prefs = (array) $preset_prefs['general_preferences'];
    if (isset($preset_prefs['preset'])) {
      if (!is_numeric(key($preset_prefs['preset']))) {
        $preset_prefs['preset'] = array($preset_prefs['preset']);
      }
      foreach ($preset_prefs['preset'] as $key => $prefs) {
        if (isset($prefs['@attributes']['name']) AND $prefs['@attributes']['name'] == $preset) {
          return $preset_prefs['preset'][$key];
        }
      }
    }
  }
  return $preset_prefs;
}

/**
 * Load (and validate) the project specific configuration XML
 */
function boincwork_get_project_specific_config() {
  $raw_config_data = variable_get('boinc_project_specific_prefs_config', '');
  
  $xsd = './' . drupal_get_path('module', 'boincwork') . '/includes/projectprefs.xsd';
  libxml_use_internal_errors(true);
  
  $xml = new DomDocument();
  $xml->loadXML($raw_config_data, LIBXML_NOBLANKS);
  if (!$xml->schemaValidate($xsd)) {
    $errors = libxml_get_errors();
    $lines = explode("\r", $raw_config_data);
    drupal_set_message("{$errors[0]->message} at line {$errors[0]->line}" .
      ': <br/>' . htmlentities($lines[$errors[0]->line - 1]), 'error');
    return NULL;
  }
  
  // Convert XML to array for validation
  $xml = load_configuration($raw_config_data);
  return $xml;
}

/**
 * Get rules by which to validate project specific data
 */
function boincwork_get_project_specific_config_validation_rules($xml = array()) {
  $rules = array();
  if (!$xml) {
    // Read the config XML
    $xml = boincwork_get_project_specific_config();
    $xml = $xml['project_specific_preferences'];
  }
  foreach ($xml as $type => $elements) {
    if (is_array($elements) AND !is_numeric(key($elements))) {
      $elements = array($elements);
    }
    switch ($type) {
    case 'compound':
      foreach ($elements as $element) {
        $rules = $rules + boincwork_get_project_specific_config_validation_rules($element);
      }
      break;
      
    case 'text':
      foreach ($elements as $element) {
        $name = $element['@attributes']['name'];
        $rules[$name] = array(
          'datatype' => $element['@attributes']['datatype']
        );
        if (isset($element['@attributes']['min'])) {
          $rules[$name]['min'] = $element['@attributes']['min'];
        }
        if (isset($element['@attributes']['max'])) {
          $rules[$name]['max'] = $element['@attributes']['max'];
        }
      }
      break;
    /*
    case 'radio':
    case 'dropdown':
      foreach ($elements as $element) {
        $name = $element['@attributes']['name'];
        $options = array();
        foreach ($element['items']['item'] as $option) {
          if (is_array($option) AND isset($option['@value'])) {
            $option = $option['@value'];
          }
          $options[] = $option;
        }
        $rules[$name] = array(
          'options' => $options
        );
      }
      break;
    */
    case 'apps':
      // At least one app needs to be selected
      $rules['apps'] = array(
        'minimum selected' => 1,
        'list' => array()
      );
      foreach ($elements as $element) {
        foreach ($element['app'] as $app) {
          $name = "app_{$app['@attributes']['id']}";
          $rules['apps']['list'][] = $name;
          //$rules[$name] = array(
          //  'options' => $options
          //);
        }
      }
      break;
    /*
    case 'boolean':
      // Shouldn't need to validate boolean...
      break;
      */
    default:
    }
  }
  return $rules;
}

/**
 * Define how project specific settings should be saved
 */
function boincwork_format_project_specific_prefs_data($values, $xml = array()) {
  $defaults = array();
  if (!$xml) {
    // Read the config XML
    $xml = boincwork_get_project_specific_config();
    $xml = $xml['project_specific_preferences'];
  }
  foreach ($xml as $type => $elements) {
    $structure_data = array();
    if (is_array($elements) AND !is_numeric(key($elements))) {
      $elements = array($elements);
    }
    switch ($type) {
    case 'compound':
      foreach ($elements as $element) {
        $name = $element['@attributes']['name'];
        $default[$name]['@attributes'] = boincwork_format_project_specific_prefs_data($values, $element['attributes']);
      }
      $defaults += $default;
      break;
      
    case 'group':
      foreach ($elements as $element) {
        $defaults += boincwork_format_project_specific_prefs_data($values, $element);
      }
      break;
      
    case 'text':
    case 'radio':
    case 'dropdown':
    case 'boolean':
      foreach ($elements as $element) {
        $name = $element['@attributes']['name'];
        if (isset($element['@attributes']['entitytype']) AND $element['@attributes']['entitytype'] == 'attribute') {
          $defaults['@attributes'][$name] = $values[$name];
        }
        else {
          $defaults[$name] = $values[$name];
        }
      }
      break;
    
    case 'apps':
      foreach ($elements as $element) {
        $defaults['app_id'] = array();
        foreach ($element['app'] as $app) {
          $app_id = $app['@attributes']['id'];
          if ($values["app_{$app_id}"]) {
            $defaults['app_id'][] = $app_id;
          }
        }
      }
      break;
      
    default:
    }
  }
  return $defaults;
}

/**
 * Add an element to the form based on its definition
 */
function boincwork_generate_prefs_element(&$form, $type, $elements, $user_prefs = null) {
  switch ($type) {
  case 'text':
    if (!is_numeric(key($elements))) {
      $elements = array($elements);
    }
    foreach ($elements as $element) {
      $name = $element['@attributes']['name'];
      $default = $element['@attributes']['default'];
      $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
      $description = '';
      if (isset($element['description'])) {
        $description = is_array($element['description']) ? $element['description']['@value'] : $element['description'];
      }
      
      $value = $default;
      $user_pref = $user_prefs;
      $entitytype = isset($element['@attributes']['entitytype']) ? $element['@attributes']['entitytype'] : 'element';
      if ($entitytype == 'attribute') {
        $user_pref = $user_prefs['@attributes'];
      }
      if (isset($user_pref[$name])) {
        if (is_array($user_pref[$name]) AND isset($user_pref[$name]['@value'])) {
          $value = $user_pref[$name]['@value'];
        }
        else {
          $value = $user_pref[$name];
        }
      }
      
      // Use appropriate datatype
      if (isset($element['@attributes']['datatype'])) {
        switch($element['@attributes']['datatype']) {
        case 'integer':
          $value = (int) $value;
          break;
          
        case 'float':
          $value = number_format((float) $value, 1);
          break;
        
        default:
        }
      }
      
      $form[$name] = array(
        '#title' => t($title),
        '#type' => 'textfield',
        '#default_value' => $value,
        '#size' => 5,
        '#description' => t($description) . t(' Default value: @default', array('@default' => $default))
      );
    }
    break;
    
  case 'boolean':
    if (!is_numeric(key($elements))) {
      $elements = array($elements);
    }
    foreach ($elements as $element) {
      $name = $element['@attributes']['name'];
      $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
      $default = (isset($element['@attributes']['selected']) AND $element['@attributes']['selected'] == 'true') ? 1 : 0;
      $description = '';
      if (isset($element['description'])) {
        $description = is_array($element['description']) ? $element['description']['@value'] : $element['description'];
      }
      
      $value = $default;
      $user_pref = $user_prefs;
      $entitytype = isset($element['@attributes']['entitytype']) ? $element['@attributes']['entitytype'] : 'element';
      if ($entitytype == 'attribute') {
        $user_pref = $user_prefs['@attributes'];
      }
      if (isset($user_pref[$name])) {
        if (is_array($user_pref[$name]) AND isset($user_pref[$name]['@value'])) {
          $value = $user_pref[$name]['@value'];
        }
        else {
          $value = $user_pref[$name];
        }
      }
      
      $form[$name] = array(
        '#title' => t($title),
        '#type' => 'radios',
        '#options' => array(1 => t('yes'), 0 => t('no')),
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $value,
        '#description' => t($description)
      );
    }
    break;
    
  case 'radio':
  case 'dropdown':
    
    if (!is_numeric(key($elements))) {
      $elements = array($elements);
    }
    foreach ($elements as $element) {
      $name = $element['@attributes']['name'];
      $default = null;
      $options = array();
      foreach($element['items']['item'] as $item) {
        if (is_array($item)) {
          $value = $item['@value'];
          if ($default === NULL AND
              isset($item['@attributes']) AND
              isset($item['@attributes']['selected'])) {
            $default = ($item['@attributes']['selected'] == 'true') ? $item['@value'] : null;
          }
        }
        else {
          $value = $item;
        }
        $options[$value] = $value;
      }
      $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
      $description = '';
      if (isset($element['description'])) {
        $description = is_array($element['description']) ? $element['description']['@value'] : $element['description'];
      }
      $user_pref = $user_prefs;
      $entitytype = isset($element['@attributes']['entitytype']) ? $element['@attributes']['entitytype'] : 'element';
      if ($entitytype == 'attribute') {
        $user_pref = $user_prefs['@attributes'];
      }
      $value = isset($user_pref[$name]) ? $user_pref[$name] : $default;
      
      $form[$name] = array(
        '#title' => t($title),
        '#type' => ($type == 'radio') ? 'radios' : 'select',
        '#options' => $options,
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $value,
        '#description' => t($description) . t(' Default value: @default', array('@default' => t($default)))
      );
    }
    break;
    
  case 'apps':
    $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
    $form['applications'] = array(
      '#title' => t('Applications'),
      '#type' => 'fieldset',
      '#description' => t($title),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );
    $applications = array();
    if (!is_array($user_prefs['app_id'])) {
      $user_prefs['app_id'] = array($user_prefs['app_id']);
    }
    foreach ($user_prefs['app_id'] as $app) {
      if (!$app) continue;
      if (is_array($app) AND isset($app['@value'])) {
        $app = $app['@value'];
      }
      $applications[] = $app;
    }
    foreach ($elements['app'] as $app) {
      $app_id = $app['@attributes']['id'];
      $app_name = $app['@value'];
      $app_enabled = TRUE;
      if (isset($app['@attributes']['enabled']) AND
          $app['@attributes']['enabled'] == 'false') {
        $app_enabled = FALSE;
      }
      if ($applications) {
        $checked = in_array($app_id, $applications);
      } else {
        $checked = TRUE;
        if (isset($app['@attributes']['selected']) AND
            $app['@attributes']['selected'] == 'false') {
          $checked = FALSE;
        }
      }
      $form['applications']["app_{$app_id}"] = array(
        '#title' => $app_name,
        '#type' => 'checkbox',
        '#default_value' => ($checked) ? 'x' : false,
        '#disabled' => !$app_enabled
      );
    }
    break;
    
  case 'group':
    if (!is_numeric(key($elements))) {
      $elements = array($elements);
    }
    foreach ($elements as $key => $element) {
      $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
      $name = str_replace(' ','_',strtolower($title));
      $name = "group_{$name}";
      $form[$name] = array(
          '#title' => t($title),
          '#type' => 'fieldset',
          //'#description' => t('Notes about this group of fields'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE
      );
      // Recursively populate the compound element
      foreach ($element as $child_type => $child) {
        boincwork_generate_prefs_element($form[$name], $child_type, $child, $user_prefs);
      }
    }
    break;
    
  case 'compound':
    if (!is_numeric(key($elements))) {
      $elements = array($elements);
    }
    foreach ($elements as $element) {
      $name = $element['@attributes']['name'];
      $title = is_array($element['title']) ? $element['title']['@value'] : $element['title'];
      $form[$name] = array(
          '#title' => t($title),
          '#type' => 'fieldset',
          //'#description' => t('Notes about this group of fields'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE
      );
      // Recursively populate the compound element
      foreach ($element['attributes'] as $child_type => $child) {
        boincwork_generate_prefs_element($form[$name], $child_type, $child, $user_prefs[$name]['@attributes']);
      }
    }
    break;
    
    default:
      // Don't generate form elements for things that aren't explicitly form
      // elements (i.e. 'title', '@attributes' keys, and the like)
  }
}

/**
 * Generate a tabular structure out of preferences for use in comparison
 * views of preference sets
 */
function boincwork_make_prefs_table($prefs, $top_level = TRUE) {

  $prefs_table = array();
  $uncategorized = array();
  
  // Parse the project preferences form
  foreach ($prefs as $key => $element) {
    
    // Determine which type of element this is and act accordingly
    $element_type = NULL;
    if (is_array($element) AND isset($element['#type'])) {
      $element_type = $element['#type'];
    }
    switch ($element_type) {
    case 'fieldset':
      $prefs_table[$key] = array(
        'name' => $element['#title'],
        'elements' => boincwork_make_prefs_table($element, FALSE),
      );
      break;
    case 'textfield':
    case 'radios':
    case 'checkbox':
    case 'select':
      // Special hack for time range preferences...
      if (in_array($key, array('start_hour', 'net_start_hour'))) {
        switch ($key) {
        case 'start_hour':
          $element['#title'] = t('Do work only between the hours of:');
          break;
        case 'net_start_hour':
          $element['#title'] = t('Use network only between the hours of:');
          break;
        default:
        }
      }
      $prefs_element = array(
        'name' => (isset($element['#title'])) ? $element['#title'] : '',
        'description' => (isset($element['#description'])) ? $element['#description'] : '',
        'default_value' => (isset($element['#default_value'])) ? $element['#default_value'] : NULL,
      );
      if ($top_level) {
        $uncategorized[$key] = $prefs_element;
      }
      else {
        $prefs_table[$key] = $prefs_element;
      }
      break;
    default:
    }
  }
  
  if ($prefs_table AND $uncategorized) {
    // Throw any settings that don't fit elsewhere into "other"
    $prefs_table['other'] = array(
      'name' => t('Other settings'),
      'elements' => $uncategorized,
    );
  }
  elseif ($uncategorized) {
    // If nothing is categorized, output all prefs under a general "settings"
    $prefs_table['settings'] = array(
      'name' => t('Settings'),
      'elements' => $uncategorized,
    );
  }
  
  return $prefs_table;
}
 
/**
 * Load general or project preferences from BOINC account
 */
function boincwork_load_prefs($type = 'general', $venue = null, $account = null) {
  
  require_boinc(array('user'));
  
  // Load the BOINC user object
  if (!$account) {
    global $user;
    $account = $user;
  }
  $account = user_load($account->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  
  // Load the desired preferences for the user
  $main_prefs = array();
  if ($type == 'project') {
    if ($boincuser->project_prefs) {
      $main_prefs = load_configuration($boincuser->project_prefs);
      $main_prefs = (array) $main_prefs['project_preferences'];
    }
  }
  else {
    if ($boincuser->global_prefs) {
      $main_prefs = load_configuration($boincuser->global_prefs);
      $main_prefs = (array) $main_prefs['global_preferences'];
    }
  }
  
  // Return general preferences or a subset based on venue
  if (!$venue OR $venue == 'generic') {
    unset($main_prefs['venue']);
    return $main_prefs;
  }
  else {
    if (isset($main_prefs['venue'])) {
      if (!is_numeric(key($main_prefs['venue']))) {
        $main_prefs['venue'] = array($main_prefs['venue']);
      }
      foreach ($main_prefs['venue'] as $key => $prefs_venue) {
        if (isset($prefs_venue['@attributes']['name']) AND $prefs_venue['@attributes']['name'] == $venue) {
          return $main_prefs['venue'][$key];
        }
      }
    }
  }
  return array('@attributes' => array(
    'name' => $venue, 'preset' => 'standard', 'cleared' => 1
  ));
}

/**
 * Save project preferences to BOINC account
 */
function boincwork_save_prefs($prefs, $type = 'general', $venue = null, $account = null) {
  
  require_boinc(array('user'));
  
  // Load existing project prefs from the BOINC user object
  if (!$account) {
    global $user;
    $account = $user;
  }
  $account = user_load($account->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  
  // Load the specified preferences for the user
  $main_prefs = array();
  if ($type == 'project') {
    if ($boincuser->project_prefs) {
      $main_prefs = load_configuration($boincuser->project_prefs);
      $main_prefs = (array) $main_prefs['project_preferences'];
    }
  }
  else {
    if ($boincuser->global_prefs) {
      $main_prefs = load_configuration($boincuser->global_prefs);
      $main_prefs = (array) $main_prefs['global_preferences'];
    }
  }
  
  // Save all preferences or a subset based on venue
  //drupal_set_message('<pre>' . print_r($main_prefs, true) . '</pre>');
  $new_venue = true;
  if (!$venue OR $venue == 'generic') {
    //$main_prefs = $prefs;
    $main_prefs = $prefs + $main_prefs;
  }
  else {
    if (isset($main_prefs['venue'])) {
      if (!is_numeric(key($main_prefs['venue']))) {
        $main_prefs['venue'] = array($main_prefs['venue']);
      }
      foreach ($main_prefs['venue'] as $key => $prefs_venue) {
        if (isset($prefs_venue['@attributes']['name']) AND $prefs_venue['@attributes']['name'] == $venue) {
          if ($prefs) {
            $main_prefs['venue'][$key] = $prefs;
          }
          else {
            // If prefs is null, clear out this preference set
            unset($main_prefs['venue'][$key]);
          }
          $new_venue = false;
          break;
        }
      }
    }
    if ($new_venue) {
      $main_prefs['venue'][] = $prefs;
    }
  }
  
  // Set modified time
  if ($type == 'general') {
    if (!isset($main_prefs['mod_time'])) {
      $main_prefs = array_merge(array('mod_time' => 0), $main_prefs);
    }
    $main_prefs['mod_time'] = time();
  }
  
  // Convert prefs back to XML and save to database
  $result = null;
  if ($type == 'project') {
    $main_prefs = array('project_preferences' => $main_prefs);
    $boincuser->project_prefs = save_configuration($main_prefs);
    db_set_active('boinc');
    $result = db_query("UPDATE user SET project_prefs = '{$boincuser->project_prefs}' WHERE id = '{$boincuser->id}'");
    db_set_active('default');
  }
  else {
    $main_prefs = array('global_preferences' => $main_prefs);
    $boincuser->global_prefs = save_configuration($main_prefs);
    db_set_active('boinc');
    $result = db_query("UPDATE user SET global_prefs = '{$boincuser->global_prefs}' WHERE id = '{$boincuser->id}'");
    db_set_active('default');
  }
  return $result;
}



/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Utility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */ 

/**
 * Check that numeric data conforms to specifications
 */
function boincwork_validate_datatype($data, $datatype = NULL) {
  switch ($datatype) {
  case 'float':
    if (!is_numeric($data)) {
      return FALSE;
    }
    $data += 0;
    if (!is_float($data)) {
      return FALSE;
    }
    break;
    
  case 'integer':
    if (!is_numeric($data)) {
      return FALSE;
    }
    $data += 0;
    if (!is_int($data)) {
      return FALSE;
    }
    break;
  
  case 'text':
  default:
    
  }
  return TRUE;
}


  //------------------------------------------------------------------------------------------------
  //  load_configuration(): Convert structured text/xml to array
  //------------------------------------------------------------------------------------------------
  
  function load_configuration($text)
  {
      if (preg_match('/^\<\?xml .*\?\>$/i', $text)) return null;
      if ($xml = text_to_xml($text)) return xml_to_array($xml);
      return false;
  }
  
  //------------------------------------------------------------------------------------------------
  //  save_configuration(): Convert array to structured text/xml
  //------------------------------------------------------------------------------------------------
  
  function save_configuration($array)
  {
      if ($xml = array_to_xml($array)) return xml_to_text($xml, false, true);
      return false;
  }
  
  //------------------------------------------------------------------------------------------------
  //  array_to_xml(): Take a multidimensional array and convert it to a structured
  //  DOM XML object
  //------------------------------------------------------------------------------------------------
  
  function array_to_xml($array, $dom = false, $parent_node = false) {
    $is_root = false;
    if (!$dom) $dom = new DomDocument('1.0');
    if (!$parent_node) {
      $parent_node = $dom;
      $is_root = true;
    }
    // Created an intermediate array to attempt to sort by @position
    $ordered_array = array();
    $unordered_array = array();
    foreach ($array as $name => $value) {
      if ($is_root) {
        $unordered_array[] = $array;
        break;
      }
      if (is_array($value)) {
        if (is_numeric(key($value))) {
          foreach ($value as $item) {
            if (is_array($item) AND isset($item['@position'])) {
              $ordered_array[$item['@position']] = array(
                $name => $item
              );
            }
            else {
              $unordered_array[] = array(
                $name => $item
              );
            }
          }
        }
        elseif (isset($value['@position'])) {
          $ordered_array[$value['@position']] = array(
            $name => $value
          );
        }
        else {
          $unordered_array[] = array(
            $name => $value
          );
        }
      }
      else {
        $unordered_array[] = array(
          $name => $value
        );
      }
    }
    
    // Now append items without explicit positions at the end
    $primed_array = array_merge($ordered_array, $unordered_array);
    
    // Convert to XML...
    foreach ($primed_array as $item) {
      list($name, $value) = each($item);
      if (strcmp($name, '@attributes') == 0) {
        if (!is_array($value)) continue;
        foreach ($value as $attributeName => $attributeValue) {
          $parent_node->setAttribute($attributeName, $attributeValue);
        }
      } elseif (strcmp($name, '@value') == 0) {
        if (isset($value)) $parent_node->nodeValue = $value;
      } elseif (strcmp($name, '@position') == 0) {
        continue;
      } else {
        if (is_numeric($name)) {
          $name = $parent_node->tagName;
        }
        $current_item = $dom->createElement($name);
        if (is_array($value)) {
          if (is_numeric(key($value))) {
            $current_node = $parent_node->appendChild($current_item);
            $current_node = array_to_xml($value, $dom, $current_node);
            $child_count = $current_node->childNodes->length;
            for ($i = 0; $i < $child_count; $i++) {
              $parent_node->appendChild($current_node->childNodes->item(0));
            }
            $parent_node->removeChild($current_node);
          } else {
            $current_node = $dom->appendChild($current_item);
            $parent_node->appendChild(array_to_xml($value, $dom, $current_node));
          }
        } else {
          if (isset($value)) $current_item->nodeValue = $value;
          $parent_node->appendChild($current_item);
        }
      }
    }
    /*
    foreach ($array as $name => $value) {
      if (strcmp($name, '@attributes') == 0) {
        if (!is_array($value)) continue;
        foreach ($value as $attributeName => $attributeValue) {
          $parent_node->setAttribute($attributeName, $attributeValue);
        }
      } elseif (strcmp($name, '@value') == 0) {
        if (isset($value)) $parent_node->nodeValue = $value;
      } else {
        if (is_numeric($name)) {
          $name = $parent_node->tagName;
        }
        $current_item = $dom->createElement($name);
        if (is_array($value)) {
          if (is_numeric(key($value))) {
            $current_node = $parent_node->appendChild($current_item);
            $current_node = array_to_xml($value, $dom, $current_node);
            $child_count = $current_node->childNodes->length;
            for ($i = 0; $i < $child_count; $i++) {
              $parent_node->appendChild($current_node->childNodes->item(0));
            }
            $parent_node->removeChild($current_node);
          } else {
            $current_node = $dom->appendChild($current_item);
            $parent_node->appendChild(array_to_xml($value, $dom, $current_node));
          }
        } else {
          if (isset($value)) $current_item->nodeValue = $value;
          $parent_node->appendChild($current_item);
        }
      }
    }*/
    return $parent_node;
  }
  
  //------------------------------------------------------------------------------------------------
  //  xml_to_text(): Convert an XML DOM object to string format
  //------------------------------------------------------------------------------------------------
  
  function xml_to_text($xml, $include_xml_declaration = true, $add_carriage_returns = false)
  {
      $xml->formatOutput = true;
      $text = $xml->saveXML();
      if (!$include_xml_declaration) {
        $text = preg_replace('/<\?xml version=.*\?>\s*/i', '', $text, 1);
      }
      if ($add_carriage_returns) {;
        $text = preg_replace('/\n/i', "\r\n", $text);
      }
      return trim($text);
  }
  
  //------------------------------------------------------------------------------------------------
  //  text_to_xml(): Convert an XML DOM object to string format
  //------------------------------------------------------------------------------------------------
  
  function text_to_xml($text) {
    if (!$xml = DomDocument::loadXML($text)) return false;
    return $xml;
  }
  
  
  //------------------------------------------------------------------------------------------------
  //  xml_to_array(): Convert an XML DOM object to array format
  //------------------------------------------------------------------------------------------------
  
  function xml_to_array($xml) {
      $node = $xml->firstChild; //$xml->first_child();
      $result = '';
      $index = 1;
      $position = 0;
      while (!is_null($node)) {
          switch ($node->nodeType) {
          case XML_TEXT_NODE:
              if (trim($node->nodeValue)  != '') $result = $node->nodeValue;
              break;
          case XML_ELEMENT_NODE:
              $node_name = $node->nodeName;
              $parent = $node->parentNode;
              $sibling = $node->nextSibling;
              
              // Determine if this node forms a set with siblings (share a node name)
              while (($sibling) AND (($sibling->nodeType != XML_ELEMENT_NODE) OR ($sibling->nodeName != $node->nodeName))) $sibling = $sibling->nextSibling;
              if (!$sibling) {
                  $sibling = $node->previousSibling;
                  while (($sibling) AND (($sibling->nodeType != XML_ELEMENT_NODE) OR ($sibling->nodeName != $node->nodeName))) $sibling = $sibling->previousSibling;
              }
              
              if ($sibling) {
                  $result[$node_name][$index] = '';
                  if ($node->childNodes) {
                      $result[$node_name][$index] = xml_to_array($node) ;
                  }
                  if ($node->hasAttributes()) {
                      $attributes = $node->attributes;
                      if ($result[$node_name][$index] !== '' AND !is_array($result[$node_name][$index])) {
                          $result[$node_name][$index] = array('@value' => $result[$node_name][$index]);
                      }
                      foreach ($attributes as $key => $attribute) {
                          $result[$node_name][$index]['@attributes'][$attribute->name] = $attribute->value;
                      }
                  }
                  // Retain the position of the element
                  if (!is_array($result[$node_name][$index])) {
                    $result[$node_name][$index] = array(
                      '@value' => $result[$node_name][$index]
                    );
                  }
                  $result[$node_name][$index]['@position'] = $position;
                  $position++;
                  $index++;
              } else {
                  $result[$node_name] = '';
                  if ($node->childNodes) {
                      $result[$node_name] = xml_to_array($node) ;
                  }
                  if ($node->hasAttributes()) {
                      $attributes = $node->attributes;
                      if ($result[$node_name] !== '' AND !is_array($result[$node_name])) {
                          $result[$node_name] = array('@value' => $result[$node_name]);
                      }
                      foreach($attributes as $key => $attribute) {
                          $result[$node_name]['@attributes'][$attribute->name] = $attribute->value;
                      }
                  }
                  // Retain the position of the element
                  if (!is_array($result[$node_name])) {
                    $result[$node_name] = array(
                      '@value' => $result[$node_name]
                    );
                  }
                  $result[$node_name]['@position'] = $position;
                  $position++;
              }
              break;
          }
          $node = $node->nextSibling;
      }
      return $result;
  }
  

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Functions for use in displaying special case text in views
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
  * Determine output for host list views when no hosts are found.
  */
function boincwork_views_host_list_empty_text($context = NULL) {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $account = user_load($view->args[0]);
  $boincuser = lookup_user_id($account->boincuser_id);
  
  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    switch($context) {
    case 'active':
      $output .=  '<h2>' . t('No active computers') . '</h2>';
      $output .=  '<p>' . t('This user has no computers that have been active
        in the last 30 days.') . '</p>';
      break;
      
    case 'preferences':
      $output .=  '<h2>' . t('No computers') . '</h2>';
      $output .=  '<p>' . t('There are no computers assigned to this preference
        set.') . '</p>';
      break;
    
    default:
      $output .=  '<h2>' . t('Computers pending') . '</h2>';
      $output .=  '<p>' . t('This user does not yet have any associated
        computers. Computers will be displayed when they have earned their
        first credits.') . '</p>';
    }
  }
  else {
    $output .=  '<h2>' . t('Computers hidden') . '</h2>';
    $output .=  '<p>' . t('This user has chosen not to show information about
      their computers.') . '</p>';
  }
  return $output;
}

/**
  * Determine output for task list views when no tasks are found.
  */
function boincwork_views_task_list_empty_text($context = NULL) {
  
  // 
  $output = '';
  switch($context) {
  default:
    $output .=  '<h2>' . t('No @type tasks', array('@type' => $context)) . '</h2>';
    $output .=  '<p>' . t('There are no tasks of this type on record') . '</p>';
  }
  return $output;
}

/**
  * Determine output for host last contact time
  */
function boincwork_host_last_contact($timestamp, $context = NULL) {
  $output = '---';
  $log_name = 'NO_SUCH_LOG';
  if ($timestamp) {
    $output = gmdate('j M Y | G:i:s', $timestamp) . ' UTC';
    $log_name = gmdate('Y-m-d_H/Y-m-d_H:i', $timestamp);
  }
  if (file_exists(boinc_get_path() . '/html/sched_logs')) {
    $output = "<a href=\"/sched_logs/{$log_name}\">{$output}</a>";
  }
  return $output;
}

/**
  * Determine output for task reported time / deadline
  */
function boincwork_task_time_reported($received_time = NULL, $deadline = NULL, $context = NULL) {
  $output = '---';
  if ($received_time OR $deadline) {
    $timestamp = ($received_time) ? $received_time : $deadline;
    $output = gmdate('j M Y, G:i:s', $timestamp) . ' UTC';
    // Add a wrapper to deadline text
    if (!$received_time) {
      if (time() < $deadline) {
        $output = '<span class="on-time">' . $output . '</span>';
      }
      else {
        $output = '<span class="past-due">' . $output . '</span>';
      }
    }
  }
  return $output;
}
