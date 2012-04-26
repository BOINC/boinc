<?php
/**
 * @file
 * Contains the current user argument default plugin.
 */

/**
 * Default argument plugin to extract the global $user
 */
class views_plugin_argument_default_current_user extends views_plugin_argument_default {
  function get_argument() {
    global $user;
    return $user->uid;
  }
}

