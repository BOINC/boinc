<?php
// $Id: views_plugin_argument_validate_php.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * @file
 * Contains the php code argument validator plugin.
 */

/**
 * Provide PHP code to validate whether or not an argument is ok.
 *
 * @ingroup views_argument_validate_plugins
 */
class views_plugin_argument_validate_php extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_php';

  function validate_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'textarea',
      '#title' => t('PHP validate code'),
      '#default_value' => $this->get_argument(),
      '#description' => t('Enter PHP code that returns TRUE or FALSE. No return is the same as FALSE, so be SURE to return something if you do not want to declare the argument invalid. Do not use &lt;?php ?&gt;. The argument to validate will be "$argument" and the view will be "$view". You may change the argument by setting "$handler->argument".'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $this->check_access($form);
  }

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * validate plugin.
   */
  function access() {
    return user_access('use PHP for block visibility');
  }

  function validate_argument($argument) {
    // set up variables to make it easier to reference during the argument.
    $view = &$this->view;
    $handler = &$this->argument;

    ob_start();
    $result = eval($this->argument->options[$this->option_name]);
    ob_end_clean();
    return $result;
  }
}

