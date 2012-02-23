<?php
// $Id: token.pages.inc,v 1.7.2.5 2010/08/10 03:33:32 davereid Exp $

/**
 * @file
 * User page callbacks for the token module.
 */

/**
 * For a given context, builds a formatted list of tokens and descriptions
 * of their replacement values.
 *
 * @param types
 *    The token types to display documentation for. Can be either a single
 *    string or an array of token types. Defaults to 'all'.
 * @param prefix
 *    The prefix your module will use when parsing tokens. Defaults to '['
 * @param suffix
 *    The suffix your module will use when parsing tokens. Defaults to ']'
 * @return An HTML table containing the formatting docs.
 */
function theme_token_help($types = 'all', $prefix = TOKEN_PREFIX, $suffix = TOKEN_SUFFIX) {
  token_include();
  $full_list = token_get_list($types);

  $headers = array(t('Token'), t('Replacement value'));
  $rows = array();
  foreach ($full_list as $key => $category) {
    $rows[] = array(array('data' => drupal_ucfirst($key) .' '. t('tokens'), 'class' => 'region', 'colspan' => 2));
    foreach ($category as $token => $description) {
      $row = array();
      $row[] = $prefix . $token . $suffix;
      $row[] = $description;
      $rows[] = $row;
    }
  }

  $output = theme('table', $headers, $rows, array('class' => 'description'));
  return $output;
}

/**
 * Provide a 'tree' display of nested tokens.
 */
function theme_token_tree($token_types = array(), $global_types = TRUE, $click_insert = TRUE) {
  //$info = token_get_list('all');
  $info = token_get_list($token_types);

  if ($token_types == 'all') {
  //  $token_types = array('all');
  }
  elseif ($global_types) {
  //  $token_types[] = 'global';
  //  $token_types = array_unique($token_types);
  }
  else {
    unset($info['global']);
  }

  // Check for token type validity and sort.
  //$token_types = array_intersect($token_types, array_keys($info));
  $token_types = array_keys($info);
  sort($token_types);

  $header = array(
    t('Token'),
    t('Description'),
  );
  $rows = array();

  foreach ($token_types as $type) {
    $parent = NULL;

    if (count($token_types) > 1) {
      $rows[] = _token_token_tree_format_row($type, array(), TRUE);
      $parent = $type;
    }

    foreach ($info[$type] as $token => $description) {
      $rows[] = _token_token_tree_format_row("[$token]", array('description' => $description, 'parent' => $parent));
    }
  }

  if (count($rows)) {
    drupal_add_js(drupal_get_path('module', 'token') . '/jquery.treeTable.js');
    drupal_add_css(drupal_get_path('module', 'token') . '/jquery.treeTable.css');
    drupal_add_js(drupal_get_path('module', 'token') . '/token.js');
    drupal_add_css(drupal_get_path('module', 'token') . '/token.css');
  }
  else {
    $rows[] = array(array(
      'data' => t('No tokens available.'),
      'colspan' => 2,
    ));
  }

  $table_options = array(
    'attributes' => array('class' => 'token-tree'),
    'caption' => '',
  );
  if ($click_insert) {
    $table_options['caption'] = t('Click a token to insert it into the field you\'ve last clicked.');
    $table_options['attributes']['class'] .= ' token-click-insert';
  }
  return theme('table', $header, $rows, $table_options['attributes'], $table_options['caption']);
}

/**
 * Build a row in the token tree.
 */
function _token_token_tree_format_row($token, $token_info = array(), $is_group = FALSE) {
  $row = array(
    'id' => _token_clean_css_identifier($token),
    'class' => array(),
    'data' => array(
      'token' => '',
      'description' => !empty($token_info['description']) ? $token_info['description'] : '',
    ),
  );

  if ($is_group) {
    // This is a token type/group.
    $row['data']['token'] = drupal_ucfirst($token);
    $row['class'][] = 'token-group';
    $row['id'] .= '-group';
  }
  else {
    // This is a token.
    $row['data']['token'] = array(
      'data' => $token,
      'class' => 'token-key',
    );
    if (!empty($token_info['parent'])) {
      $row['class'][] = 'child-of-' . _token_clean_css_identifier($token_info['parent']) . '-group';
    }
  }

  $row['class'] = implode(' ', $row['class']);

  return $row;
}

function _token_clean_css_identifier($id) {
  return 'token-' . str_replace(array('][', '_', ' ', ':'), '-', trim($id, '[]'));
}
