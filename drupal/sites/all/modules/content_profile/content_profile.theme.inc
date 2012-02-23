<?php
// $Id: content_profile.theme.inc,v 1.1.2.9 2009/09/15 15:35:47 fago Exp $

/**
 * @file
 * Theme and template preprocessing code
 */

/**
 * Themes the add link.
 */
function theme_content_profile_display_add_link($element) {
  $type = $element['#content_type'];
  $uid = $element['#admin'] ? 'uid='. intval($element['#uid']) .'&' : '';
  $text = t($element['#admin'] ? "Create the user's @profile_node." : "Create your @profile_node.", array('@profile_node' => node_get_types('name', $type)));
  return l($text, content_profile_get_add_path($type, $element['#uid']), array('query' => $uid . drupal_get_destination(), 'html' => TRUE));
}

/**
 * Theme function for the content_profile display as link
 */
function theme_content_profile_display_link($element) {
  if ($node = content_profile_load($element['#content_type'], $element['#uid'])) {
    if (node_access('view', $node)) {
      $output = l(node_get_types('name', $node->type), 'node/'. $node->nid);
      if ($element['#edit_link'] && node_access('update', $node)) {
        $output .= ' '. l('['. t('edit') .']', content_profile_get_edit_path($node), array('query' => drupal_get_destination()));
      }
      return $output;
    }
  }
}

/**
 * Implementation of content_profile_preprocess_HOOK()
 */
function content_profile_preprocess_content_profile_display_view(&$variables) {
  $element = $variables['element'];
  $node = content_profile_load($element['#content_type'], $element['#uid']);
  $variables['node'] = &$node;
  $variables['uid'] = $element['#uid'];
  $variables['type'] = $element['#content_type'];

  $path = drupal_get_path('module', 'content_profile') .'/content_profile.css';
  drupal_add_css($path, 'module', 'all', FALSE);

  $variables['title'] = check_plain(node_get_types('name', $node->type));

  $tabs = array();
  if ($element['#edit_link']) {
    $tabs[] = theme('content_profile_display_tab_view', $node);
    $tabs[] = theme('content_profile_display_tab_edit', $node);
  }
  if (count($tabs) > 0) {
    $variables['tabs'] = $tabs;
  }
  $variables['content'] = node_view($node, ($element['#style'] == 'teaser'), TRUE, TRUE);
}

/**
 * Themes the view tab
 */
function theme_content_profile_display_tab_view($node) {
  return l(t('View'), 'node/'. $node->nid);
}

/**
 * Themes the edit tab
 */
function theme_content_profile_display_tab_edit($node) {
  if (node_access('update', $node)) {
    return l(t('Edit'), content_profile_get_edit_path($node), array('query' => drupal_get_destination()));
  }
}

/**
 * Gets the edit path for a content_profile
 */
function content_profile_get_edit_path($node) {
  $handler = variable_get('content_profile_path_handler', 'content_profile_default_path_handler');
  return $handler('edit', $node, $node->uid);
}

/**
 * Gets the add path for a content_profile of the active user
 */
function content_profile_get_add_path($type, $uid) {
  $handler = variable_get('content_profile_path_handler', 'content_profile_default_path_handler');
  return $handler('add', $type, $uid);
}

/**
 * Default path handler for content profile, which uses the default system paths
 *
 * @param $action 'add' or 'edit
 * @param $arg For 'add' the content type, for 'edit' the node to be edited
 * @param $uid the uid of the profile's owner.
 */
function content_profile_default_path_handler($action, $arg, $uid) {
  $type = $action == 'add' ? $arg : $arg->type;
  if (content_profile_get_settings($type, 'edit_tab') == 'top') {
    return 'user/'. $uid . '/profile/'. $type;
  }
  elseif (content_profile_get_settings($type, 'edit_tab') == 'sub') {
    return 'user/'. $uid . '/edit/'. $type;
  }
  elseif ($action == 'add') {
    return 'node/add/'. str_replace('_', '-', $arg);
  }
  else {
    return 'node/'. $arg->nid .'/edit';
  }
}
