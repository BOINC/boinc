<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function team_forums_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "context" && $api == "context") {
    return array("version" => 3);
  }
  elseif ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function team_forums_node_info() {
  $items = array(
    'team_forum' => array(
      'name' => t('Team forum topic'),
      'module' => 'features',
      'description' => t('A <em>team forum topic</em> is a special topic within a team forum. This may or may not be visible only to members of the corresponding team.'),
      'has_title' => '1',
      'title_label' => t('Subject'),
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
function team_forums_views_api() {
  return array(
    'api' => '2',
  );
}
