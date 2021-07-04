<?php
// $Id: content_profile.theme_vars.inc,v 1.1.2.6 2009/07/31 16:41:15 fago Exp $

/**
 * @file
 * Provides a helper class for lazy loading of variables for themes.
 */

/**
 * A helper class, which offers lazy loading of variables for themes.
 */
class content_profile_theme_variables {

  var $uid;
  var $_cache = array();

  function __constructor($uid) {
    $this->uid = $uid;
  }

  function content_profile_theme_variables($uid) {
    self::__constructor($uid);
  }

  /**
   * Gets the user id of the profiles owner.
   */
  function get_uid() {
    return $uid;
  }

  /**
   * Gets all type names keyed with their machine readable names.
   */
  function get_profile_types() {
    return content_profile_get_types('names');
  }

  /**
   * Gets all template variables for the content profile of this type.
   *
   * @param $type
   *   The type of the user's content profile
   * @param $teaser
   *   Whether the value is to be generated for the teaser.
   * @param $page
   *   Whether the value is to be generated for the page view.
   *
   * @return
   *   An array of variables available for the profile node
   *   or FALSE if there has been no profile created yet.
   */
  function get_variables($type, $teaser = FALSE, $page = FALSE) {
    if (!isset($this->_cache[$type][$teaser][$page])) {
      $this->_cache[$type][$teaser][$page] = FALSE;

      if ($node = content_profile_load($type, $this->uid)) {
        // Make sure the node is prepared for viewing
        $node = node_build_content($node, $teaser, $page);
        $vars = array('node' => $node, 'teaser' => $teaser, 'page' => $page);

        // Apply all node template preprocessors
        foreach ($this->_get_node_preprocessors() as $function) {
          if (function_exists($function)) {
            $function($vars, 'node');
          }
        }
        $this->_cache[$type][$teaser][$page] = $vars;
      }
    }
    return $this->_cache[$type][$teaser][$page];
  }

  /**
   * Gets a single template variable for the content profile of this type.
   *
   * @param $type
   *   The type of the user's content profile
   * @param $name
   *   The name of the variable to get.
   *
   * @return
   *   The variable or FALSE if there has been no profile created yet.
   */
  function get_variable($type, $name) {
    if ($vars = $this->get_variables($type)) {
      return $vars[$name];
    }
    return FALSE;
  }


  /**
   * Generate a display of the given node.
   *
   * @param $type
   *   The type of the user's content profile
   * @param $teaser
   *   Whether to display the teaser only or the full form.
   * @param $page
   *   Whether the node is being displayed by itself as a page.
   * @param $links
   *   Whether or not to display node links. Links are omitted for node previews.
   *
   * @return
   *   An HTML representation of the themed node or FALSE if there has been no profile created yet.
   */
  function get_view($type, $teaser = FALSE, $page = FALSE, $links = TRUE) {
    if ($node = content_profile_load($type, $this->uid)) {
      return node_view($node, $teaser, $page, $links);
    }
    return FALSE;
  }


  function _get_node_preprocessors() {
    $hooks = theme_get_registry();
    $functions = $hooks['node']['preprocess functions'];
    // We don't need 'template_preprocess'
    unset($functions[0]);
    return $functions;
  }
}
