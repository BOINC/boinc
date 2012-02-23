<?php
// $Id: views_plugin_argument_validate_user.inc,v 1.2.4.2 2010/06/16 20:17:47 merlinofchaos Exp $

/**
 * Validate whether an argument is a valid user.
 *
 * This supports either numeric arguments (UID) or strings (username) and
 * converts either one into the user's UID.  This validator also sets the
 * argument's title to the username.
 */
class views_plugin_argument_validate_user extends views_plugin_argument_validate {
  function validate_form(&$form, &$form_state) {
    // We are unable to rely on options having already been set, so let's make
    // sure defaults are here:
    if (!isset($this->argument->options['validate_user_argument_type'])) {
      $this->argument->options['validate_user_argument_type'] = 'uid';
      $this->argument->options['validate_user_roles'] = array();
    }

    $form['validate_user_argument_type'] = array(
      '#type' => 'radios',
      '#title' => t('Type of user argument to allow'),
      '#options' => array(
        'uid' => t('Only allow numeric UIDs'),
        'name' => t('Only allow string usernames'),
        'either' => t('Allow both numeric UIDs and string usernames'),
      ),
      '#default_value' => $this->argument->options['validate_user_argument_type'],
      '#process' => array('expand_radios', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
      '#prefix' => '<div id="edit-options-validate-user-argument-type-wrapper">',
      '#suffix' => '</div>',
    );

    $form['validate_user_restrict_roles'] = array(
      '#type' => 'checkbox',
      '#title' => t('Restrict user based on role'),
      '#default_value' => !empty($this->argument->options['validate_user_restrict_roles']),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_user_roles'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-user-roles-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Restrict to the selected roles'),
      '#options' => user_roles(TRUE),
      '#default_value' => $this->argument->options['validate_user_roles'],
      '#description' => t('If no roles are selected, users from any role will be allowed.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array(
        'edit-options-validate-type' => array($this->id),
        'edit-options-validate-user-restrict-roles' => array(1),
      ),
      '#dependency_count' => 2,
    );
  }

  function validate_argument($argument) {
    $type = $this->argument->options['validate_user_argument_type'];
    // is_numeric() can return false positives, so we ensure it's an integer.
    // However, is_integer() will always fail, since $argument is a string.
    if (is_numeric($argument) && $argument == (int)$argument) {
      if ($type == 'uid' || $type == 'either') {
        if ($argument == $GLOBALS['user']->uid) {
          $account = $GLOBALS['user'];
        }
        $where = 'uid = %d';
      }
    }
    else {
      if ($type == 'name' || $type == 'either') {
        if ($argument == $GLOBALS['user']->name) {
          $account = $GLOBALS['user'];
        }
        $where = "name = '%s'";
      }
    }

    // If we don't have a WHERE clause, the argument is invalid.
    if (empty($where)) {
      return FALSE;
    }

    if (!isset($account)) {
      $query = "SELECT uid, name FROM {users} WHERE $where";
      $account = db_fetch_object(db_query($query, $argument));
    }
    if (empty($account)) {
      // User not found.
      return FALSE;
    }

    // See if we're filtering users based on roles.
    if (!empty($this->argument->options['validate_user_restrict_roles']) && !empty($this->argument->options['validate_user_roles'])) {
      $roles = $this->argument->options['validate_user_roles'];
      $account->roles = array();
      $account->roles[] = $account->uid ? DRUPAL_AUTHENTICATED_RID : DRUPAL_ANONYMOUS_RID;
      $result = db_query('SELECT rid FROM {users_roles} WHERE uid = %d', $account->uid);
      while ($role = db_fetch_object($result)) {
        $account->roles[] = $role->rid;
      }
      if (!(bool)array_intersect($account->roles, $roles)) {
        return FALSE;
      }
    }

    $this->argument->argument = $account->uid;
    $this->argument->validated_title = check_plain($account->name);
    return TRUE;
  }
}

