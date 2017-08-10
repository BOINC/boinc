<?php

/**
 * @file
 * Flag module tokens support.
 */

/**
 * Implementation of hook_token_list().
 */
function flag_token_list($type = 'all') {
  $tokens = array();
  if ($type == 'flag' || $type == 'all') {
    $tokens['flag']['flag-name'] = t('The flag name.');
    $tokens['flag']['flag-title'] = t('The flag title.');
  }
  if ($type == 'flag-action' || $type == 'all') {
    $tokens['flag-action']['flag-action'] = t('The flagging action taking place.');
    $tokens['flag-action']['flag-content-url'] = t('The URL of the content being flagged.');
    $tokens['flag-action']['flag-content-title'] = t('The title of the content being flagged.');
    $tokens['flag-action']['flag-content-type'] = t('The type of content being flagged, such as <em>node</em> or <em>comment</em>.');
    $tokens['flag-action']['flag-content-id'] = t('The ID of content being flagged, may be a nid or cid.');
    $tokens['flag-action']['flag-count'] = t('The current count total for this flag.');
  }
  foreach (flag_get_types() as $flag_type) {
    if ($type == $flag_type || $type == 'all') {
      $flags = flag_get_flags($flag_type);
      foreach ($flags as $flag) {
        $tokens[$flag_type]['flag-'. str_replace('_', '-', $flag->name) .'-count'] = t('Total flag count for flag @flag', array('@flag' => $flag->get_title()));
        $tokens[$flag_type]['flag-'. str_replace('_', '-', $flag->name) .'-link'] = t('Flag/unflag link for @flag', array('@flag' => $flag->get_title()));
      }
    }
  }
  return $tokens;
}

/**
 * Implementation of hook_token_values().
 */
function flag_token_values($type, $object = NULL) {
  $values = array();
  if ($type == 'flag') {
    $values['flag-name'] = check_plain($object->name);
    $values['flag-title'] = check_plain($object->get_title());
  }
  elseif ($type == 'flag-action') {
    $values['flag-action'] = $object->action;
    $values['flag-content-url'] = check_url($object->content_url);
    $values['flag-content-title'] = check_plain($object->content_title);
    $values['flag-content-type'] = $object->content_type;
    $values['flag-content-id'] = $object->content_id;
    $values['flag-count'] = $object->count;
  }
  if (in_array($type, flag_get_types())) {
    $flags = flag_get_flags($type);
    foreach ($flags as $flag) {
      $values['flag-'. str_replace('_', '-', $flag->name) .'-count'] = $flag->get_count($flag->get_content_id($object));
      $values['flag-'. str_replace('_', '-', $flag->name) .'-link'] = flag_create_link($flag->name, $flag->get_content_id($object));
    }
  }
  return $values;
}

/**
 * This is a replacement for Token's theme_token_help().
 *
 * This is a duplicate of the Token's function, but with a small modification:
 * the $type parameter is now $types, allowing for listing tokens from several
 * categories.
 *
 * @todo Incorporate this feature into the Token module(?)
 */
function theme_flag_tokens_browser($types = array('all'), $prefix = '[', $suffix = ']') {
  token_include();
  $tokens = flag_token_get_list($types);

  $header = array(t('Token'), t('Replacement value'));
  $rows = array();
  foreach ($tokens as $key => $category) {
    $rows[] = array(array('data' => drupal_ucfirst($key) . ' ' . t('tokens'), 'class' => 'region', 'colspan' => 2));
    foreach ($category as $token => $description) {
      $row = array();
      $row[] = $prefix . $token . $suffix;
      $row[] = $description;
      $rows[] = $row;
    }
  }

  $output = theme('table', $header, $rows, array('class' => 'description'));
  return $output;
}

/**
 * This is a replacement for Token's token_get_list().
 *
 * It is used only by theme_flag_tokens_browser(), above.
 *
 * This is a duplicate of the Token's function, but with a small modification:
 * the $type parameter is now $types, allowing for listing tokens from several
 * categories.
 *
 * @todo Incorporate this feature into the Token module(?)
 */
function flag_token_get_list($types = array('all')) {
  token_include();
  $return = array();
  foreach ($types as $type) {
    foreach (module_implements('token_list') as $module) {
      $function = $module .'_token_list';
      $result = $function($type);
      if (is_array($result)) {
        foreach ($result as $category => $tokens) {
          foreach ($tokens as $token => $title) {
            $return[$category][$token] = $title;
          }
        }
      }
    }
  }
  // For aesthetic reasons, we don't want the 'global' section to appear in
  // varying places, so let's move it to the bottom.
  if (isset($return['global'])) {
    $global = $return['global'];
    unset($return['global']);
    $return['global'] = $global;
  }
  return $return;
}
