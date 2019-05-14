<?php

/**
 * @file
 * Performs a dependency check against the passed content type.
 */

/**
 * Adds a dependency on content type.
 */
class FacetapiDependencyRole extends FacetapiDependency {

  /**
   * Executes the dependency check.
   */
  public function execute() {
    global $user;
    if (1 != $user->uid) {
      $roles = array_filter($this->settings['roles']);
      if ($roles && !array_intersect_key($user->roles, $roles)) {
        return FALSE;
      }
    }
  }

  /**
   * Adds dependency settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {
    $form[$this->id]['roles'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Show facet for specific roles'),
      '#default_value' => $this->settings['roles'],
      '#options' => array_map('check_plain', user_roles()),
      '#description' => t('Show this facet only for the selected role(s). If you select no roles, the facet will be visible to all users.'),
    );
  }

  /**
   * Returns defaults for settings.
   */
  public function getDefaultSettings() {
    return array(
      'roles' => array(),
    );
  }
}
