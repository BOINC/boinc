<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function discussion_forums_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['comment_form_block-comment_form'] = array(
    'module' => 'comment_form_block',
    'delta' => 'comment_form',
    'theme' => 'boinc',
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
  $types = array(\'forum\', \'team_forum\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
    // Don\'t show if comments aren\'t enabled...
    if ($node->comment != 2) {
      $match = FALSE;
    }
  }

  return $match;
?>',
    'title' => '',
    'cache' => '1',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['comment_form_block-comment_form'] = array(
    'module' => 'comment_form_block',
    'delta' => 'comment_form',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '0',
    'region' => 'content_bottom',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '2',
    'pages' => '<?php
  // Only show if $match is true
  $match = FALSE;

  // Which node types
  $types = array(\'forum\', \'team_forum\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
    // Don\'t show if comments aren\'t enabled...
    if ($node->comment != 2) {
      $match = FALSE;
    }
  }

  return $match;
?>',
    'title' => '',
    'cache' => '1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['comment_form_block-comment_form'] = array(
    'module' => 'comment_form_block',
    'delta' => 'comment_form',
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
  $types = array(\'forum\', \'team_forum\');

  // Match current node type with array of types
  if (arg(0) == \'node\' && is_numeric(arg(1))) {
    $nid = arg(1);
    $node = node_load(array(\'nid\' => $nid));
    $type = $node->type;
    $match |= in_array($type, $types);
    // Don\'t show if comments aren\'t enabled...
    if ($node->comment != 2) {
      $match = FALSE;
    }
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
