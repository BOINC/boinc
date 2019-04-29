<?php

/**
 * @file
 * Current Search default hooks.
 */

/**
 * Deletes object and block visibility information from the database.
 *
 * @param stdClass $object
 *   The object being deleted.
 */
function current_search_export_crud_delete($object) {
  // Deletes the current search block configuration.
  $query = "DELETE FROM {current_search} WHERE name = '%s'";
  db_query($query, array($object->name));

  // Only deletes the association with the searcher if we are not reverting a
  // configuration setting.
  if (!($object->export_type & EXPORT_IN_CODE && $object->export_type & EXPORT_IN_DATABASE)) {
    $query = "DELETE FROM {block_current_search} WHERE delta = '%s'";
    db_query($query, array($object->name));
  }
}

/**
 * Implements hook_current_search_default_items().
 */
function current_search_current_search_default_items() {
  $items = array();

  $item = new stdClass;
  $item->disabled = FALSE;
  $item->api_version = 1;
  $item->name = 'standard';
  $item->label = 'Standard';
  $item->settings = array(
    'items' => array(
      'results' => array(
        'id' => 'text',
        'label' => 'Results',
        'text' => 'Search found [facetapi_results:result-count] item',
        'plural' => 1,
        'text_plural' => 'Search found [facetapi_results:result-count] items',
        'plural_condition' => 'facetapi_results:result-count',
        'wrapper' => 1,
        'element' => 'h3',
        'css' => 0,
        'classes' => '',
        'weight' => '-50',
      ),
      'active_items' => array(
        'id' => 'active',
        'label' => 'Active items',
        'pattern' => '[facetapi_active:active-value]',
        'keys' => 1,
        'css' => 0,
        'classes' => '',
        'weight' => '-49',
      ),
    ),
    'advanced' => array(
      'empty_searches' => 0,
    ),
  );
  $items[$item->name] = $item;

  return $items;
}
