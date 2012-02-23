<?php
// $Id: pathauto_node.inc,v 1.47.2.3 2010/07/31 15:57:16 davereid Exp $

/**
 * @file
 * Hook implementations for node module integration.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_pathauto().
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['groupheader'] = t('Node paths');
      $settings['patterndescr'] = t('Default path pattern (applies to all node types with blank patterns below)');
      $settings['patterndefault'] = 'content/[title-raw]';
      $settings['bulkname'] = t('Bulk generate aliases for nodes that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing nodes which do not already have aliases.');

      $patterns = token_get_list('node');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['supportsfeeds'] = 'feed';

      if (module_exists('locale')) {
        $languages = array('' => t('Language neutral')) + locale_language_list('name');
      }
      else {
        $languages = array();
      }
      foreach (node_get_types('names') as $node_type => $node_name) {
        if (variable_get('language_content_type_'. $node_type, 0) && count($languages)) {
          $settings['patternitems'][$node_type] = t('Default path pattern for @node_type (applies to all @node_type node types with blank patterns below)', array('@node_type' => $node_name));
          foreach ($languages as $lang_code => $lang_name) {
            if (!empty($lang_code)) {
              $settings['patternitems'][$node_type .'_'. $lang_code] = t('Pattern for all @node_type paths in @language', array('@node_type' => $node_name, '@language' => $lang_name));
            }
            else {
              $settings['patternitems'][$node_type .'_'. $lang_code] = t('Pattern for all language neutral @node_type paths', array('@node_type' => $node_name));
            }
          }
        }
        else {
          $settings['patternitems'][$node_type] = t('Pattern for all @node_type paths', array('@node_type' => $node_name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all nodes without aliases.
 */
function node_pathauto_bulkupdate() {
  // From all node types, only attempt to update those with patterns
  $pattern_types = array();

  // If there's a default pattern we assume all types might be updated.
  if (trim(variable_get('pathauto_node_pattern', ''))) {
    $pattern_types = array_keys(node_get_types('names'));
  }
  else {
    // Check first for a node specific pattern...
    $languages = array();
    if (module_exists('locale')) {
      $languages = array('' => t('Language neutral')) + locale_language_list('name');
    }
    foreach (array_keys(node_get_types('names')) as $type) {
      if (trim(variable_get('pathauto_node_'. $type .'_pattern', ''))) {
        $pattern_types[$type] = $type;
        continue;
      }
      // ...then for a node-language pattern.
      if (variable_get('language_content_type_'. $type, 0) && $languages) {
        foreach ($languages as $lang_code => $lang_name) {
          if (trim(variable_get('pathauto_node_'. $type .'_'. $lang_code .'_pattern', ''))) {
            $pattern_types[$type] = $type;
            continue 2;
          }
        }
      }
    }
  }

  $count = 0;
  if (count($pattern_types)) {
    $query = "SELECT n.nid, n.vid, n.type, n.title, n.uid, n.created, n.language, alias.src, alias.dst FROM {node} n LEFT JOIN {url_alias} alias ON CONCAT('node/', CAST(n.nid AS CHAR)) = alias.src WHERE alias.src IS NULL AND n.type IN (". db_placeholders($pattern_types, 'varchar') .')';
    $result = db_query_range($query, $pattern_types, 0, variable_get('pathauto_max_bulk_update', 50));

    $placeholders = array();
    while ($node_ref = db_fetch_object($result)) {
      $node = node_load($node_ref->nid, NULL, TRUE);
      $node->src = $node_ref->src;
      $node->dst = $node_ref->dst;
      if (module_exists('taxonomy')) {
        // Must populate the terms for the node here for the category
        // placeholders to work
        $node->taxonomy = array_keys(taxonomy_node_get_terms($node));
      }
      $placeholders = pathauto_get_placeholders('node', $node);
      $source = "node/$node->nid";
      if (pathauto_create_alias('node', 'bulkupdate', $placeholders, $source, $node->nid, $node->type, $node->language)) {
        $count++;
      }
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of nodes completed, one alias generated.',
    'Bulk generation of nodes completed, @count aliases generated.'));
}
