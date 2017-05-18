<?php

/**
 * Implementation of hook_imagecache_default_presets().
 */
function features_test_imagecache_default_presets() {
  $items = array(
    'features_test' => array(
      'presetname' => 'features_test',
      'actions' => array(
        '0' => array(
          'weight' => '0',
          'module' => 'imagecache',
          'action' => 'imagecache_scale',
          'data' => array(
            'width' => '100%',
            'height' => '100%',
            'upscale' => 0,
          ),
        ),
      ),
    ),
  );
  return $items;
}

/**
 * Implementation of hook_node_info().
 */
function features_test_node_info() {
  $items = array(
    'features_test' => array(
      'name' => t('Testing: Features'),
      'module' => 'features',
      'description' => t('Content type provided for Features tests.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '1',
      'body_label' => t('Body'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implementation of hook_views_api().
 */
function features_test_views_api() {
  return array(
    'api' => '2',
  );
}
