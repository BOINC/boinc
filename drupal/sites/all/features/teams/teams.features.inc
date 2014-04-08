<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function teams_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function teams_node_info() {
  $items = array(
    'team' => array(
      'name' => t('Team'),
      'module' => 'features',
      'description' => t('A grouping of users, centered on a common interest.'),
      'has_title' => '1',
      'title_label' => t('Name'),
      'has_body' => '1',
      'body_label' => t('Description'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implementation of hook_views_api().
 */
function teams_views_api() {
  return array(
    'api' => '2',
  );
}
