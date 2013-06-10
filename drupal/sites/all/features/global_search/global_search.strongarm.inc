<?php

/**
 * Implementation of hook_strongarm().
 */
function global_search_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'search404_page_title';
  $strongarm->value = 'Page not found';
  $export['search404_page_title'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'search_by_page_settings';
  $strongarm->value = array(
    1 => array(
      'environment_name' => 'general',
      'page_title' => '',
      'block_title' => 'Search',
      'field_label' => 'Search',
      'button_label' => 'Search',
      'page_path' => 'find',
      'exclude_tags' => '',
      'sbp_nodes_types_indexed' => array(
        'forum' => 'forum',
        'page' => 'page',
        'panel' => 'panel',
        'profile' => 'profile',
        'story' => 'story',
      ),
      'sbp_nodes_display_type' => 'excerpts',
      'sbp_nodes_role' => '1',
      'sbp_nodes_min_time' => '1',
      'sbp_nodes_max_time' => '0',
      'sbp_paths_min_time' => '1',
      'sbp_paths_max_time' => '0',
      'sbp_users_roles_indexed' => array(
        2 => 2,
        4 => 0,
        5 => 0,
        3 => 0,
        6 => 0,
      ),
      'sbp_users_role' => '1',
      'sbp_users_min_time' => '1',
      'sbp_users_max_time' => '0',
      'op' => 'Save configuration',
    ),
  );
  $export['search_by_page_settings'] = $strongarm;

  return $export;
}
