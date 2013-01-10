<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function user_profiles_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function user_profiles_node_info() {
  $items = array(
    'profile' => array(
      'name' => t('Profile'),
      'module' => 'features',
      'description' => t('A user profile built as content.'),
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
