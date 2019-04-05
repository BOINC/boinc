<?php

/**
 * Expose current user role as a context condition.
 */
class context_condition_user extends context_condition {
  function condition_values() {
    $values = array();
    foreach (user_roles() as $rid => $role_name) {
      if ($rid == DRUPAL_ANONYMOUS_RID) {
        $values['anonymous user'] = check_plain($role_name);
      }
      elseif ($rid == DRUPAL_AUTHENTICATED_RID) {
        $values['authenticated user'] = check_plain($role_name);
      }
      else {
        $values[$role_name] = check_plain($role_name);
      }
    }
    return $values;
  }

  function execute($account) {
    $roles = $account->roles;
    foreach ($roles as $rid => $role) {
      foreach ($this->get_contexts($role) as $context) {
        $this->condition_met($context, $role);
      }
    }
  }
}
