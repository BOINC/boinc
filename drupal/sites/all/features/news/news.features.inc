<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function news_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "context" && $api == "context") {
    return array("version" => 3);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function news_node_info() {
  $items = array(
    'news' => array(
      'name' => t('News item'),
      'module' => 'features',
      'description' => t('News items are bits of information and current events relevant to a BOINC project that are to be published to the website and RSS feeds.'),
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
function news_views_api() {
  return array(
    'api' => '2',
  );
}
