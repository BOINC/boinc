<?php
/**
 * @file
 * Contains the base argument validator plugin.
 */

/**
 * @defgroup views_argument_validate_plugins Views' argument validate plugins
 * @{
 *
 * Allow specialized methods of validating arguments.
 *
 * @see hook_views_plugins
 */

/**
 * Base argument validator plugin to provide basic functionality.
 *
 * @ingroup views_argument_validate_plugins
 */
class views_plugin_argument_validate extends views_plugin {
  var $option_name = 'validate_argument';

  /**
   * Initialize this plugin with the view and the argument
   * it is linked to.
   */
  function init(&$view, &$argument, $id = NULL) {
    $this->view = &$view;
    $this->argument = &$argument;
    $this->id = $id;
  }

  /**
   * Determine if the administrator has the privileges to use this
   * plugin
   */
  function access() { return TRUE; }

  function argument_form(&$form, &$form_state) {
  }

  /**
   * If we don't have access to the form but are showing it anyway, ensure that
   * the form is safe and cannot be changed from user input.
   */
  function check_access(&$form) {
    if (!$this->access()) {
      $form[$this->option_name]['#disabled'] = TRUE;
      $form[$this->option_name]['#value'] = $form[$this->option_name]['#default_value'];
      $form[$this->option_name]['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this. If you change the validator, this setting will be lost and you will NOT be able to get it back.') . '</strong>';
    }
  }

  /**
   * Return the validate argument.
   */
  function get_argument() {
    return isset($this->argument->options[$this->option_name]) ? $this->argument->options[$this->option_name] : '';
  }

  function validate_form(&$form, &$form_state) { }

  function validate_argument($arg) { return TRUE; }
}

/**
 * @}
 */

