<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function news_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['node_comment_block-node_comments'] = array(
    'module' => 'node_comment_block',
    'delta' => 'node_comments',
    'theme' => 'boinc',
    'status' => '1',
    'weight' => '3',
    'region' => 'content_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php
  // Only show if $match is true
  $match = FALSE;

  // Which node types
  $types = array(\'news\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
  }

  return $match;
?>',
    'title' => '',
    'cache' => '1',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['node_comment_block-node_comments'] = array(
    'module' => 'node_comment_block',
    'delta' => 'node_comments',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-19',
    'region' => 'content_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php
  // Only show if $match is true
  $match = FALSE;

  // Which node types
  $types = array(\'news\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
  }

  return $match;
?>',
    'title' => '',
    'cache' => '1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['node_comment_block-node_comments'] = array(
    'module' => 'node_comment_block',
    'delta' => 'node_comments',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php
  // Only show if $match is true
  $match = FALSE;

  // Which node types
  $types = array(\'news\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
  }

  return $match;
?>',
    'title' => '',
    'cache' => '1',
  );

  $export['garland'] = $theme;

  $theme_default = variable_get('theme_default', 'garland');
  $themes = list_themes();
  foreach ($export as $theme_key => $settings) {
    if ($theme_key != $theme_default && empty($themes[$theme_key]->status)) {
      unset($export[$theme_key]);
    }
  }
  return $export;
}
