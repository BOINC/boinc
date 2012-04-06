<?php
// $Id$

/**
 * Functions that are shared amongst files and dependent modules go
 * here to keep the clutter down in the main module file.
 */ 


/**
 * Load project preferences from BOINC account
 */
function boincwork_load_project_prefs($venue = null, $account = null) {
  
  require_boinc(array('user'));
  
  // Load project prefs from the BOINC user object
  if (!$account) {
    global $user;
    $account = $user;
  }
  $account = user_load($account->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  if ($boincuser->project_prefs) {
    $main_prefs = load_configuration($boincuser->project_prefs);
    $main_prefs = (array) $main_prefs['project_preferences'];
  }
  else {
    $main_prefs = array();
  }
  
  // Return all preferences or a subset based on venue
  if (!$venue OR $venue == 'default') {
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
  return array('ATTRIBUTES' => array('name' => $venue));
}

/**
 * Save project preferences to BOINC account
 */
function boincwork_save_project_prefs($prefs, $venue = null, $account = null) {
  
  require_boinc(array('user'));
  
  // Load existing project prefs from the BOINC user object
  if (!$account) {
    global $user;
    $account = $user;
  }
  $account = user_load($account->uid);
  $boincuser = BoincUser::lookup_id($account->boincuser_id);
  if ($boincuser->project_prefs) {
    $main_prefs = load_configuration($boincuser->project_prefs);
    $main_prefs = (array) $main_prefs['project_preferences'];
  }
  else {
    $main_prefs = array();
  }
  
  // Save all preferences or a subset based on venue
  $new_venue = true;
  if (!$venue OR $venue == 'default') {
    $main_prefs = $prefs;
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
  
  $main_prefs = array('project_preferences' => $main_prefs);
  $boincuser->project_prefs = save_configuration($main_prefs);
  
  db_set_active('boinc');
  db_query("UPDATE user SET project_prefs = '{$boincuser->project_prefs}' WHERE id = '{$boincuser->id}'");
  db_set_active('default');
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
function views_host_list_empty_text($context = null) {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $boinc_id = $view->args[0];
  $boincuser = lookup_user_id($boinc_id);
  
  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    if ($context == 'active') {
      $output .=  '<h2>' . t('No active computers') . '</h2>';
      $output .=  '<p>' . t('This user has no computers that have been active in the last 30 days.') . '</p>';
    }
    else {
      $output .=  '<h2>' . t('Computers pending') . '</h2>';
      $output .=  '<p>' . t('This user does not yet have any associated computers. Computers will be displayed when they have earned their first credits.') . '</p>';
    }
  }
  else {
    $output .=  '<h2>' . t('Computers hidden') . '</h2>';
    $output .=  '<p>' . t('This user has chosen not to show information about their computers.') . '</p>';
  }
  return $output;
}

/**
  * Determine output for active host list views when no hosts are found.
  */
function views_active_host_list_empty_text() {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $boinc_id = $view->args[0];
  $boincuser = lookup_user_id($boinc_id);
  
  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    $output .= '<h2>' . t('No active computers found') . '</h2>';
    $output .=  '<p>' . t('This user does not have any computers that have made contact with @project recently.', array('@project' => PROJECT)) . '</p>';
  } else {
    $output .=  '<h2>' . t('Computers hidden') . '</h2>';
    $output .=  '<p>' . t('This user has chosen not to show information about their computers.') . '</p>';
  }
  return $output;
}
