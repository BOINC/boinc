<?php

/**
 * @file
 * Callbacks referenced in hook implementations.
 */

/**
 * Maps bundle machine names to their human friendly label.
 *
 * @param array $values
 *   An array of indexed values being mapped.
 * @param array $options
 *   An associative array of map options containing:
 *   - entities: An array of entities that $values are bundles for.
 *
 * @return array
 *   An array mapping the indexed values to human readable values.
 */
function facetapi_map_bundle(array $values, array $options) {
  if (empty($options['entities'])) {
    $options['entities'] = array('node');
  }
  foreach ($options['entities'] as $entity_type) {
    // Ignore entity types for Drupal 6
    if ($info = content_types()) {
      foreach ($info as $bundle_name => $bundle_info) {
        $names[$bundle_name] = $bundle_info['name'];
      }
    }
  }
  return array_intersect_key($names, array_flip($values));
}

/**
 * Map callback for node authors.
 */
function facetapi_map_author(array $values) {
  $map = array();
  if (!empty($values)) {
    $query = "SELECT u.uid AS uid, u.name AS name FROM {users} u WHERE u.uid IN (" . db_placeholders($values) . ")";
    $result = db_query($query, $values);
    while ($record = db_fetch_array($result)) {
      $map[$record['uid']] = $record['name'];
    }
  }
  if (isset($map[0])) {
    $map[0] = variable_get('anonymous', t('Anonymous'));
  }
  return $map;
}

/**
 * Map callback for languages.
 */
function facetapi_map_language(array $values) {
  $map = array();
  $language_list = language_list();
  foreach ($values as $language) {
    if (isset($language_list[$language])) {
      $map[$language] = t($language_list[$language]->name);
    }
    else {
      $map[$language] = t('Language neutral');
    }
  }
  return $map;
}

/**
 * Maps date ranges to human readable dates.
 *
 * @param array $values
 *   An array of indexed values being mapped.
 * @param array $options
 *   An associative array of map options containing:
 *   - format callback: The callback used to format the date, defaults to
 *     "facetapi_format_timestamp".
 *
 * @return array
 *   An array mapping the indexed values to human readable values.
 */
function facetapi_map_date(array $values, array $options) {
  if (empty($options['format callback'])) {
    $options['format callback'] = 'facetapi_format_timestamp';
  }
  $map = array();
  foreach ($values as $value) {
    $range = explode(' TO ', trim($value, '{[]}'));
    if (isset($range[1])) {
      $gap = facetapi_get_date_gap($range[0], $range[1]);
      $map[$value] = facetapi_format_date($range[0], $gap, $options['format callback']);
    }
  }
  return $map;
}

/**
 * Callback that returns the minimum date in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_min_date(array $facet) {
  $query = "SELECT MIN(%s) as min FROM node n WHERE n.status = 1";
  $result = db_result(db_query($query, array($facet['name'])));
  return $result;
}

/**
 * Callback that returns the minimum value in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_max_date(array $facet) {
  $query = "SELECT MAX(%s) as max FROM node n WHERE n.status = 1";
  $result = db_result(db_query($query, array($facet['name'])));
  return $result;
}

/**
 * Map callback for taxonomy terms.
 */
function facetapi_map_taxonomy_terms(array $values) {
  $map = array();
  if(!empty($values)) {
    $query = "SELECT tid, name
      FROM {term_data}
      WHERE tid IN (" . db_placeholders($values, 'int') . ")";
    $result = db_query($query, $values);
    while($term = db_fetch_object($result)) {
      $map[$term->tid] = $term->name;
    }
  }
  return $map;
}

/**
 * Gets parent information for taxonomy terms.
 *
 * @param array $values
 *   An array containing the term ids.
 *
 * @return
 *   An associative array keyed by term ID to parent ID.
 */
function facetapi_get_taxonomy_hierarchy(array $values) {
  $parents = array();
  if(!empty($values)) {
    $query = "SELECT th.tid AS tid, th.parent AS parent
      FROM {term_hierarchy} th
      WHERE (th.parent > 0)
      OR (th.tid IN(" . db_placeholders($values) . ") AND th.parent IN(" . db_placeholders($values) . "))";
    $result = db_query($query, array_merge($values, $values));

    while($record = db_fetch_object($result)) {
      $parents[$record->tid][] = $record->parent;
    }
  }
  return $parents;
}
