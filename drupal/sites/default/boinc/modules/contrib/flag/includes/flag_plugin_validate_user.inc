<?php
/**
 * @file
 * Contains the Flag Bookmark view argument validator.
 */

/**
 * Validates whether an argument is a valid UID.
 *
 * @ingroup views
 */
class flag_plugin_validate_user extends views_plugin_argument_validate_user {

  /**
   * Define the options for the plugin, including the default permission.
   * @return multitype:string
   */
  function option_definition() {
    // Initialize the base class.
    $options = parent::option_definition();

    // Set the default permission.
    $options['bypass_perm'] = array('default' => 'administer users');

    return $options;
  }

  /**
   * Returns a option form for the plugin.
   */
  function options_form(&$form, &$form_state) {
    // Get the options form from the base class.
    parent::options_form($form, $form_state);

    $perms = array();

    $perms[] = t(' - None - ');

    // Produce an array of permissions keyed by module name.
    foreach (module_implements('perm') as $module) {
      $permissions = module_invoke($module, 'perm');
      foreach ($permissions as $perm) {
        $perms[$module][$perm] = strip_tags($perm);
      }
    }

    asort($perms);

    // Create the form field for the validator. Returned by reference.
    $form['bypass_perm'] = array(
      '#type' => 'select',
      '#options' => $perms,
      '#title' => t('Override permission'),
      '#default_value' => $this->options['bypass_perm'],
      '#description' => t('Users with this permission bypass the argument check and are granted access.'),
    );
  }

  /**
   * Validates the argument to be a proper UID.
   * @param mixed $argument
   * @return boolean
   */
  function validate_argument($argument) {
    // The parent class takes care of all its options, returning TRUE if the
    // argument value validates to a user account, and an account that has the
    // required role.
    $argument_validates = parent::validate_argument($argument);

    // If the parent didn't validate the argument, then we certainly can't
    // either.
    if ($argument_validates === FALSE) {
      return $argument_validates;
    }

    // If the current user has the bypass permission, then we're done: return
    // the validation status we got from the parent.
    if (!empty($this->options['bypass_perm']) && user_access($this->options['bypass_perm'])) {
      return $argument_validates;
    }

    // Otherwise, perform our additional check to enforce that the argument
    // user ID is the current user.
    // The parent method has stored the uid from the argument.
    return ($this->argument->argument === $GLOBALS['user']->uid);
  }

}
