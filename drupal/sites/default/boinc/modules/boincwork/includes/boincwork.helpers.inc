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
          <disk_max_used_gb>0</disk_max_used_gb>
          <disk_min_free_gb>2</disk_min_free_gb>
          <disk_max_used_pct>0</disk_max_used_pct>
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
        if (isset($prefs['ATTRIBUTES']['name']) AND $prefs['ATTRIBUTES']['name'] == $preset) {
          return $preset_prefs['preset'][$key];
        }
      }
    }
  }
  return $preset_prefs;
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
  if (!$venue OR $venue == 'default') {
    unset($main_prefs['venue']);
    return $main_prefs;
  }
  else {
    if (isset($main_prefs['venue'])) {
      if (!is_numeric(key($main_prefs['venue']))) {
        $main_prefs['venue'] = array($main_prefs['venue']);
      }
      foreach ($main_prefs['venue'] as $key => $prefs_venue) {
        if (isset($prefs_venue['ATTRIBUTES']['name']) AND $prefs_venue['ATTRIBUTES']['name'] == $venue) {
          return $main_prefs['venue'][$key];
        }
      }
    }
  }
  return array('ATTRIBUTES' => array('name' => $venue, 'preset' => 'standard'));
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
  if (!$venue OR $venue == 'default') {
    //$main_prefs = $prefs;
    $main_prefs = $prefs + $main_prefs;
  }
  else {
    if (isset($main_prefs['venue'])) {
      if (!is_numeric(key($main_prefs['venue']))) {
        $main_prefs['venue'] = array($main_prefs['venue']);
      }
      foreach ($main_prefs['venue'] as $key => $prefs_venue) {
        if (isset($prefs_venue['ATTRIBUTES']['name']) AND $prefs_venue['ATTRIBUTES']['name'] == $venue) {
          $main_prefs['venue'][$key] = $prefs;
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
  if (!isset($main_prefs['mod_time'])) {
    $main_prefs = array_merge(array('mod_time' => 0), $main_prefs);
  }
  $main_prefs['mod_time'] = time();
  
  //drupal_set_message('<pre>' . print_r($main_prefs, true) . '</pre>');
  
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
    if (!$dom) $dom = new DomDocument('1.0');
    if (!$parent_node) $parent_node = $dom;
    foreach ($array as $name => $value) {
      if (strcmp($name, 'ATTRIBUTES') == 0) {
        if (!is_array($value)) continue;
        foreach ($value as $attributeName => $attributeValue) {
          $parent_node->setAttribute($attributeName, $attributeValue);
        }
      } elseif (strcmp($name, 'VALUE') == 0) {
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
    }
    return $parent_node;
  }
  
  //------------------------------------------------------------------------------------------------
  //  xml_to_text(): Convert an XML DOM object to string format
  //------------------------------------------------------------------------------------------------
  
  function xml_to_text($xml, $include_xml_declaration = true, $add_carriage_returns = false)
  {
      $text = $xml->saveXML();
      if (!$include_xml_declaration) $text = preg_replace('/<\?xml version=.*\?>\s*/i', '', $text, 1);
      if ($add_carriage_returns) $text = preg_replace('/\n/i', "\r\n", $text);
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
                  if ($attributes = $node->attributes) {
                      foreach( $attributes as $key =>$attribute) {
                          $result[$node_name][$index]['ATTRIBUTES'][$attribute->name] = $attribute->value;
                      }
                  }
                  $index++;
              } else {
                  $result[$node_name] = '';
                  if ($node->childNodes) {
                      $result[$node_name] = xml_to_array($node) ;
                  }
                  if ($attributes = $node->attributes) {
                      foreach($attributes as $key =>$attribute) {
                          $result[$node_name]['ATTRIBUTES'][$attribute->name] = $attribute->value;
                      }
                  }
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
function boincwork_views_host_list_empty_text($context = null) {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $boinc_id = $view->args[0];
  $boincuser = lookup_user_id($boinc_id);
  
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
