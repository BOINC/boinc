<?php

/**
 * Expose user pages as a context condition.
 */
class context_condition_user_page extends context_condition {
  function condition_values() {
    $values = array();
    $values['view'] = t('User profile');
    $values['form'] = t('User account form');
    $values['register'] = t('Registration form');
    return $values;
  }

  function options_form($context) {
    $defaults = $this->fetch_from_context($context, 'options');
    return array(
      'mode' => array(
        '#title' => t('Active for'),
        '#type' => 'select',
        '#options' => array(
          'all' => t('Any user'),
          'current' => t('Only the current user'),
          'other' => t('Only other users'),
        ),
        '#default_value' => isset($defaults['mode']) ? $defaults['mode'] : 'all',
      ),
    );
  }

  function execute($account, $op) {
    global $user;
    foreach ($this->get_contexts($op) as $context) {
      if ($op === 'register') {
        $this->condition_met($context);
      }
      else {
        $options = $this->fetch_from_context($context, 'options');
        $mode = !empty($options['mode']) ? $options['mode'] : 'all';
        switch ($options['mode']) {
          case 'current':
            if ($account->uid == $user->uid) {
              $this->condition_met($context);
            }
            break;
          case 'other':
            if ($account->uid != $user->uid) {
              $this->condition_met($context);
            }
            break;
          case 'all':
          default:
            $this->condition_met($context);
            break;
        }
      }
    }
  }
}
