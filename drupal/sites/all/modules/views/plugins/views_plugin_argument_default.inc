<?php
/**
 * @file
 * Contains the fixed argument default plugin.
 */

/**
 * @defgroup views_argument_default_plugins Views' argument default plugins
 * @{
 *
 * Allow specialized methods of filling in arguments when they aren't
 * provided.
 *
 * @see hook_views_plugins
 */

/**
 * The fixed argument default handler; also used as the base.
 */
class views_plugin_argument_default extends views_plugin {
  var $option_name = 'default_argument_fixed';
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
    $form[$this->option_name] = array(
      '#type' => 'textfield',
      '#title' => t('Default argument'),
      '#default_value' => $this->get_argument(),
      '#process' => array('views_process_dependency'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );

    // Only do this if using one simple standard form gadget
    $this->check_access($form);
  }

  /**
   * If we don't have access to the form but are showing it anyway, ensure that
   * the form is safe and cannot be changed from user input.
   */
  function check_access(&$form) {
    if (!$this->access()) {
      $form[$this->option_name]['#disabled'] = TRUE;
      $form[$this->option_name]['#value'] = $form[$this->option_name]['#default_value'];
      $form[$this->option_name]['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this. If you change the default argument type, this setting will be lost and you will NOT be able to get it back.') . '</strong>';
    }
  }

  /**
   * Return the default argument.
   */
  function get_argument() {
    return isset($this->argument->options[$this->option_name]) ? $this->argument->options[$this->option_name] : '';
  }
}

/**
 * @}
 */

