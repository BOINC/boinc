<?php

/**
 * Implementation of hook_filter_default_formats().
 */
function features_test_filter_default_formats() {
  $formats = array();

  // Exported format: features_test
  $formats['features_test'] = array(
    'name' => 'features_test',
    'roles' => array(
      '0' => 'anonymous user',
      '1' => 'authenticated user',
      '2' => 'manager',
    ),
    'filters' => array(
      '0' => array(
        'module' => 'filter',
        'delta' => '2',
        'weight' => '0',
      ),
      '1' => array(
        'module' => 'filter',
        'delta' => '0',
        'weight' => '1',
      ),
      '2' => array(
        'module' => 'filter',
        'delta' => '1',
        'weight' => '2',
      ),
      '3' => array(
        'module' => 'filter',
        'delta' => '3',
        'weight' => '10',
      ),
    ),
  );

  return $formats;
}
