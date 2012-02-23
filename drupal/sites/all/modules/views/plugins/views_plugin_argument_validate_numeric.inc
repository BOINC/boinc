<?php
// $Id: views_plugin_argument_validate_numeric.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * @file
 * Contains the numeric argument validator plugin.
 */

/**
 * Validate whether an argument is numeric or not.
 *
 * @ingroup views_argument_validate_plugins
 */
class views_plugin_argument_validate_numeric extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_numeric';

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * validate plugin.
   */
  function access() {
    return !empty($this->argument->definition['numeric']);
  }

  function validate_argument($argument) {
    return is_numeric($argument);
  }
}

