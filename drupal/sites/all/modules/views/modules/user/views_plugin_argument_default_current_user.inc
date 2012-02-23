<?php
// $Id: views_plugin_argument_default_current_user.inc,v 1.1 2008/10/01 19:50:31 merlinofchaos Exp $
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

