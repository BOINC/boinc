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

  // If not configured yet, use these values as for inital
  // computing/general preferences.
  if (!$saved_state) {
    // Get BOINC project disk space configurations from config.xml to
    // fill in initial preference values.
    require_boinc(array('db', 'prefs'));
    $disk_space_config = get_disk_space_config();

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
          <max_ncpus_pct>100</max_ncpus_pct>
          <cpu_usage_limit>80</cpu_usage_limit>
          <disk_max_used_gb>'.$disk_space_config->disk_max_used_gb.'</disk_max_used_gb>
          <disk_min_free_gb>'.$disk_space_config->disk_min_free_gb.'</disk_min_free_gb>
          <disk_max_used_pct>'.$disk_space_config->disk_max_used_pct.'</disk_max_used_pct>
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
        $name = $element['@attributes']['name'];
        $rules[$name] = boincwork_get_project_specific_config_validation_rules($element['attributes']);
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

    case 'group':
      foreach ($elements as $element) {
        $name = $element['@attributes']['name'];
        $rules += boincwork_get_project_specific_config_validation_rules($element);
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
        $default[$name]['@attributes'] = boincwork_format_project_specific_prefs_data($values[$name], $element['attributes']);
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
          if ($values['applications']["app_{$app_id}"]) {
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
          $value = number_format((float) $value, 2);
          break;

        default:
        }
      }

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }
      if ($description) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($description));
        $description = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($description));
      }

      $form[$name] = array(
        '#title' => $title,
        '#type' => 'textfield',
        '#default_value' => $value,
        '#size' => 5,
        '#description' => $description . bts(' Default value: @default', array('@default' => $default), NULL, 'boinc:account-preferences-project')
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

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }
      if ($description) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($description));
        $description = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($description));
      }

      $form[$name] = array(
        '#title' => $title,
        '#type' => 'radios',
        '#options' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $value,
        '#description' => $description
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

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }
      if ($description) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($description));
        $description = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($description));
      }

      $form[$name] = array(
        '#title' => $title,
        '#type' => ($type == 'radio') ? 'radios' : 'select',
        '#options' => $options,
        '#attributes' => array('class' => 'fancy'),
        '#default_value' => $value,
        '#description' => $description . bts(' Default value: @default', array('@default' =>$default), NULL, 'boinc:account-preferences-project')
      );
    }
    break;

  case 'apps':
    $title = is_array($elements['title']) ? $elements['title']['@value'] : $elements['title'];

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }

    $form['applications'] = array(
      '#title' => bts('Applications', array(), NULL, 'boinc:account-preferences'),
      '#type' => 'fieldset',
      '#description' => $title,
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

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }

      $form[$name] = array(
          '#title' => $title,
          '#type' => 'fieldset',
          '#tree' => FALSE,
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

      // Translate elements as appropriate
      if ($title) {
        i18nstrings_update('project:prefs_xml', _boinctranslate_supertrim($title));
        $title = i18nstrings('project:prefs_xml', _boinctranslate_supertrim($title));
      }

      $form[$name] = array(
          '#title' => $title,
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
          $element['#title'] = bts('Compute only between:', array(), NULL, 'boinc:account-preferences-computing');
          break;
        case 'net_start_hour':
          $element['#title'] = bts('Transfer files only between:', array(), NULL, 'boinc:account-preferences-comuting');
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
      'name' => bts('Other settings', array(), NULL, 'boinc:account-preferences'),
      'elements' => $uncategorized,
    );
  }
  elseif ($uncategorized) {
    // If nothing is categorized, output all prefs under a general "settings"
    $prefs_table['settings'] = array(
      'name' => bts('Settings', array(), NULL, 'boinc:account-preferences'),
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
    // Use the length of the $main_prefs array as a condition as to
    // whether the preferences have already been set. This is
    // HARDCODED here.
    if (count($main_prefs) < 3)
        $main_prefs['@attributes'] = array('cleared' => 1);
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

  return array(
    '@attributes' => array('name' => $venue, 'cleared' => 1)
  );
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
            if (count($main_prefs['venue']) == 0) {
              // If that was the only preference set configured, unset the
              // venue tag altogether
              unset($main_prefs['venue']);
            }
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
  if ($type == 'project') {
    if (!isset($main_prefs['modified'])) {
      $main_prefs = array_merge(array('modified' => 0), $main_prefs);
    }
    $main_prefs['modified'] = time();
  } else {
    if (!isset($main_prefs['mod_time'])) {
      $main_prefs = array_merge(array('mod_time' => 0), $main_prefs);
    }
    $main_prefs['mod_time'] = time();
    // unset source information, the Client will fill this in again
    if (isset($main_prefs['source_project'])) {
      unset($main_prefs['source_project']);
    }
    if (isset($main_prefs['source_scheduler'])) {
      unset($main_prefs['source_scheduler']);
    }
  }

  // Convert prefs back to XML and save to database
  $result = null;
  if ($type == 'project') {
    $main_prefs = array('project_preferences' => $main_prefs);
    $boincuser->project_prefs = save_configuration($main_prefs);
    db_set_active('boinc_rw');
    $result = db_query("UPDATE user SET project_prefs = '{$boincuser->project_prefs}' WHERE id = '{$boincuser->id}'");
    db_set_active('default');
  }
  else {
    $main_prefs = array('global_preferences' => $main_prefs);
    $boincuser->global_prefs = save_configuration($main_prefs);
    db_set_active('boinc_rw');
    $result = db_query("UPDATE user SET global_prefs = '{$boincuser->global_prefs}' WHERE id = '{$boincuser->id}'");
    db_set_active('default');
  }
  return $result;
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Ignore user functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

// These functions are boinc-ppecific "replacements" for the
// ignore_user module's add and remove user functions.


/**
 *Add another user to the current user's ignore list using a BOINC
 *username. Called from privacy preference form.
 */
function boincwork_ignore_user_add_user_username($name = NULL) {
  global $user;

  if (isset($name)) {
    // Get the BOINC ID from the name string, and lookup the
    // corresponding drupal user.
    $boincname = substr($name, 0, strrpos($name, '_'));
    $boincid = substr($name, strrpos($name, '_') + 1);
    // iuid is the drupal uid to be ignored and author to be blocked.
    $iuid = get_drupal_id($boincid);

    if ($user->uid == $iuid) {
      drupal_set_message(bts('You can\'t add yourself to your own ignore list.', array(), NULL, 'boinc:ignore-user-error-message'), 'error');
      drupal_goto('account/prefs/privacy');
    }

    if (is_numeric($iuid) && $iuid > 0) {
      if (!db_result(db_query('SELECT COUNT(iuid) FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $iuid))) {
        db_query('INSERT INTO {ignore_user} (uid, iuid) VALUES (%d, %d)', $user->uid, $iuid);
      }// endif db_result

      if (module_exists('pm_block_user')) {
        if (!db_result(db_query('SELECT COUNT(recipient) FROM {pm_block_user} WHERE author = %d AND recipient = %d', $iuid, $user->uid))) {
          db_query('INSERT INTO {pm_block_user} (author, recipient) VALUES (%d, %d)', $iuid, $user->uid);
        }// endif db_result
      }// endif module_exists('pm_block_user')

    }// endif $iuid
  }
}

/**
 * Add another user's UID to the current user's ignore list.
 */
function boincwork_ignore_user_add_user($iuid = NULL) {
  global $user;

  if ($user->uid == $iuid) {
    drupal_set_message(bts('You can\'t add yourself to your own ignore list.', array(), NULL, 'boinc:ignore-user-error-message'), 'error');
    drupal_goto();
  }

  $otheraccount = user_load($iuid);

  if (is_numeric($iuid) && $iuid > 0) {
    if (!db_result(db_query('SELECT COUNT(iuid) FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $iuid))) {
      db_query('INSERT INTO {ignore_user} (uid, iuid) VALUES (%d, %d)', $user->uid, $iuid);
    }// endif db_result

    if (module_exists('pm_block_user')) {
      if (!db_result(db_query('SELECT COUNT(recipient) FROM {pm_block_user} WHERE author = %d AND recipient = %d', $iuid, $user->uid))) {
        db_query('INSERT INTO {pm_block_user} (author, recipient) VALUES (%d, %d)', $iuid, $user->uid);
      }// endif db_result
    }// endif module_exists('pm_block_user')

    drupal_set_message(
      bts('@username has been added to your ignore list. See your !privacy_preferences for more details.',
        array(
          '@username' => $otheraccount->boincuser_name,
          '!privacy_preferences' => l(bts('privacy preferences', array(), NULL, 'boinc:ignore-user-add'), 'account/prefs/privacy'),
        ),
        NULL, 'boinc:ignore-user-add'),
      'status');
    drupal_goto();
  }
  else {
    drupal_not_found();
  }// endif iuid
}

/**
 * Remove user from user's ignore list.
 */
function boincwork_ignore_user_remove_user($iuid = NULL) {
  global $user;
  $otheraccount = user_load($iuid);

  db_query('DELETE FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $iuid);
  if (module_exists('pm_block_user')) {
    db_query('DELETE FROM {pm_block_user} WHERE author = %d AND recipient = %d', $iuid, $user->uid);
  }// endif module_exists
  drupal_set_message(
    bts('@username has been removed from your ignore list. See your !privacy_preferences for more details.',
      array(
        '@username' => $otheraccount->boincuser_name,
        '!privacy_preferences' => l(bts('privacy preferences', array(), NULL, 'boinc:ignore-user-add'), 'account/prefs/privacy'),
      ),
      NULL, 'boinc:ignore-user-add'),
    'status');
  drupal_goto('account/prefs/privacy');
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Consent to Privacy funtions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Load consent types for privacy preferences from database. Returns
 * an array of the relevant consent_types. Otherwise returns an emtpy
 * array.
 */
function boincwork_load_privacyconsenttypes() {

  db_set_active('boinc_rw');
  $db_result = db_query("SELECT id,shortname,description FROM consent_type WHERE enabled=1 AND privacypref=1 ORDER by project_specific ASC");
  db_set_active('drupal');

  if ($db_result) {
    $consent_types = array();
    while ($result = db_fetch_array($db_result)) {
      $consent_types[] = $result;
    }
    return $consent_types;
  }
  return array();
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Utility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Set the default preference set for the user
 */
function boincwork_set_default_venue($venue = '') {

  global $user;
  $account = user_load($user->uid);

  if ($venue == 'generic') {
    $venue = '';
  }

  db_set_active('boinc_rw');
  db_query("
    UPDATE user
    SET venue = '%s'
    WHERE id = %d",
    $venue, $account->boincuser_id
  );
  db_set_active('default');
}

/**
 * Recursively validate submitted form values against a set of rules
 */
function boincwork_validate_form($validation_rules, $values, $path = array()) {
  foreach ($validation_rules as $field => $rules) {
    $parents = $path;
    if (is_array($values[$field])) {
      // Process nested form elements
      $parents[] = $field;
      boincwork_validate_form($rules, $values[$field], $parents);
    }
    else {
      if ($parents) {
        // form_set_error() identifies nested form elements with '][' as a
        // delimiter between each parent and child element
        $parents[] = $field;
        $form_field = implode('][', $parents);
      }
      else {
        $form_field = $field;
      }
      if (isset($rules['datatype']) AND !boincwork_validate_datatype($values[$field], $rules['datatype'])) {
        form_set_error($form_field, bts('Invalid data type for @field', array('@field' => $field), NULL, 'boinc:account-preferences'));
      }
      if (isset($rules['min']) AND $values[$field] < $rules['min']) {
        form_set_error($form_field, bts('Minimum value not met for @field', array('@field' => $field), NULL, 'boinc:account-preferences'));
      }
      if (isset($rules['max']) AND $values[$field] > $rules['max']) {
        form_set_error($form_field, bts('Maximum value exceeded for @field', array('@field' => $field), NULL, 'boinc:account-preferences'));
      }
    }
  }
}

/**
 * Check that numeric data conforms to specifications
 */
function boincwork_validate_datatype($data, $datatype = NULL) {
  switch ($datatype) {
  case 'float':
    if (!is_numeric($data)) {
      return FALSE;
    }
    $data += 0.0;
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

/**
 * Format a number to be displayed using a maximum number of digits
 */
function boincwork_format_stats($number, $max_digits = 4) {
  $suffix = array(
    0 => '',
    1 => 'k',
    2 => 'M',
    3 => 'G',
    4 => 'T',
    5 => 'P',
    6 => 'E',
    7 => 'Z',
    8 => 'Y'
  );
  if (!is_numeric($number)) $number = 0;

  $digits = floor(log($number, 10)) + 1;
  $magnitude = 0;
  $precision = 0;
  if ($digits > $max_digits) {
    $magnitude = floor(($digits - ($max_digits - 3)) / 3);
    $precision = $max_digits - ($digits - ($magnitude * 3) + 1);
    $number = round($number / pow(1000, $magnitude), $precision);
  }
  $number = number_format($number, $precision) . (($magnitude) ? "{$suffix[$magnitude]}" : '');

  return $number;
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
    $xml = new DomDocument();
    if ( !($xml->loadXML($text)) ) return false;
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
  require_boinc('boinc_db');
  $view = views_get_current_view();
  $account = user_load($view->args[0]);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);

  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    switch($context) {
    case 'active':
      $output .=  '<h2>' . bts('No active computers', array(), NULL, 'boinc:host-list') . '</h2>';
      $output .=  '<p>' . bts('This user has no computers that have been'
      . ' active in the last 30 days.', array(), NULL, 'boinc:host-list') . '</p>';
      break;

    case 'preferences':
      $output .=  '<h2>' . bts('No computers', array(), NULL, 'boinc:host-list') . '</h2>';
      $output .=  '<p>' . bts('There are no computers assigned to this'
      . ' preference set.', array(), NULL, 'boinc:host-list') . '</p>';
      break;

    default:
      $output .=  '<h2>' . bts('Computers pending', array(), NULL, 'boinc:host-list') . '</h2>';
      $output .=  '<p>' . bts('This user does not yet have any associated'
      . ' computers. Computers will be displayed when they have earned their'
      . ' first credits.', array(), NULL, 'boinc:host-list') . '</p>';
    }
  }
  else {
    $output .=  '<h2>' . bts('Computers hidden', array(), NULL, 'boinc:host-list') . '</h2>';
    $output .=  '<p>' . bts('This user has chosen not to show information'
    . ' about their computers.', array(), NULL, 'boinc:host-list') . '</p>';
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
    $output .=  '<h2>' . bts('No @type tasks', array('@type' => $context), NULL, 'boinc:task-list')
    . '</h2>';
    $output .=  '<p>' . bts('There are no tasks of this type on record', array(), NULL, 'boinc:task-list')
    . '</p>';
  }
  return $output;
}

/**
  * Output links to perform host actions
  */
function boincwork_host_action_links($host_id) {
  $output = '';
  if (boincwork_host_user_is_owner($host_id)) {
    // Show merge hosts option
    $output = '<ul class="tab-list"><li class="first tab">';
    $output .= l(bts('Merge', array(), NULL, 'boinc:form-merge'), "host/{$host_id}/merge");
    $output .= '</li>';
    // If host has no tasks, allow the host to be deleted
    if (!boincwork_host_get_task_count($host_id)) {
      $output .= '<li class="tab">';
      $output .= l(bts('Delete', array(), NULL, 'boinc:form-delete'), "host/{$host_id}/delete",
        array(
          'attributes' => array(
            'onclick' => 'return confirm(\'' . bts('This will delete host @id'
              . ' from your account forever. Are you sure this is OK?',
              array('@id' => $host_id),
              NULL, 'boinc:account-host-delete') . '\')'
          )
        )
      );
      $output .= '</li>';
    }
    $output .= '</ul>';
  }
  return $output;
}

/**
 * Get details for a given host
 */
function boincwork_host_get_info($host_id) {
  db_set_active('boinc_ro');
  $host = db_fetch_object(db_query(
    "SELECT * FROM {host} WHERE id = '%d'",
    $host_id
  ));
  db_set_active('default');
  return $host;
}

/**
 * Get the number of tasks associated with a given host
 */
function boincwork_host_get_task_count($host_id) {
  db_set_active('boinc_ro');
  $count = db_result(db_query(
    "SELECT COUNT(*) FROM {result} WHERE hostid = '%d'",
    $host_id
  ));
  db_set_active('default');
  return $count;
}

/**
 * Check whether a user is the owner of a host
 */
function boincwork_host_user_is_owner($host_id, $uid = NULL) {
  if (!$uid) {
    global $user;
    $uid = $user->uid;
  }
  $account = user_load($uid);
  // Get host owner
  db_set_active('boinc_ro');
  $owner = db_result(db_query(
    "SELECT userid FROM {host} WHERE id = '%d'",
    $host_id
  ));
  db_set_active('default');
  return ($account->boincuser_id === $owner);
}

/**
  * Determine output for host last contact time
  */
function boincwork_host_last_contact($timestamp, $host_id = NULL, $context = NULL) {
  $output = '---';
  $root_log_dir = variable_get('boinc_host_sched_logs_dir', '');
  $log = '';
  if ($timestamp) {
    $output = date('j M Y G:i:s T', $timestamp);
  }
  if ($root_log_dir AND $host_id) {
    $subdir = substr($host_id, 0, -3) OR $subdir = 0;
    $log = implode('/', array($root_log_dir, $subdir, $host_id));
  }
  if ($log AND file_exists($log)) {
    $output = l($output, "host/{$host_id}/log");
  }
  return $output;
}

/**
 *
 */
function boincwork_host_venue_selector($host_id) {
  $output = '';
  if (function_exists('jump_quickly')) {
    $path = "host/{$host_id}/set-venue";
    $venues = array(
      "{$path}/generic" => bts('Generic', array(), NULL, 'boinc:account-preferences-location'),
      "{$path}/home" => bts('Home', array(), NULL, 'boinc:account-preferences-location:-1:ignoreoverwrite'),
      "{$path}/work" => bts('Work', array(), NULL, 'boinc:account-preferences-location'),
      "{$path}/school" => bts('School', array(), NULL, 'boinc:account-preferences-location')
    );
    variable_set('jump_use_js_venues-Array', 1);
    drupal_add_js(drupal_get_path('module', 'jump') . '/jump.js');
    drupal_add_js(drupal_get_path('theme', 'boinc') . '/js/prefs.js', 'theme');
    // Get current venue
    db_set_active('boinc_ro');
    $venue = db_result(db_query(
      "SELECT venue FROM {host} WHERE id = '%d'",
      $host_id
    ));
    db_set_active('default');
    $output .= jump_quickly($venues, 'venues', 1, "{$path}/{$venue}");
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
    $output = date('j M Y G:i:s T', $timestamp);
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

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Functions for creating a task table for users, hosts, and workunits
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * boincwork_tasktable
 *
 * Given two DB results, generates a table of tasks with a navigation
 * header, and application filter.
 *
 * Parameters
 *
 * @param int $category
 *   An integer specifying the category of task table to display: 0 =>
 *   account, 1=> workunit, 2=> host. These are hardcoded.
 * @param int $queryid
 *   The id of the user, host, or workunit to query. Will be added to
 *   the SQL query.
 * @param int $tselect
 *   The task selection state chosen by the user. 0 is for all states,
 *   other states are defined in html/inc/result.inc
 * @param int $app_id
 *   Application id of the application chosen by user. 0 and -1 are
 *   for all applications.
 * @param int $tablerows
 *   Number of table row to display per page. Defaults to 20.
 */
function boincwork_tasktable($category = 0, $queryid = 1, $tselect = NULL, $app_id = 0, $tablerows = 20) {
  // Check type parameter, if not (0,2) then return an error.
  if ( ($category!=0) AND ($category!=1) AND ($category!=2) ) {
    watchdog('boincwork', 'task table called with invalid category = %category', array('%category' => $category), WATCHDOG_WARNING);
    return '';
  }

  require_boinc( array('util', 'result') );

  global $language;
  $locality=$language->language;
  $nf = new NumberFormatter($locality, NumberFormatter::DECIMAL);
  $nf->setAttribute(NumberFormatter::MIN_FRACTION_DIGITS, 0);
  $nf->setAttribute(NumberFormatter::MAX_FRACTION_DIGITS, 0);

  $output = '';

  $state_hnames = array(
    STATE_ALL => 'All',
    STATE_IN_PROGRESS => 'In progress',
    STATE_PENDING => 'Pending',
    STATE_VALID => 'Valid',
    STATE_INVALID => 'Invalid',
    STATE_ERROR => 'Error',
  );
  // Array (hash) to count total number of results/tasks, and their states.
  $taskstates = array(
    STATE_ALL => 0,
    STATE_IN_PROGRESS => 0,
    STATE_PENDING => 0,
    STATE_VALID => 0,
    STATE_INVALID => 0,
    STATE_ERROR => 0,
  );
  // Array to hold pretty-print result data to be displayed in a table.
  $resultdata = array();
  // Arrays for applications. Form below uses $applications as parameter.
  $application_map = array();
  $application_select_count = array();
  $applications = array();

  // BOINC DB queries for results, application names
  db_set_active('boinc_ro');

  // Query to retreive all results, in order to calculate status for a
  // host with application names for each result.
  $sqlall = "SELECT user_friendly_name,"
      ."r.appid as appid,"
      ."r.server_state AS server_state,"
      ."r.outcome AS outcome,"
      ."r.client_state AS client_state,"
      ."r.validate_state AS validate_state,"
      ."r.exit_status AS exit_status "
      ."FROM {result} AS r "
      ."INNER JOIN {app} AS a "
      ."ON r.appid=a.id ";

  // Use userid, hostid, or workunitid
  if ($category==0) {
    $sqlall .= " WHERE r.userid='%s' ";
  }
  elseif ($category==1) {
    $sqlall .= " WHERE r.workunitid='%s' ";
  }
  elseif ($category==2) {
    $sqlall .= " WHERE r.hostid='%s' ";
  }
  $sqlall .= " ORDER BY user_friendly_name";
  $dbres_all = db_query($sqlall, $queryid);
  db_set_active('default');

  // Loop 1 of DB results
  if ($dbres_all) {
    while ($result = db_fetch_object($dbres_all)) {
      $mystate = state_num($result);
      if ( ($result->appid==$app_id) OR ($app_id==0) OR ($app_id==-1) ) {
        $taskstates[STATE_ALL]++;
        switch ($mystate) {
        case STATE_IN_PROGRESS:
          $taskstates[STATE_IN_PROGRESS]++;
          break;
        case STATE_PENDING:
          $taskstates[STATE_PENDING]++;
          break;
        case STATE_VALID:
          $taskstates[STATE_VALID]++;
          break;
        case STATE_INVALID:
          $taskstates[STATE_INVALID]++;
          break;
        case STATE_ERROR:
          $taskstates[STATE_ERROR]++;
          break;
        }
      }// if app_id

      //map holds a map between app ids and user friendly names for all applications.
      $application_map[$result->appid] = $result->user_friendly_name;
      if ( ($mystate == $tselect) OR ($tselect==STATE_ALL) ) {
        //count of appids in the results.
        $application_select_count[$result->appid]++;
      }// if mystate

    }// while
  }
  else {
  }

  // Entry for all applications.
  $allcount = $application_select_count ? array_sum($application_select_count) : 0;
  $applications[-1] = bts('Application', array(), NULL, 'boinc:task-table');
  $applications[0] = bts('All applications', array(), NULL, 'boinc:task-table') . ' (' . $allcount . ')';
  // Create application filter from application_map and application_select_count.
  foreach($application_map as $akey => $aname) {
    $acount = 0;
    if ( $application_select_count and array_key_exists($akey, $application_select_count) ) {
      $acount = $application_select_count[$akey];
    }
    $applications[$akey] = $aname . ' ('. $acount . ')';
  }
  // Header array for (sub) results table.
  $resultheader = array(
    array(
      'data' => bts('Task ID', array(), NULL, 'boinc:task-table'),
      'field' => 'id',
    ),
    array(
      'data' => bts('Workunit ID', array(), NULL, 'boinc:task-table'),
      'field' => 'workunitid',
    ),
    array(
      'data' => bts('Computer', array(), NULL, 'boinc:task-table'),
      'field' => 'hostid',
    ),
    array(
      'data' => bts('Sent', array(), NULL, 'boinc:task-table'),
      'field' => 'sent_time',
    ),
    array(
      'data' => bts('Time Reported or Deadline', array(), NULL, 'boinc:task-table')
    ),
    array(
      'data' => bts('Status', array(), NULL, 'boinc:task-table')
    ),
    array(
      'data' => bts('Run time', array(), NULL, 'boinc:task-table'),
      'field' => 'elapsed_time',
    ),
    array(
      'data' => bts('CPU time', array(), NULL, 'boinc:task-table'),
      'field' => 'cpu_time',
    ),
    array(
      'data' => bts('Granted Credit', array(), NULL, 'boinc:task-table'),
      'field' => 'granted_credit',
    ),
    // Application is a column, but won't be added until after tablesort_sql().
  );

  // Query to retreive a subset of the total results for the results table.
  db_set_active('boinc_ro');
  $sqlsub = "SELECT r.id AS id,"
      ."r.name AS name,"
      ."r.workunitid AS workunitid,"
      ."r.hostid as hostid,"
      ."r.sent_time AS sent_time,"
      ."r.received_time AS received_time,"
      ."r.report_deadline AS report_deadline,"
      ."r.server_state AS server_state,"
      ."r.outcome AS outcome,"
      ."r.client_state AS client_state,"
      ."r.validate_state AS validate_state,"
      ."r.exit_status AS exit_status,"
      ."r.elapsed_time AS elapsed_time,"
      ."r.cpu_time AS cpu_time,"
      ."r.granted_credit AS granted_credit,"
      ."r.appid AS appid,"
      ."r.app_version_id AS app_version_id,"
      ."a.user_friendly_name,"
      ."av.version_num AS version_number,"
      ."av.plan_class AS plan_class,"
      ."pl.name AS platform "
      ."FROM {result} AS r "
      ."INNER JOIN {app} AS a "
      ."ON r.appid=a.id "
      ."LEFT JOIN {app_version} AS av "
      ."ON r.app_version_id=av.id "
      ."LEFT JOIN {platform} AS pl "
      ."ON av.platformid=pl.id ";

  // Build an array for the WHERE clauses. The individual clauses are
  // placed into an array, and implode() is used to combine them into
  // a WHERE statement for the sql query. If no such clauses are added
  // to the array, then no WHERE statement will be included.
  $sqlwhere = array();

  // Use userid, hostid, or workunitid
  if ($category==0) {
    $sqlwhere[] = "r.userid = '%s'";
  }
  elseif ($category==1) {
    $sqlwhere[] = "r.workunitid = '%s'";
  }
  elseif ($category==2) {
    $sqlwhere[] = "r.hostid = '%s'";
  }

  // Append additional where clauses based on task selection.
  switch ($tselect) {
  case STATE_IN_PROGRESS:
    $sqlwhere[] = "r.server_state = 4";
    break;
  case STATE_PENDING:
    $sqlwhere[] = "(r.server_state = 5) AND (r.outcome = 1) AND (r.validate_state >= 0) AND (r.validate_state <= 0 OR r.validate_state >= 4) AND (r.validate_state <= 4)";
    break;
  case STATE_VALID:
    $sqlwhere[] = "(r.server_state = 5) AND (r.outcome = 1) AND (r.validate_state = 1)";
    break;
  case STATE_INVALID:
    $sqlwhere[] = "(r.server_state = 5) AND ((r.outcome = 6) OR ((r.outcome = 1) AND ((r.validate_state = 2) OR (r.validate_state = 3) OR (r.validate_state = 5))))";
    break;
  case STATE_ERROR:
    $sqlwhere[] = "(r.server_state = 5) AND (r.outcome >= 3) AND (r.outcome <= 4 OR r.outcome >= 7) AND (r.outcome <= 7)";
    break;
  default:
  }

  if (is_numeric($app_id) AND $app_id>0 ) {
    $sqlwhere[] = "r.appid = '%s'";
    if ($sqlwhere)  $sqlsub .= " WHERE " . implode(' AND ', $sqlwhere);
    $dbres_sub = pager_query( $sqlsub . tablesort_sql($resultheader), $tablerows, 0, NULL, $queryid, $app_id);
  }
  else {
    if ($sqlwhere)  $sqlsub .= " WHERE " . implode(' AND ', $sqlwhere);
    $dbres_sub = pager_query( $sqlsub . tablesort_sql($resultheader), $tablerows, 0, NULL, $queryid);
  }
  db_set_active('default');

  // Loop 2 over DB results.
  if ($dbres_sub) {
    while ($result = db_fetch_object($dbres_sub)) {
      // state_num() function changes $result object, clone $result
      // object for use in state_num()
      // check if state matches selection
      if ( (state_num(clone $result) == $tselect) OR ($tselect==STATE_ALL) ) {
        // create pretty result row
        $prettyresult = array(
          array(
            'data' => l($result->name, "task/{$result->id}"),
            'class' => 'task-name',
          ),
          l($result->workunitid, "workunit/{$result->workunitid}"),
          l($result->hostid, "host/{$result->hostid}"),
          date('j M Y G:i:s T', $result->sent_time),
          boincwork_task_time_reported($result->received_time, $result->report_deadline),
          state_string($result),
          $nf->format($result->elapsed_time),
          $nf->format($result->cpu_time),
          $nf->format($result->granted_credit),
          array(
            'data' => $result->user_friendly_name . " " . pretty_application_version($result->app_version_id,$result->version_number, $result->plan_class, $result->platform),
            'class' => 'task-app',
          ),
        );
        $resultdata[] = array_values($prettyresult);
      }
    }// while
  }
  else {
  }
  // Begin result navigation

  // Set pathprefix based on type
  if ($category==0) {
    $pathprefix = 'account/tasks';
  }
  elseif ($category==1) {
    $pathprefix = 'workunit/' . $queryid . '/tasks';
  }
  elseif ($category==2) {
    $pathprefix = 'host/' . $queryid . '/tasks';
  }
  // Need an "All" tab as well, maps to app_id of zero.
  $application_map[0] = bts('All', array(), NULL, 'boinc:task-table');
  $stitems = array();
  foreach ($taskstates as $state => $numstates) {
    $mypath = $pathprefix . '/' . $state . '/' . $app_id;
    if ($state==STATE_ALL) {
      $ltext = '<span class="tab task-app-name">' . bts('All', array(), NULL, 'boinc:task-table') . ' (' . $numstates . ')</span>';
    }
    else {
      $ltext = '<span class="tab">' . bts($state_hnames[$state], array(), NULL, 'boinc:task-table') . ' (' . $numstates . ')</span>';
    }
    $myitem = array(
      'data' => l($ltext, $mypath, array('html' => TRUE) ),
    );
    if ($state==$tselect) {
      $myitem['class'] = 'active';
    }
    $stitems[] = $myitem;
  }
  // Add reset button
  $mypath = $pathprefix . '/0/0';
  $ltext = '<span class="tab">' . bts('Reset', array(), NULL, 'boinc:task-table') . '</span>';
  $stitems[] = array( 'data' => l($ltext, $mypath, array('html' => TRUE) ) );

  $output .= theme_item_list($stitems, NULL, 'ul' . ' class="tabs secondary clearfix"');

  // Application select-drop down form
  // Hack to place Application form into header
  // App ID of zero maps to "-1" for drop-down box.
  if ($app_id==0) {
    $app_id=-1;
  }
  $resultheader[] = drupal_get_form('boincwork_selectapp_form', $applications, $app_id);

  // Begin table of results
  if ( is_array($resultheader) AND is_array($resultdata) ) {

    // Take advantage of the fact that $category is the same as the row/column we want to remove.
    if ( ($category==1) OR ($category==2) ) {
      unset($resultheader[$category]);
      delete_col($resultdata, $category);
    }

    $output .= theme_table($resultheader, $resultdata);
    if (count($resultdata) > 0) {
      $output .= theme('pager');
    }
  }
  return $output;
}

/**
 * Function to delete a column from an array.
 */
function delete_col(&$array, $offset) {
  return array_walk($array, function (&$v) use ($offset) {
     array_splice($v, $offset, 1);
  });
}

/**
 * Pretty print the application version
 *
 * Parameters are obtained from the BOINC project database, result,
 * app, app_version, and platform tables.
 *
 * Parameters
 * @param int $appverid
 *  app_version_id field, may be negative- anonymous platforms.
 * @param int $vernum
 *  version_num field, application version number, may be NULL
 * @param int $plan_class
 *  plan_class field, applcation plan class, may be NULL
 * @param string $plfm
 *  platform name, may be NULL
 */
function pretty_application_version($appverid, $vernum, $plan_class, $plfm) {
  switch ($appverid) {
    case ANON_PLATFORM_UNKNOWN:
      return "Anonymous platform";
    case ANON_PLATFORM_CPU:
      return "Anonymous platform CPU";
    case ANON_PLATFORM_NVIDIA:
      return "Anonymous platform NVIDIA GPU";
    case ANON_PLATFORM_ATI:
      return "Anonymous platform AMD GPU";
    case ANON_PLATFORM_INTEL_GPU:
      return "Anonymous platform Intel GPU";
    case ANON_PLATFORM_APPLE_GPU:
      return "Anonymous platform Apple GPU";
    case 0:
      return "---";
    default:
      // Handle the case where the appversid is still negative. This
      // may be cause BOINC has introduced a new anonymous platform
      // that is not handled by the above case statements.
      if ($appverid < 0) {
        return "Unknown Anonymous platform";
      }
      else {
        $prettyv = sprintf("%d.%02d", $vernum/100, $vernum%100);
        $prettyc = ($plan_class) ? "($av->plan_class)" : '';
        return "v$prettyv $prettyc $plfm";
      }
  }
}
